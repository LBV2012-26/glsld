#pragma once

#include <atomic>
#include <filesystem>
#include <memory>
#include <shared_mutex>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "Analyzer/Syntax/Document.hpp"
#include "Base/Hash.hpp"
#include "Base/IncludeLoader.hpp"
#include "Base/Source.hpp"
#include "Base/ThreadPool.hpp"

namespace glsld {
    class Workspace {
    public:
        explicit Workspace(ThreadPool& thread_pool);

        const SourceFile* InternSource(std::string_view uri);
        const SourceFile* GetSource(std::string_view uri) const;

        void UpdateDocument(
            std::string_view uri,
            std::string_view context,
            int version_replica,
            std::shared_ptr<const std::atomic<int>> version,
            bool open_document = false);

        void RemoveDocument(std::string_view uri);
        std::shared_ptr<Document> GetDocumentSnapshot(std::string_view uri) const;

        void AddIncludeDirectory(std::filesystem::path directory);
        void RemoveIncludeDirectory(const std::filesystem::path& directory);

        void set_include_dirs(std::vector<std::filesystem::path> include_dirs);
        std::span<const std::filesystem::path> include_dirs() const;

    private:
        StringHeteroHashTable<std::shared_ptr<Document>> documents_;
        ThreadPool&                                      thread_pool_;
        SourceTable                                      source_table_;
        IncludeLoader                                    include_loader_;
        std::vector<std::filesystem::path>               include_dirs_;
        mutable std::shared_mutex                        mutex_;
    };
}

#include "Workspace.inl"
