#include "stdafx.h"
#include "Document.hpp"

#include "Base/Hash.hpp"

namespace glsld {
    std::size_t LocationHash::operator()(SourceLocation location) const {
        auto seed = 0uz;
        HashCombine(seed, std::hash<std::size_t>{}(location.line));
        HashCombine(seed, std::hash<std::size_t>{}(location.column));
        HashCombine(seed, std::hash<std::string>{}(location.source->normalized_path));
        HashCombine(seed, std::hash<std::string>{}(location.source->uri));
        return seed;
    }

    SourceReference FileTable::Intern(std::string_view normalized_path, std::string_view uri) {
        auto it = sources_.find(normalized_path);
        if (it != sources_.end()) {
            return it->second;
        }

        auto source = std::make_shared<SourceFile>(std::string(normalized_path), std::string(uri));

        sources_.emplace(source->normalized_path, source);
        return source;
    }
}
