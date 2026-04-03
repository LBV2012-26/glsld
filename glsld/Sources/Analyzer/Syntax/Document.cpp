#include "stdafx.h"
#include "Document.hpp"

#include "Base/Hash.hpp"

namespace glsld {
    std::size_t LocationHash::operator()(SourceLocation location) const {
        auto hash1 = std::hash<std::size_t>{}(location.line);
        auto hash2 = std::hash<std::size_t>{}(location.column);
        auto hash3 = std::hash<std::uint32_t>{}(location.file_index);

        auto seed = 0uz;
        HashCombine(seed, hash1);
        HashCombine(seed, hash2);
        HashCombine(seed, hash3);
        return seed;
    }

    std::uint32_t FileTable::Intern(std::string_view uri, std::string_view normalized_path) {
        auto it = path_index_map.find(normalized_path);
        if (it != path_index_map.end()) {
            return it->second;
        }

        auto index = static_cast<std::uint32_t>(files.size());
        files.push_back({
            .index           = index,
            .uri             = std::string(uri),
            .normalized_path = std::string(normalized_path),
        });

        path_index_map.emplace(normalized_path, index);
        uri_index_map.emplace(uri, index);

        return index;
    }

    const FileEntry* FileTable::FindByIndex(std::uint32_t index) const {
        if (index >= files.size()) {
            return nullptr;
        }

        return &files[index];
    }

    std::optional<std::uint32_t> FileTable::FindByUri(std::string_view uri) const {
        auto it = uri_index_map.find(uri);
        if (it == uri_index_map.end()) {
            return std::nullopt;
        }

        return it->second;
    }
}
