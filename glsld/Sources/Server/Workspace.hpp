#pragma once

#include <atomic>
#include <filesystem>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "Analyzer/Syntax/Document.hpp"
#include "Base/FileSystem/IncludeLoader.hpp"
#include "Base/FileSystem/Source.hpp"
#include "Base/Hash.hpp"
#include "Base/ThreadPool.hpp"

namespace glsld {
    struct ExtraShaderConfig {
        std::optional<std::string> version;
        std::optional<std::string> shader_stage;
        std::optional<std::string> target_env;
        std::optional<std::string> target_spv;
    };

    class Workspace {
    public:
        explicit Workspace(ThreadPool& thread_pool);

        const SourceFile* InternSource(std::string_view uri);
        const SourceFile* GetSource(std::string_view uri) const;

        void UpdateDocument(
            std::string_view uri,
            std::string_view source,
            int version_replica,
            std::shared_ptr<const std::atomic<int>> version_pointer,
            bool open_document = false);

        void RemoveDocument(std::string_view uri);
        std::shared_ptr<Document> GetDocumentSnapshot(std::string_view uri) const;
        std::vector<std::string> GetAffectedDocuments(std::string_view changed_uri) const;

        void InvalidateInclude(std::string_view uri);

        void AddIncludeDirectory(std::filesystem::path directory);
        void RemoveIncludeDirectory(const std::filesystem::path& directory);

        void set_include_dirs(std::vector<std::filesystem::path> include_dirs);
        std::span<const std::filesystem::path> include_dirs() const;
        const StringHeteroHashMap<ExtraShaderConfig>& shader_configs() const;

    private:
        friend class LspServer;

        void ProcessSource(
            const SourceFile* source_file,
            std::string_view source,
            int version_replica,
            std::shared_ptr<const std::atomic<int>> version_pointer,
            Document& document);

        void UnregisterDependencies(std::string_view uri);
        void UpdateDependencies(std::string_view uri, std::shared_ptr<const Document> document);
        void RemoveDependencies(std::string_view uri);

        StringHeteroHashMap<std::shared_ptr<Document>> documents_;
        StringHeteroHashMap<ExtraShaderConfig>         shader_configs_; // [Uri, Config]
        ThreadPool&                                    thread_pool_;
        SourceTable                                    source_table_;
        IncludeLoader                                  include_loader_;
        std::vector<std::filesystem::path>             include_dirs_;
        StringHeteroHashMap<std::vector<std::string>>  forward_dependencies_;
        StringHeteroHashMap<StringHeteroHashSet>       reverse_dependencies_;
        mutable std::shared_mutex                      mutex_;
    };
}

#include "Workspace.inl"
