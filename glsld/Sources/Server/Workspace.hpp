#pragma once

#include <cstdint>
#include <condition_variable>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <span>
#include <stop_token>
#include <string>
#include <string_view>
#include <vector>

#include "Analyzer/Syntax/Document.hpp"
#include "Base/FileSystem/IncludeLoader.hpp"
#include "Base/FileSystem/Source.hpp"
#include "Base/Arena.hpp"
#include "Base/Hash.hpp"
#include "Base/ThreadPool.hpp"
#include "Server/Index/GlobalIndex.hpp"
#include "Server/Index/IndexCache.hpp"

namespace glsld {
    struct ExtraShaderConfig {
        std::optional<std::string> version;
        std::optional<std::string> shader_stage;
        std::optional<std::string> target_env;
        std::optional<std::string> target_spv;
    };

    struct ActiveMacro {
        std::string name;
        std::string replacement;
    };

    struct ActiveVariant {
        std::string              variant_name;
        std::vector<ActiveMacro> macros;
    };

    enum class VariantType {
        kShared,
        kUnique
    };

    class Workspace {
    public:
        const SourceFile* InternSource(std::string_view uri);
        const SourceFile* GetSource(std::string_view uri) const;

        void UpdateDocument(
            std::string_view uri,
            std::string_view source,
            int version_replica,
            VersionPointer version_pointer);

        std::shared_ptr<Document> GetDocumentSnapshot(std::string_view uri) const;
        std::vector<std::string> GetAffectedDocuments(std::string_view changed_uri) const;

        void InvalidateInclude(std::string_view uri);

        void AddIncludeDirectory(std::filesystem::path directory);
        void RemoveIncludeDirectory(const std::filesystem::path& directory);

        void AddExtraShaderConfig(std::string_view key, ExtraShaderConfig config);
        void RemoveExtraShaderConfig(std::string_view key);

        void ApplyVariants(std::optional<ActiveVariant> shared, StringHeteroHashMap<ActiveVariant> unique);
        void ChangeVariant(VariantType type, ActiveVariant variant, std::string_view uri = "");
        void RemoveVariant(VariantType type, std::string_view uri = "");

        void StartBackgroundIndex(
            std::vector<std::filesystem::path> roots,
            std::filesystem::path cache_path,
            std::string cache_key);

        void StopBackgroundIndex();

        void MarkDocumentOpen(std::string_view uri);
        void CloseDocument(std::string_view uri);

        void ScheduleDiskIndex(const std::filesystem::path& filename);
        void ScheduleDiskIndexByUri(std::string_view uri);

        void set_include_dirs(IncludeDirectoryHandle include_dirs);
        IncludeDirectoryHandle include_dirs() const;
        const StringHeteroHashMap<ExtraShaderConfig>& shader_configs() const;
        const GlobalIndex& global_index() const;
        const TypeMemberIndex& type_member_index() const;

    private:
        void ProcessSource(
            Document& document,
            const SourceFile* source_file,
            std::string_view source,
            int version_replica,
            VersionPointer version_pointer);

        void InjectVariantMacro(Document& document, const SourceFile* source_file, const ActiveMacro& macro);

        void UnregisterDependencies(std::string_view uri);
        void UpdateDependencies(std::string_view uri, std::span<const std::string> dependencies);
        void UpdateDependencies(std::string_view uri, std::shared_ptr<const Document> document);
        void RemoveDependencies(std::string_view uri);

        void BackgroundIndexLoop(std::stop_token stop_token);
        void LoadBackgroundCache();
        void ReconcileWorkspace();

        void ProcessDiskIndexTask(
            std::string_view uri,
            const std::filesystem::path& filename,
            std::uint64_t revision);

        void FlushBackgroundCache();

        std::vector<std::filesystem::path> DiscoverIndexCandidates() const;
        bool IsIndexCandidate(const std::filesystem::path& filename) const;

        struct DiskIndexTask {
            std::string           uri;
            std::filesystem::path filename;
        };

        StringHeteroHashMap<std::shared_ptr<Document>> documents_;
        StringHeteroHashMap<ExtraShaderConfig>         shader_configs_; // [Uri, Config]
        SourceTable                                    source_table_;

        ArenaPool                                      arena_pool_;
        ThreadPool                                     loader_pool_{ std::jthread::hardware_concurrency() };
        IncludeLoader                                  include_loader_{ source_table_, loader_pool_ };
        IncludeDirectoryHandle                         include_dirs_{ std::make_shared<std::vector<std::filesystem::path>>() };

        StringHeteroHashMap<std::vector<std::string>>  forward_dependencies_;
        StringHeteroHashMap<StringHeteroHashSet>       reverse_dependencies_;
        StringHeteroHashMap<ActiveVariant>             active_variants_;
        std::optional<ActiveVariant>                   shared_variant_;
        mutable std::mutex                             dependency_mutex_;
        mutable std::shared_mutex                      document_mutex_;
        std::shared_mutex                              variant_mutex_;

        GlobalIndex                                    global_index_;
        TypeMemberIndex                                type_member_index_;
        std::mutex                                     index_mutex_;
        std::vector<std::filesystem::path>             index_roots_;
        std::filesystem::path                          index_cache_path_;
        std::string                                    index_cache_key_;

        StringHeteroHashMap<DiskIndexRecord>           disk_index_records_;
        StringHeteroHashMap<std::uint64_t>             index_revisions_;
        StringHeteroHashSet                            open_document_uris_;
        StringHeteroHashSet                            queued_disk_uris_;
        std::queue<DiskIndexTask>                      index_task_queue_;

        ThreadPool                                     background_index_pool_{ std::jthread::hardware_concurrency() };
        std::mutex                                     background_index_mutex_;
        std::condition_variable_any                    background_index_condition_;
        std::jthread                                   background_index_thread_;
        bool                                           background_cache_dirty_{ false };
    };
}

#include "Workspace.inl"
