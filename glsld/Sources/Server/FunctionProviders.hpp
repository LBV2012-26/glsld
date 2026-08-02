#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Base/FileSystem/Source.hpp"
#include "Base/Unicode.hpp"
#include "Server/Index/GlobalIndex.hpp"
#include "Server/Context.hpp"

namespace glsld {
    nlohmann::json ConvertScopeToDocumentSymbols(
        Context& context,
        std::string_view uri,
        const Scope* const scope,
        const PositionMapper& mapper);

    std::vector<std::uint32_t> GetSemanticData(
        Context& context,
        const SourceFile* source_file,
        Snapshot snapshot,
        const PositionMapper& mapper);

    std::optional<std::string> GotoInclude(
        Context& context,
        Snapshot snapshot,
        const SourceLocation& location,
        IncludeDirectoryHandle include_dirs);

    SymbolList GetDefinitionSymbols(Context& context, Snapshot snapshot, const SourceLocation& location, bool toggle_function);

    struct ReferenceResult {
        std::vector<SourceLocation> locations;
        const SymbolInfo*           symbol{ nullptr };
    };

    ReferenceResult GetReferences(
        Context& context,
        Snapshot snapshot,
        const SourceLocation& location,
        const GlobalIndex& global_index);

    std::vector<InlayHint> GetInlayHints(Context& context, Snapshot snapshot);

    struct SignatureHelpResult {
        SymbolList candidates;
        int active_signature_index{};
        int active_param_index{};
    };

    std::optional<SignatureHelpResult> GetSignatureHelp(Context& context, Snapshot snapshot, const SourceLocation& location);

    nlohmann::json GetIncludeCompletionItems(
        Context& context, 
        Snapshot snapshot,
        const SourceLocation& location,
        IncludeDirectoryHandle include_dirs,
        PositionMapper& mapper);

    nlohmann::json GetCompletionItems(Context& context, Snapshot snapshot, const SourceLocation& location);

    nlohmann::json GetFieldCompletionItems(
        Context& context,
        Snapshot snapshot,
        const SourceLocation& location,
        const TypeMemberIndex& type_member_index);

    nlohmann::json GetExtensionCompletionItems(Context& context, Snapshot snapshot, const SourceLocation& location);

    struct FunctionFormatResult {
        std::string return_typename;
        std::string base_name;
        std::string full_spec;
        std::vector<std::string> params;
    };

    FunctionFormatResult FormatFunctionSymbol(const SymbolInfo* symbol, Snapshot snapshot);

    std::string BuildHoverMarkdown(
        const SymbolInfo* symbol,
        Snapshot snapshot,
        const SourceLocation& location,
        std::string_view current_uri);
}
