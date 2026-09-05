#include "pch.hpp"
#include "MetadataManager.hpp"

#include <cassert>
#include <cstddef>
#include <algorithm>
#include <format>
#include <mutex>
#include <ranges>
#include <stdexcept>
#include <system_error>

#include "Analyzer/Passes/MacroBinder.hpp"
#include "Analyzer/Passes/SymbolLinker.hpp"
#include "Analyzer/Passes/TypeResolver.hpp"
#include "Analyzer/Syntax/Lexer.hpp"
#include "Analyzer/Syntax/Parser.hpp"
#include "Base/FileSystem/Source.hpp"
#include "Base/Unicode.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    namespace {
        bool EndsWithCaseInsensitive(std::string_view text, std::string_view suffix) {
            if (suffix.size() > text.size()) {
                return false;
            }

            auto text_it   = text.end() - suffix.size();
            auto suffix_it = suffix.begin();
            while (suffix_it != suffix.end()) {
                if (std::tolower(*text_it) != std::tolower(*suffix_it)) {
                    return false;
                }

                ++text_it;
                ++suffix_it;
            }

            return true;
        }

        std::string MakeMacroFingerprint(const MacroTable& macros) {
            std::vector<std::string_view> names;
            for (const auto& [name, _] : macros) {
                names.push_back(name);
            }

            std::ranges::sort(names);
            std::string key;
            for (const auto& name : names) {
                key += name;
                key += ';';
            }

            return key;
        }

        std::optional<std::filesystem::path> TryResolveMetadataFile(const std::filesystem::path& relative_path) {
            const auto path = std::filesystem::path(Utils::GetFilePath(relative_path.generic_string()));
            std::error_code ec;
            if (!std::filesystem::exists(path, ec) || ec) {
                return std::nullopt;
            }

            return Utils::NormalizePath(path);
        }

        struct MacroCollectResult {
            MacroTable       injected_macros;
            std::string_view version{ "460" };
        };

        MacroCollectResult CollectRequestedExtensionsAndVersion(std::span<const Token> raw_tokens) {
            MacroCollectResult result;

            auto InjectMacro = [&injected_macros = result.injected_macros](std::string_view name) -> void {
                injected_macros.try_emplace(name, MacroDefinition{
                    .is_function = false,
                    .original_token = Token{
                        .text = name,
                        .type = TokenType::kIdentifier
                    },
                    .replacement_list = { Token{
                        .text = "1",
                        .type = TokenType::kNumberLiteral
                    } },
                });
            };

            static const StringHeteroHashMap<std::string> kStageMacros{
                { "vertex",      "GL_VERTEX_SHADER"             },
                { "fragment",    "GL_FRAGMENT_SHADER"           },
                { "tesscontrol", "GL_TESS_CONTROL_SHADER"       },
                { "tesseval",    "GL_TESS_EVALUATION_SHADER"    },
                { "geometry",    "GL_GEOMETRY_SHADER"           },
                { "compute",     "GL_COMPUTE_SHADER"            },
                { "raygen",      "GL_RAY_GENERATION_SHADER_EXT" },
                { "intersect",   "GL_INTERSECTION_SHADER_EXT"   },
                { "anyhit",      "GL_ANY_HIT_SHADER_EXT"        },
                { "closest",     "GL_CLOSEST_HIT_SHADER_EXT"    },
                { "miss",        "GL_MISS_SHADER_EXT"           },
                { "callable",    "GL_CALLABLE_SHADER_EXT"       },
                { "task",        "GL_TASK_SHADER_EXT"           },
                { "mesh",        "GL_MESH_SHADER_EXT"           }
            };

            for (auto i = 0uz; i != raw_tokens.size(); ++i) {
                if (raw_tokens[i].type != TokenType::kSharp) {
                    continue;
                }

                auto j = i + 1;
                while (j < raw_tokens.size() && raw_tokens[j].type == TokenType::kSharp) {
                    ++j;
                }

                if (j >= raw_tokens.size()) {
                    break;
                }

                const auto& directive = raw_tokens[j];

                if (directive.text == "version" &&
                    j + 1 < raw_tokens.size() &&
                    raw_tokens[j + 1].type == TokenType::kNumberLiteral)
                {
                    result.version = raw_tokens[j + 1].text;

                    i = j + 1;
                    continue;
                }

                if (directive.text == "extension" &&
                    j + 3 < raw_tokens.size() &&
                    raw_tokens[j + 2].type == TokenType::kColon)
                {
                    const auto& action = raw_tokens[j + 3];
                    if (action.text == "enable" || action.text == "require") {
                        InjectMacro(raw_tokens[j + 1].text);
                    }

                    i = j + 3;
                    continue;
                }

                if (directive.text == "pragma" && j + 2 < raw_tokens.size() &&
                    raw_tokens[j + 1].text == "shader_stage" &&
                    raw_tokens[j + 2].type == TokenType::kOpenParen) {
                    for (auto k = j + 3; k < raw_tokens.size() && raw_tokens[k].type != TokenType::kCloseParen; ++k) {
                        if (raw_tokens[k].type == TokenType::kIdentifier) {
                            auto it = kStageMacros.find(raw_tokens[k].text);
                            if (it != kStageMacros.end()) {
                                InjectMacro(it->second);
                            }
                        }
                    }

                    i = j + 3;
                    continue;
                }
            }

            return result;
        }

        std::string ResolveExtensionFilename(std::string_view extension) {
            const auto first = extension.find('_');
            if (first == std::string_view::npos) {
                return "";
            }

            const auto second = extension.find('_', first + 1);
            const auto vendor = second == std::string_view::npos
                              ? extension.substr(first + 1)
                              : extension.substr(first + 1, second - first - 1);

            return std::format("Database/Meta/Extensions/Main/{}/{}.glsl", vendor, extension);
        }

        struct CollectResult {
            std::vector<std::filesystem::path> required_files;
            MacroCollectResult                 macro_collect_result;
        };

        CollectResult CollectRequiredMetadataFiles(std::span<const Token> raw_tokens) {
            std::vector<std::filesystem::path> required_files;

            auto PushIfExists = [&required_files](std::string_view relative_path) {
                auto resolved = TryResolveMetadataFile(relative_path);
                if (resolved.has_value()) {
                    required_files.push_back(std::move(*resolved));
                }
            };

            PushIfExists("Database/Meta/BuiltinFunctions.glsl");

            static const StringHeteroHashMap<std::string> kMacroStages{
                { "GL_VERTEX_SHADER",             "Vertex",         },
                { "GL_FRAGMENT_SHADER",           "Fragment",       },
                { "GL_TESS_CONTROL_SHADER",       "TessControl",    },
                { "GL_TESS_EVALUATION_SHADER",    "TessEvaluation", },
                { "GL_GEOMETRY_SHADER",           "Geometry",       },
                { "GL_COMPUTE_SHADER",            "Compute",        },
                { "GL_RAY_GENERATION_SHADER_EXT", "RayGeneration",  },
                { "GL_INTERSECTION_SHADER_EXT",   "Intersection",   },
                { "GL_ANY_HIT_SHADER_EXT",        "AnyHit",         },
                { "GL_CLOSEST_HIT_SHADER_EXT",    "ClosestHit",     },
                { "GL_MISS_SHADER_EXT",           "Miss",           },
                { "GL_CALLABLE_SHADER_EXT",       "Callable",       },
                { "GL_TASK_SHADER_EXT",           "Task",           },
                { "GL_MESH_SHADER_EXT",           "Mesh",           }
            };

            auto [injected_macros, version] = CollectRequestedExtensionsAndVersion(raw_tokens);
            for (const auto& [name, _] : injected_macros) {
                auto it = kMacroStages.find(name);
                if (it != kMacroStages.end()) { // shader_stage
                    PushIfExists(std::format("Database/Meta/BuiltinVariables/{}.glsl", it->second));
                } else {
                    PushIfExists(ResolveExtensionFilename(name));
                }
            }

            std::ranges::sort(required_files);
            auto [first, last] = std::ranges::unique(required_files);
            required_files.erase(first, last);

            return {
                .required_files = std::move(required_files),
                .macro_collect_result = {
                    .injected_macros = std::move(injected_macros),
                    .version         = version
                }
            };
        }
    }

    MetadataManager::MetadataManager()
        : include_loader_{ source_table_, thread_pool_ }
    {}

    void MetadataManager::AttachBuiltinMetadata(
        Document& target,
        std::optional<std::string> shader_stage,
        std::span<const Token> raw_tokens,
        IncludeDirectoryHandle include_dirs)
    {
        target.InjectMacro("_GLSLD");
        target.InjectMacro("__LINE__");
        target.InjectMacro("__FILE__");

        static const StringHeteroHashMap<std::string> kMacros{
            { "vert",  "GL_VERTEX_SHADER"             },
            { "frag",  "GL_FRAGMENT_SHADER"           },
            { "comp",  "GL_COMPUTE_SHADER"            },
            { "geom",  "GL_GEOMETRY_SHADER"           },
            { "tesc",  "GL_TESS_CONTROL_SHADER"       },
            { "tese",  "GL_TESS_EVALUATION_SHADER"    },
            { "mesh",  "GL_MESH_SHADER_EXT"           },
            { "task",  "GL_TASK_SHADER_EXT"           },
            { "rgen",  "GL_RAY_GENERATION_SHADER_EXT" },
            { "rahit", "GL_ANY_HIT_SHADER_EXT"        },
            { "rchit", "GL_CLOSEST_HIT_SHADER_EXT"    },
            { "rmiss", "GL_MISS_SHADER_EXT"           },
            { "rint",  "GL_INTERSECTION_SHADER_EXT"   },
            { "rcall", "GL_CALLABLE_SHADER_EXT"       },
        };

        static const StringHeteroHashMap<std::string> kStages{
            { "vert",  "Vertex"         },
            { "frag",  "Fragment"       },
            { "comp",  "Compute"        },
            { "geom",  "Geometry"       },
            { "tesc",  "TessControl"    },
            { "tese",  "TessEvaluation" },
            { "mesh",  "Mesh"           },
            { "task",  "Task"           },
            { "rgen",  "RayGeneration"  },
            { "rahit", "AnyHit"         },
            { "rchit", "ClosestHit"     },
            { "rmiss", "Miss"           },
            { "rint",  "Intersection"   },
            { "rcall", "Callable"       },
        };

        auto [required_files, macro_collect_result] = CollectRequiredMetadataFiles(raw_tokens);
        const auto [injected_macros, version]       = std::move(macro_collect_result);

        target.InjectMacro(MacroDefinition{
            .is_function = false,
            .original_token = Token{
                .text = "__VERSION__",
                .type = TokenType::kIdentifier
            },
            .replacement_list = { Token{
                .text = version,
                .type = TokenType::kNumberLiteral
            } }
        });

        auto macro_it = kMacros.find(shader_stage.value_or(""));
        if (macro_it != kMacros.end()) {
            bool has_stage = false;
            for (const auto& [name, _] : injected_macros) {
                if (kMacros.contains(name)) {
                    has_stage = true;
                    break;
                }
            }

            if (!has_stage) {
                target.InjectMacro(macro_it->second);
            }

            auto stage_it = kStages.find(shader_stage.value_or(""));
            if (stage_it != kStages.end()) {
                const auto builtin_filename = std::format("Database/Meta/BuiltinVariables/{}.glsl", stage_it->second);
                auto resolved = TryResolveMetadataFile(builtin_filename);
                if (resolved.has_value()) {
                    required_files.push_back(std::move(*resolved));
                }
            }
        }

        for (auto [_, definition] : injected_macros) {
            target.InjectMacro(std::move(definition));
        }

        for (const auto& path : required_files) {
            auto result = EnsureBuiltinDocumentLoaded(path, include_dirs, &injected_macros);
            // use dot operator to suppress IntelliSense error static analysis
            if (result.document == nullptr) {
                continue;
            }

            for (const auto& [_, definition] : result.document->macro_table) {
                target.InjectMacro(definition);
            }

            target.metadata_attachments.push_back({
                .filename   = std::move(result.filename),
                .cached_key = std::move(result.cached_key)
            });

            auto* target_root = target.symbols.root_scope();
            target_root->AddBuiltinScope(result.document->symbols.root_scope());
            target.symbols.AttachBuiltinSymbols(&result.document->symbols);
            target.builtins.push_back(result.document);
        }
    }

    void MetadataManager::InvalidateMetadata(Document& document) {
        std::lock_guard lock(builtin_mutex_);

        for (const auto& attachment : document.metadata_attachments) {
            auto file_it = builtin_documents_.find(attachment.filename);
            if (file_it == builtin_documents_.end()) {
                continue;
            }

            auto variant_it = file_it->second.find(attachment.cached_key);
            if (variant_it == file_it->second.end()) {
                continue;
            }

            auto& variant = variant_it->second;
            assert(variant.ref_count > 0);

            if (--variant.ref_count == 0)
                file_it->second.erase(variant_it);
            if (file_it->second.empty())
                builtin_documents_.erase(file_it);
        }

        document.metadata_attachments.clear();
    }

    const StringHeteroHashMap<TokenType>* MetadataManager::GetLexicalTable() {
        EnsureLexicalLoaded();

        std::shared_lock lock(lexical_mutex_);
        return &lexical_table_;
    }

    std::optional<std::string_view> MetadataManager::GetLexicalSubtype(std::string_view word) {
        EnsureLexicalLoaded();

        std::shared_lock lock(lexical_mutex_);
        auto it = lexical_entries_.find(word);
        if (it == lexical_entries_.end()) {
            return std::nullopt;
        }

        return it->second.subtype;
    }

    const std::vector<std::pair<std::string, std::string>>& MetadataManager::GetMeta() {
        EnsureLexicalLoaded();
        return meta_;
    }

    bool MetadataManager::IsNoExpandHint(std::string_view word) const {
        return no_expand_hints_.contains(word);
    }

    MetadataManager& MetadataManager::GetInstance() {
        static MetadataManager instance;
        return instance;
    }

    void MetadataManager::EnsureLexicalLoaded() {
        if (lexical_loaded_.load(std::memory_order::relaxed)) {
            return;
        }

        std::unique_lock lock(lexical_mutex_);
        if (lexical_loaded_.load(std::memory_order::relaxed)) {
            return;
        }

        lexical_entries_.clear();
        lexical_table_.clear();

        const auto lexical_root = std::filesystem::path(Utils::GetFilePath("Database/Lexicals"));
        std::error_code ec;
        if (!std::filesystem::exists(lexical_root, ec) || ec) {
            throw std::runtime_error("Lexical metadata directory does not exist: " + lexical_root.string());
        }

        for (const auto& entry : std::filesystem::recursive_directory_iterator(lexical_root)) {
            if (!entry.is_regular_file()) {
                continue;
            }

            const auto filename = entry.path().filename().string();
            if (!EndsWithCaseInsensitive(filename, ".txt")) {
                continue;
            }

            const auto relative_path = std::filesystem::relative(entry.path(), lexical_root).generic_string();
            LoadLexicalMetadata(entry.path(), relative_path);
        }

        lexical_loaded_.store(true, std::memory_order::relaxed);
        LoadNoExpandHints();

        for (const auto& [name, entry] : lexical_entries_) {
            const auto& subtype = entry.subtype;
            if (!subtype.empty()) {
                meta_.emplace_back(subtype, name);
            }
        }
    }

    MetadataManager::BuiltinDocumentResult MetadataManager::EnsureBuiltinDocumentLoaded(
        const std::filesystem::path& path,
        IncludeDirectoryHandle include_dirs,
        const MacroTable* injected_macros)
    {
        const auto normalized = Utils::NormalizePath(path);
        auto       filename   = normalized.generic_string();

        std::error_code ec;
        const auto latest = std::filesystem::last_write_time(normalized, ec);
        if (ec) {
            return {};
        }

        const auto time_epoch = latest.time_since_epoch().count();
        auto cached_key = std::format(
            "{}_{}", time_epoch, injected_macros == nullptr ? "NO_MACRO" : MakeMacroFingerprint(*injected_macros));

        {
            std::shared_lock lock(builtin_mutex_);
            auto it = builtin_documents_.find(filename);
            if (it != builtin_documents_.end()) {
                auto it2 = it->second.find(cached_key);
                if (it2 != it->second.end() && it2->second.document != nullptr) {
                    return {
                        .document   = it2->second.document,
                        .filename   = std::move(filename),
                        .cached_key = std::move(cached_key)
                    };
                }
            }
        }

        auto parsed = ParseMetadataDocument(normalized, include_dirs, injected_macros);
        if (parsed == nullptr || parsed->ast == nullptr) {
            return {};
        }

        std::lock_guard lock(builtin_mutex_);
        auto& cache = builtin_documents_[filename];

        cache.try_emplace(cached_key, BuiltinDocumentVariant{
            .document  = std::move(parsed),
            .ref_count = 1
        });

        return {
            .document   = cache[cached_key].document,
            .filename   = std::move(filename),
            .cached_key = std::move(cached_key)
        };
    }

    namespace {
        std::vector<std::string_view> ExtractWords(std::string_view text) {
            auto words_range = text | std::views::chunk_by([](auto lhs, auto rhs) -> bool {
                return (Unicode::IsAsciiSpace(static_cast<unsigned char>(lhs)) ==
                        Unicode::IsAsciiSpace(static_cast<unsigned char>(rhs)));
            }) | std::views::filter([](auto chunk) -> bool {
                return !Unicode::IsAsciiSpace(static_cast<unsigned char>(chunk.front()));
            }) | std::views::transform([](auto word_view) -> std::string_view {
                return std::string_view(word_view);
            });

            return std::ranges::to<std::vector<std::string_view>>(words_range);
        }

        TokenType ResolveTokenType(std::string_view relative_path) {
            if (relative_path == "Builtins/Arithmetics.txt" || relative_path == "Builtins/Opaques.txt")
                return TokenType::kBuiltInType;
            if (relative_path == "Keywords/Meta.txt")
                return TokenType::kKeyword;
            if (relative_path == "Preprocessors/Meta.txt")
                return TokenType::kPreprocessor;
            if (relative_path == "Primitives/SpirvIntrinsics.txt")
                return TokenType::kSpirvIntrinsic;
            if (relative_path.starts_with("Primitives/"))
                return TokenType::kPrimitive;
            return TokenType::kUnknown;
        }

        std::string BuildSubtype(std::string_view relative_path) {
            if (relative_path.ends_with(".txt")) {
                relative_path.remove_suffix(4);
            }

            std::string result(relative_path);
            std::ranges::replace(result, '/', '.');
            return result;
        }
    }

    void MetadataManager::LoadLexicalMetadata(const std::filesystem::path& path, std::string_view relative_path) {
        const auto source = Utils::LoadSource(path);
        if (!source.has_value()) {
            throw std::runtime_error("Failed to load lexical metadata: " + source.error());
        }

        const auto words      = ExtractWords(*source);
        const auto token_type = ResolveTokenType(relative_path);
        const auto subtype    = BuildSubtype(relative_path);

        for (auto word : words) {
            if (word.empty()) {
                continue;
            }

            lexical_table_.try_emplace(word, token_type);
            lexical_entries_[word] = LexicalEntry{
                .type    = token_type,
                .subtype = subtype
            };
        }
    }

    std::shared_ptr<Document> MetadataManager::ParseMetadataDocument(
        const std::filesystem::path& path,
        IncludeDirectoryHandle include_dirs,
        const MacroTable* injected_macros)
    {
        const auto normalized = Utils::NormalizePath(path);
        const auto filename   = normalized.generic_string();
        const auto uri        = Utils::PathToUri(normalized);

        auto source = Utils::LoadSource(normalized);
        if (!source.has_value()) {
            throw std::runtime_error("Failed to load metadata: " + source.error());
        }

        auto document = std::make_shared<Document>();
        document->source = std::move(*source);
        document->arena  = arena_pool_.Acquire();

        if (injected_macros != nullptr) {
            for (const auto& [_, definition] : *injected_macros) {
                document->InjectMacro(definition);
            }
        }

        const auto* source_file = source_table_.Intern(filename, uri);
        Lexer lexer(source_file, document->source, include_loader_, include_dirs);
        auto raw_tokens = lexer.Tokenize();

        Parser parser(*document, source_table_, source_file, std::move(raw_tokens), include_loader_, include_dirs, 0, nullptr);

        if (document->ast == nullptr) {
            return nullptr;
        }

        document->FinalizeInjectedMacros(source_file);

        SymbolLinker linker(*document, 0, nullptr);
        TypeResolver resolver(*document, 0, nullptr);
        MacroBinder binder(*document, 0, nullptr);

        return document;
    }

    void MetadataManager::LoadNoExpandHints() {
        const auto path   = Utils::GetFilePath("Database/NoExpandHints.txt");
        const auto source = Utils::LoadSource(path);
        if (!source.has_value()) {
            throw std::runtime_error("Failed to load no-expand hints: " + source.error());
        }

        const auto words = ExtractWords(*source);
        for (auto word : words) {
            no_expand_hints_.insert(word);
        }
    }
}
