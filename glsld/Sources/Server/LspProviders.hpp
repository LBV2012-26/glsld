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
    std::vector<std::uint32_t> GetSemanticData(std::shared_ptr<const Document> document_snapshot);
    SymbolList GetDefinitionSymbols(std::shared_ptr<const Document> document_snapshot, SourceLocation location);
    std::vector<InlayHint> GetInlayHints(std::shared_ptr<const Document> document_snapshot);
    std::string FormatSymbol(const SymbolInfo* symbol);
}
