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
#include <utility>
#include <vector>

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
        void AttachBuiltinMetadata(
            Document& target,
            std::optional<std::string> shader_stage,
            std::span<const Token> raw_tokens,
            IncludeDirectoryHandle include_dirs);

        const StringHeteroHashMap<TokenType>* GetLexicalTable();
        std::optional<std::string_view> GetLexicalSubtype(std::string_view word);
        const std::vector<std::pair<std::string, std::string>>& GetMeta();
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
            IncludeDirectoryHandle include_dirs,
            const MacroTable* injected_macros);

        void LoadLexicalMetadata(const std::filesystem::path& path, std::string_view relative_path);

        std::shared_ptr<Document> ParseMetadataDocument(
            const std::filesystem::path& path,
            IncludeDirectoryHandle include_dirs,
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
        std::vector<std::pair<std::string, std::string>> meta_; // [subtype, name]
        std::shared_mutex                         lexical_mutex_;
        std::shared_mutex                         builtin_mutex_;
        std::atomic<bool>                         lexical_loaded_{ false };
        ThreadPool                                thread_pool_{ std::thread::hardware_concurrency() };
        SourceTable                               source_table_{ SourceKind::kMetadata };
        IncludeLoader                             include_loader_;
    };
}
