#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace glsld::utils {
    std::string GetFilePath(std::string_view filename);
    std::filesystem::path UriToPath(std::string_view uri);
    std::string PathToUri(const std::filesystem::path& path);
    std::filesystem::path NormalizePath(const std::filesystem::path& path);
    std::string_view UnmangleFunctionName(std::string_view mangled_name);
    void PrintIndent(int level);
}
