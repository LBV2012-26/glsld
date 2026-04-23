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
    }

    MetadataManager::MetadataManager()
        : include_loader_{ source_table_, thread_pool_ }
    {}

    void MetadataManager::MergeBuiltinMetadata(Document& target, std::span<const std::filesystem::path> include_dirs) {}

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
    }

    void MetadataManager::EnsureBuiltinDocumentLoaded(
        const std::filesystem::path& path,
        std::span<const std::filesystem::path> include_dirs)
    {
        auto normalized = utils::NormalizePath(path);
        auto filename   = normalized.generic_string();

        std::error_code ec;
        auto latest = std::filesystem::last_write_time(normalized, ec);
        if (ec) {
            return;
        }

        {
            std::shared_lock lock(builtin_mutex_);
            auto it = builtin_documents_.find(filename);
            if (it != builtin_documents_.end() &&
                it->second.document != nullptr &&
                it->second.write_time == latest)
            {
                return;
            }
        }

        auto parsed = ParseMetadataDocument(normalized, include_dirs);
        if (parsed == nullptr || parsed->ast == nullptr) {
            return;
        }

        std::unique_lock lock(builtin_mutex_);
        builtin_documents_[filename] = BuiltinDocumentCache{
            .write_time = latest,
            .document   = std::move(parsed)
        };
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

    std::vector<std::filesystem::path> MetadataManager::CollectRequiredMetadataFiles(const Document& target) const {
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
        for (const auto& extension : required_extensions) {
            auto filename = std::format("Assets/Meta/ExtensionHeaders/{}.glsl", extension);
            PushIfExists(filename);
        }

        std::ranges::sort(required_files);
        auto [first, last] = std::ranges::unique(required_files);
        required_files.erase(first, last);
        return required_files;
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
        std::span<const std::filesystem::path> include_dirs)
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
}
