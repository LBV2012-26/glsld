#pragma once

#include <string>
#include <string_view>

#include "Analyzer/Syntax/Token.hpp"

namespace glsld::utils {
    std::string GetFilePath(std::string_view filename);
    void PrintIndent(int level);
    bool IsPositionInToken(const Token& token, SourceLocation position);
}
