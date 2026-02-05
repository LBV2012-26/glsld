#pragma once

#include <string>
#include <string_view>

namespace glsld::utils {
    std::string GetFilePath(std::string_view filename);
    void PrintIndent(int level);
} // namespace valky::utils
