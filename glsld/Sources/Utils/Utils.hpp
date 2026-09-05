#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <concepts>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Syntax/Symbol.hpp"

namespace glsld::Utils {
    // FileSystem
    std::string GetFilePath(std::string_view filename);
    std::filesystem::path UriToPath(std::string_view uri);
    std::string PathToUri(const std::filesystem::path& path);
    std::filesystem::path NormalizePath(const std::filesystem::path& path);

    std::expected<std::vector<std::byte>, std::string> LoadBinary(const std::filesystem::path& filename);
    std::expected<std::string, std::string> LoadSource(const std::filesystem::path& filename);

    // String manipulation
    std::string_view UnmangleFunctionName(std::string_view mangled_name);
    void PrintIndent(int level);

    // Serialization
    std::string SerializeQualifierArguments(const QualifierArgumentNode* argument);

    template <typename Ty>
    concept IsQualifierArgument = std::same_as<Ty, LayoutQualifierNode> || std::same_as<Ty, SpirvIntrinsicNode>;

    std::string BuildQualifierParameterList(const IsQualifierArgument auto* node);

    // Parsing helper
    std::string UnquoteStringLiteral(std::string_view text);

    enum class NumberLiteralKind {
        kInvalid,
        kSignedInteger,
        kUnsignedInteger,
        kFloatingPoint
    };

    struct NumberLiteralInfo {
        NumberLiteralKind kind{ NumberLiteralKind::kInvalid };
        std::string_view  core;
        int               base{ 10 };
        int               bits{};
        FloatEncoding     float_encoding{ FloatEncoding::kStandard };

        explicit operator bool() const;
    };

    NumberLiteralInfo AnalyzeNumberLiteral(std::string_view text);

    std::int64_t ParseNumberLiteralToInteger(std::string_view text);

    struct VectorSwizzle {
        std::array<std::size_t, 4> indices{};
        std::size_t                count{};
    };

    std::optional<VectorSwizzle> ParseVectorSwizzle(std::string_view text, std::size_t length);

    template <typename Ty>
    std::optional<std::vector<Ty>> CollectArgumentArray(
        const QualifierArgumentNode* rhs,
        QualifierArgumentKind required_kind,
        auto&& pred);

    std::optional<SymbolList> CollectStructFieldsOrdered(const SymbolInfo* struct_symbol);

    bool HasInterfaceLayoutQualifier(const SymbolInfo* symbol, std::string_view qualifier);

    // External process execution
    std::string ExecuteCommand(
        std::string_view command,
        std::string_view working_dir = {},
        std::string_view input = {},
        int timeout_ms = 10'000,
        int* exit_code = nullptr);
}

#include "Utils.inl"
