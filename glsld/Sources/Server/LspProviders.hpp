#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Token.hpp"

namespace glsld {
    nlohmann::json ConvertScopeToDocumentSymbols(const Scope* const scope);
    std::vector<std::uint32_t> GetSemanticData(std::shared_ptr<const Document> snapshot);
    SymbolList GetDefinitionSymbols(std::shared_ptr<const Document> snapshot, SourceLocation location);
    std::vector<InlayHint> GetInlayHints(std::shared_ptr<const Document> snapshot);
    nlohmann::json GetCompletionItems(std::shared_ptr<const Document> snapshot, SourceLocation location);
    nlohmann::json GetFieldCompletionItems(std::shared_ptr<const Document> snapshot, SourceLocation location);
    std::string FormatSymbol(const SymbolInfo* symbol);
}
