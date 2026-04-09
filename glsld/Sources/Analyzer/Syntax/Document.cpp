#include "stdafx.h"
#include "Document.hpp"

namespace glsld {
    SourceReference FileTable::Intern(std::string_view filename, std::string_view uri) {
        auto it = sources_.find(filename);
        if (it != sources_.end()) {
            return it->second;
        }

        auto source = std::make_shared<SourceFile>(std::string(filename), std::string(uri));

        // sources_.emplace(source->filename, source);
        return source;
    }
}
