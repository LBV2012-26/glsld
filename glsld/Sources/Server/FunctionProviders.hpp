#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Passes/TypeResolver.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Token.hpp"

namespace glsld {
    nlohmann::json ConvertScopeToDocumentSymbols(const Scope* const scope);
    std::vector<std::uint32_t> GetSemanticData(std::shared_ptr<const Document> snapshot);
    SymbolList GetDefinitionSymbols(std::shared_ptr<const Document> snapshot, SourceLocation location, bool toggle_function);
    std::vector<InlayHint> GetInlayHints(std::shared_ptr<const Document> snapshot);

    struct SignatureHelpResult {
        SymbolList candidates;
        int active_signature_index{};
        int active_param_index{};
    };

    std::optional<SignatureHelpResult> GetSignatureHelp(std::shared_ptr<const Document> snapshot, SourceLocation location);

    nlohmann::json GetCompletionItems(std::shared_ptr<const Document> snapshot, SourceLocation location);
    nlohmann::json GetFieldCompletionItems(std::shared_ptr<const Document> snapshot, SourceLocation location);
    std::string FormatSymbol(const SymbolInfo* symbol);
}
