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

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Passes/TypeResolver.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Token.hpp"

namespace glsld {
    nlohmann::json ConvertScopeToDocumentSymbols(std::string_view uri, const Scope* const scope);
    std::vector<std::uint32_t> GetSemanticData(std::string_view uri, std::shared_ptr<const Document> snapshot);

    std::optional<std::string> GotoInclude(
        std::shared_ptr<const Document> snapshot,
        const SourceLocation& location,
        std::span<const std::filesystem::path> include_dirs);

    SymbolList GetDefinitionSymbols(std::shared_ptr<const Document> snapshot, const SourceLocation& location, bool toggle_function);
    std::vector<InlayHint> GetInlayHints(std::shared_ptr<const Document> snapshot);

    struct SignatureHelpResult {
        SymbolList candidates;
        int active_signature_index{};
        int active_param_index{};
    };

    std::optional<SignatureHelpResult> GetSignatureHelp(std::shared_ptr<const Document> snapshot, const SourceLocation& location);

    nlohmann::json GetIncludeCompletionItems(
        std::shared_ptr<const Document> snapshot,
        const SourceLocation& location,
        std::span<const std::filesystem::path> include_dirs);

    nlohmann::json GetCompletionItems(std::shared_ptr<const Document> snapshot, const SourceLocation& location);
    nlohmann::json GetFieldCompletionItems(std::shared_ptr<const Document> snapshot, const SourceLocation& location);
    std::string FormatSymbol(const SymbolInfo* symbol);
}
