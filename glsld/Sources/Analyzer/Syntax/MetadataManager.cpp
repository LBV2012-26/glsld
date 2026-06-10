#include "stdafx.h"
#include "MetadataManager.hpp"

#include <cctype>
#include <cstddef>
#include <algorithm>
#include <format>
#include <fstream>
#include <ios>
#include <ranges>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>

#include "Analyzer/Passes/MacroBinder.hpp"
#include "Analyzer/Passes/SymbolLinker.hpp"
#include "Analyzer/Passes/TypeResolver.hpp"
#include "Analyzer/Syntax/Parser.hpp"
#include "Base/FileSystem/Source.hpp"
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
    }

    MetadataManager::MetadataManager()
        : include_loader_{ source_table_, thread_pool_ }
    {}

    void MetadataManager::AttachBuiltinMetadata(Document& target, std::span<const std::filesystem::path> include_dirs) {
        auto [required_filenames, injected_macros] = CollectRequiredMetadataFiles(target);

        for (const auto& [name, defination] : injected_macros) {
            target.InjectMacro(name, defination);
        }

        for (const auto& path : required_filenames) {
            std::shared_ptr<Document> source = EnsureBuiltinDocumentLoaded(path, include_dirs, &injected_macros);
            if (source == nullptr) {
                continue;
            }

            for (const auto& [name, defination] : source->macros) {
                target.InjectMacro(name, defination);
            }

            auto* target_root = target.symbols.root_scope();
            target_root->AddBuiltinScope(source->symbols.root_scope());
            target.symbols.AttachBuiltinSymbols(&source->symbols);
        }
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

        auto lexical_root = std::filesystem::path(utils::GetFilePath("Assets/Lexicals"));
        std::error_code ec;
        if (!std::filesystem::exists(lexical_root, ec) || ec) {
            throw std::runtime_error("Lexical metadata directory does not exist: " + lexical_root.string());
        }

        for (const auto& entry : std::filesystem::recursive_directory_iterator(lexical_root)) {
            if (!entry.is_regular_file()) {
                continue;
            }

            auto filename = entry.path().filename().string();
            if (!EndsWithCaseInsensitive(filename, ".txt")) {
                continue;
            }

            auto relative_path = std::filesystem::relative(entry.path(), lexical_root).generic_string();
            LoadLexicalMetadata(entry.path(), relative_path);
        }

        lexical_loaded_.store(true, std::memory_order::relaxed);
        LoadNoExpandHints();
    }

    std::shared_ptr<Document> MetadataManager::EnsureBuiltinDocumentLoaded(
        const std::filesystem::path& path,
        std::span<const std::filesystem::path> include_dirs,
        const MacroTable* injected_macros)
    {
        auto normalized = utils::NormalizePath(path);
        auto filename   = normalized.generic_string();

        std::error_code ec;
        auto latest = std::filesystem::last_write_time(normalized, ec);
        if (ec) {
            return nullptr;
        }

        auto cached_key = injected_macros == nullptr ? "" : MakeMacroFingerprint(*injected_macros);

        {
            std::shared_lock lock(builtin_mutex_);
            auto it = builtin_documents_.find(filename);
            if (it != builtin_documents_.end() &&
                it->second.write_time == latest)
            {
                auto it2 = it->second.variants.find(cached_key);
                if (it2 != it->second.variants.end() && it2->second != nullptr) {
                    return it2->second;
                }
            }
        }

        auto parsed = ParseMetadataDocument(normalized, include_dirs, injected_macros);
        if (parsed == nullptr || parsed->ast == nullptr) {
            return nullptr;
        }

        std::unique_lock lock(builtin_mutex_);
        BuiltinDocumentCache cache;
        cache.write_time = latest;
        cache.variants.try_emplace(cached_key, std::move(parsed));
        builtin_documents_.insert_or_assign(filename, std::move(cache));

        return builtin_documents_[filename].variants[cached_key];
    }

    namespace {
        std::vector<std::string_view> ExtractWords(std::string_view text) {
            auto words_range = text | std::views::chunk_by([](auto lhs, auto rhs) -> bool {
                return (std::isspace(static_cast<unsigned char>(lhs)) ==
                        std::isspace(static_cast<unsigned char>(rhs)));
            }) | std::views::filter([](auto chunk) -> bool {
                return !std::isspace(static_cast<unsigned char>(chunk.front()));
            }) | std::views::transform([](auto word_view) -> std::string_view {
                return std::string_view(word_view);
            });

            return std::ranges::to<std::vector<std::string_view>>(words_range);
        }

        TokenType ResolveTokenType(std::string_view relative_path) {
            if (relative_path == "Builtins/Functions.txt")
                return TokenType::kBuiltInFunction;
            if (relative_path == "Builtins/Variables.txt")
                return TokenType::kBuiltInVariable;
            if (relative_path == "Builtins/Types.txt")
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

    namespace {
        std::optional<std::filesystem::path> TryResolveMetadataFile(const std::filesystem::path& relative_path) {
            auto path = std::filesystem::path(utils::GetFilePath(relative_path.generic_string()));
            std::error_code ec;
            if (!std::filesystem::exists(path, ec) || ec) {
                return std::nullopt;
            }

            return utils::NormalizePath(path);
        }

        std::vector<std::string> CollectRequestedExtensions(const Document& target) {
            if (target.ast == nullptr) {
                return {};
            }

            std::vector<std::string> results;
            for (const auto* node : target.ast->preprocessor_references) {
                if (node == nullptr || node->directive != "extension" || node->tokens.empty()) {
                    continue;
                }

                const auto& extension_token = node->tokens.front();
                if (extension_token.type != TokenType::kIdentifier && extension_token.type != TokenType::kKeyword) {
                    continue;
                }

                if (node->tokens.back().text != "require" && node->tokens.back().text != "enable") {
                    continue;
                }

                results.push_back(extension_token.text);
            }

            std::ranges::sort(results);
            auto [first, last] = std::ranges::unique(results);
            results.erase(first, last);
            return results;
        }
    }

    MetadataManager::CollectResult MetadataManager::CollectRequiredMetadataFiles(const Document& target) const {
        std::vector<std::filesystem::path> required_files;

        auto PushIfExists = [&required_files](std::string_view relative_path) {
            auto resolved = TryResolveMetadataFile(relative_path);
            if (resolved.has_value()) {
                required_files.push_back(*resolved);
            }
        };

        PushIfExists("Assets/Meta/BuiltinFunctions.glsl");
        PushIfExists("Assets/Meta/BuiltinVariables.glsl");

        auto required_extensions = CollectRequestedExtensions(target);
        MacroTable injected_macros;
        for (const auto& extension : required_extensions) {
            auto filename = std::format("Assets/Meta/ExtensionHeaders/{}.glsl", extension);
            PushIfExists(filename);

            injected_macros.try_emplace(extension, MacroDefination{
                .is_function    = false,
                .original_token = Token{
                    .text = extension,
                    .type = TokenType::kIdentifier
                },
                .replacement_list = { Token{
                    .text = "1",
                    .type = TokenType::kNumberLiteral
                } },
            });
        }

        static const StringHeteroHashMap<std::string> kStageMacros{
            { "vertex",         "GL_VERTEX_SHADER" },
            { "fragment",       "GL_FRAGMENT_SHADER" },
            { "compute",        "GL_COMPUTE_SHADER" },
            { "geometry",       "GL_GEOMETRY_SHADER" },
            { "tesscontrol",    "GL_TESS_CONTROL_SHADER" },
            { "tessevaluation", "GL_TESS_EVALUATION_SHADER" },
            { "mesh",           "GL_MESH_SHADER_EXT" },
            { "task",           "GL_TASK_SHADER_EXT" },
            { "raygen",         "GL_RAY_GENERATION_SHADER_EXT" },
            { "anyhit",         "GL_ANY_HIT_SHADER_EXT" },
            { "closesthit",     "GL_CLOSEST_HIT_SHADER_EXT" },
            { "miss",           "GL_MISS_SHADER_EXT" },
            { "intersection",   "GL_INTERSECTION_SHADER_EXT" },
            { "callable",       "GL_CALLABLE_SHADER_EXT" },
        };

        for (const auto* node : target.ast->preprocessor_references) {
            if (node == nullptr || node->directive != "pragma")
                continue;
            if (node->tokens.front().text != "shader_stage" || node->tokens.size() < 2)
                continue;

            auto begin = node->tokens.begin() + 1;
            auto end   = node->tokens.end();
            if (begin != end && begin->type == TokenType::kOpenParen)
                ++begin;
            if (end != begin && (end - 1)->type == TokenType::kCloseParen)
                --end;

            for (auto it = begin; it != end; ++it) {
                const auto& stage = it->text;
                const auto& macro = kStageMacros.find(stage);
                if (macro == kStageMacros.end()) {
                    continue;
                }

                injected_macros.try_emplace(macro->second, MacroDefination{
                    .is_function = false,
                    .original_token = Token{
                        .text = macro->second,
                        .type = TokenType::kIdentifier
                    },
                    .replacement_list = { Token{
                        .text = "1",
                        .type = TokenType::kNumberLiteral
                    } }
                });
            }
        }

        std::ranges::sort(required_files);
        auto [first, last] = std::ranges::unique(required_files);
        required_files.erase(first, last);

        return {
            .required_filenames = std::move(required_files),
            .injected_macros    = std::move(injected_macros)
        };
    }

    void MetadataManager::LoadLexicalMetadata(const std::filesystem::path& path, std::string_view relative_path) {
        auto [source, error] = LoadSource(path);
        if (!error.empty()) {
            throw std::runtime_error("Failed to load lexical metadata: " + error);
        }

        auto words      = ExtractWords(source);
        auto token_type = ResolveTokenType(relative_path);
        auto subtype    = BuildSubtype(relative_path);

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
        std::span<const std::filesystem::path> include_dirs,
        const MacroTable* injected_macros)
    {
        auto normalized = utils::NormalizePath(path);
        auto filename   = normalized.generic_string();
        auto uri        = utils::PathToUri(normalized);

        auto [source, error] = LoadSource(normalized);
        if (!error.empty()) {
            throw std::runtime_error("Failed to load metadata: " + error);
        }

        auto document = std::make_shared<Document>();
        document->source = std::move(source);

        if (injected_macros != nullptr) {
            document->macros = *injected_macros;
        }

        const auto* source_file = source_table_.Intern(filename, uri);
        Parser parser(source_table_, source_file, document->source, include_loader_, include_dirs, 0, nullptr, *document);

        if (document->ast == nullptr) {
            return nullptr;
        }

        SymbolLinker linker(*document, 0, nullptr);
        TypeResolver resolver(*document, 0, nullptr);
        MacroBinder binder(*document, 0, nullptr);

        return document;
    }

    void MetadataManager::LoadNoExpandHints() {
        auto path = utils::GetFilePath("Assets/NoExpandHints.txt");
        auto [source, error] = LoadSource(path);
        if (!error.empty()) {
            throw std::runtime_error("Failed to load no-expand hints: " + error);
        }

        auto words = ExtractWords(source);
        for (auto word : words) {
            no_expand_hints_.insert(word);
        }
    }
}
