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
#include "Server/Context.hpp"

namespace glsld {
    nlohmann::json ConvertScopeToDocumentSymbols(Context& context, std::string_view uri, const Scope* const scope);
    std::vector<std::uint32_t> GetSemanticData(Context& context, const SourceFile* source_file, std::shared_ptr<const Document> snapshot);

    std::optional<std::string> GotoInclude(
        Context& context,
        std::shared_ptr<const Document> snapshot,
        const SourceLocation& location,
        std::span<const std::filesystem::path> include_dirs);

    SymbolList GetDefinitionSymbols(Context& context, std::shared_ptr<const Document> snapshot, const SourceLocation& location, bool toggle_function);
    std::vector<InlayHint> GetInlayHints(Context& context, std::shared_ptr<const Document> snapshot);

    struct SignatureHelpResult {
        SymbolList candidates;
        int active_signature_index{};
        int active_param_index{};
    };

    std::optional<SignatureHelpResult> GetSignatureHelp(Context& context, std::shared_ptr<const Document> snapshot, const SourceLocation& location);

    nlohmann::json GetIncludeCompletionItems(
        Context& context, 
        std::shared_ptr<const Document> snapshot,
        const SourceLocation& location,
        std::span<const std::filesystem::path> include_dirs);

    nlohmann::json GetCompletionItems(Context& context, std::shared_ptr<const Document> snapshot, const SourceLocation& location);
    nlohmann::json GetFieldCompletionItems(Context& context, std::shared_ptr<const Document> snapshot, const SourceLocation& location);
    nlohmann::json GetExtensionCompletionItems(Context& context, std::shared_ptr<const Document> snapshot, const SourceLocation& location);

    struct FunctionFormatResult {
        std::string return_typename;
        std::string base_name;
        std::string full_spec;
        std::vector<std::string> params;
    };

    FunctionFormatResult FormatFunctionSymbol(const SymbolInfo* symbol, std::shared_ptr<const Document> snapshot);

    std::string BuildHoverMarkdown(
        const SymbolInfo* symbol,
        std::shared_ptr<const Document> snapshot,
        const SourceLocation& location,
        std::string_view current_uri);
}
