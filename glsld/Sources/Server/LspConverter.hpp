#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <nlohmann/json.hpp>

#include "Analyzer/Syntax/SymbolTable.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Token.hpp"

namespace glsld {
    nlohmann::json ConvertScopeToDocumentSymbols(const Scope* const scope);
    std::vector<std::uint32_t> SemanticData(const Document& document);
}
