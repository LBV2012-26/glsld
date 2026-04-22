#pragma once

#include <cstdint>
#include <concepts>
#include <filesystem>
#include <string>
#include <string_view>

namespace glsld {
    struct QualifierArgumentNode;
    struct LayoutQualifierNode;
    struct SpirvIntrinsicNode;
}

namespace glsld::utils {
    // FileSystem
    std::string GetFilePath(std::string_view filename);
    std::filesystem::path UriToPath(std::string_view uri);
    std::string PathToUri(const std::filesystem::path& path);
    std::filesystem::path NormalizePath(const std::filesystem::path& path);

    // String manipulation
    std::string_view UnmangleFunctionName(std::string_view mangled_name);
    void PrintIndent(int level);

    // Serialization
    std::string SerializeQualifierArguments(const QualifierArgumentNode* argument);

    template <typename Ty>
    concept IsQualifierArgument = std::same_as<Ty, LayoutQualifierNode> || std::same_as<Ty, SpirvIntrinsicNode>;
    std::string BuildQualifierParameterList(const IsQualifierArgument auto* node);

    // Parsing helper
    std::int64_t ParseNumberLiteralToInteger(std::string_view text);
}

#include "Utils.inl"
