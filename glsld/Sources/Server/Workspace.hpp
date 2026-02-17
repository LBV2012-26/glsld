#pragma once

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
        void UpdateDocument(std::string_view uri, std::string_view context, int version);
        void RemoveDocument(std::string_view uri);
        std::shared_ptr<Document> GetDocumentSnapshot(std::string_view uri) const;

    private:
        utils::StringHeteroHashTable<std::string, std::shared_ptr<Document>> documents_;
        mutable std::shared_mutex mutex_;
    };
}
