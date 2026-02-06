#pragma once

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Syntax/SymbolTable.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    struct BindingHash {
        std::size_t operator()(SourceLocation location) const {
            std::size_t hash1 = std::hash<std::size_t>{}(location.line);
            std::size_t hash2 = std::hash<std::size_t>{}(location.column);

            std::size_t seed = 0;
            utils::HashCombine(seed, hash1);
            utils::HashCombine(seed, hash2);
            return seed;
        }
    };

    using BindingMap = std::unordered_map<SourceLocation, SymbolRef, BindingHash>;

    struct Document {
        DocumentSymbols                      symbols;
        std::vector<Token>                   tokens;
        std::unique_ptr<TranslationUnitNode> ast;
        BindingMap                           bindings;
    };
}
