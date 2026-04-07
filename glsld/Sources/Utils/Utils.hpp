#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Token.hpp"

namespace glsld::utils {
    std::string GetFilePath(std::string_view filename);
    std::filesystem::path UriToPath(std::string_view uri);
    std::string PathToUri(const std::filesystem::path& path);
    std::filesystem::path NormalizePath(const std::filesystem::path& path);
    std::string_view UnmangleFunctionName(std::string_view mangled_name);
    void PrintIndent(int level);
    bool IsPositionInToken(const Token& token, SourceLocation position);
    bool IsPositionInFunctionName(const SymbolInfo* symbol, SourceLocation position);
}
