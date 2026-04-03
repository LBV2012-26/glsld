#pragma once

#include <atomic>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>

#include "Analyzer/Syntax/Document.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    class Workspace {
    public:
        void UpdateDocument(std::string_view uri, std::string_view context, int version_replica,
                            std::shared_ptr<const std::atomic<int>> version, bool open_document = false);

        void RemoveDocument(std::string_view uri);
        std::shared_ptr<Document> GetDocumentSnapshot(std::string_view uri) const;

    private:
        StringHeteroHashTable<std::shared_ptr<Document>> documents_;
        mutable std::shared_mutex                        mutex_;
    };
}
