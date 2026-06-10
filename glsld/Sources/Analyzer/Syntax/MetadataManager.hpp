#pragma once

#include <atomic>
#include <filesystem>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <span>
#include <string>
#include <string_view>
#include <thread>

#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Base/FileSystem/IncludeLoader.hpp"
#include "Base/FileSystem/Source.hpp"
#include "Base/Hash.hpp"
#include "Base/ThreadPool.hpp"

namespace glsld {
    struct LexicalEntry {
        TokenType type{ TokenType::kUnknown };
        std::string subtype;
    };

    class MetadataManager {
    public:
        void AttachBuiltinMetadata(Document& target, std::span<const std::filesystem::path> include_dirs);

        const StringHeteroHashMap<TokenType>* GetLexicalTable();
        std::optional<std::string_view> GetLexicalSubtype(std::string_view word);
        bool IsNoExpandHint(std::string_view word) const;

        static MetadataManager& GetInstance();

    private:
        MetadataManager();
        MetadataManager(const MetadataManager&) = delete;
        MetadataManager(MetadataManager&&)      = delete;
        ~MetadataManager()                      = default;

        MetadataManager& operator=(const MetadataManager&) = delete;
        MetadataManager& operator=(MetadataManager&&)      = delete;

        void EnsureLexicalLoaded();

        std::shared_ptr<Document> EnsureBuiltinDocumentLoaded(
            const std::filesystem::path& path,
            std::span<const std::filesystem::path> include_dirs,
            const MacroTable* injected_macros);

        struct CollectResult {
            std::vector<std::filesystem::path> required_filenames;
            MacroTable                         injected_macros;
        };

        CollectResult CollectRequiredMetadataFiles(const Document& target) const;

        void LoadLexicalMetadata(const std::filesystem::path& path, std::string_view relative_path);

        std::shared_ptr<Document> ParseMetadataDocument(
            const std::filesystem::path& path,
            std::span<const std::filesystem::path> include_dirs,
            const MacroTable* injected_macros);

        void LoadNoExpandHints();

        struct BuiltinDocumentCache {
            std::filesystem::file_time_type                write_time{};
            StringHeteroHashMap<std::shared_ptr<Document>> variants;
        };

        StringHeteroHashMap<LexicalEntry>         lexical_entries_;
        StringHeteroHashMap<TokenType>            lexical_table_;
        StringHeteroHashMap<BuiltinDocumentCache> builtin_documents_;
        StringHeteroHashSet                       no_expand_hints_;
        std::shared_mutex                         lexical_mutex_;
        std::shared_mutex                         builtin_mutex_;
        std::atomic<bool>                         lexical_loaded_{ false };
        ThreadPool                                thread_pool_{ std::thread::hardware_concurrency() };
        SourceTable                               source_table_;
        IncludeLoader                             include_loader_;
    };
}
