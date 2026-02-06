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
    using LocationPair = std::pair<SourceLocation, SourceLocation>;

    struct BindingHash {
        std::size_t operator()(const LocationPair& pair) const {
            std::size_t hash1 = std::hash<std::size_t>{}(pair.first.line);
            std::size_t hash2 = std::hash<std::size_t>{}(pair.first.column);
            std::size_t hash3 = std::hash<std::size_t>{}(pair.second.line);
            std::size_t hash4 = std::hash<std::size_t>{}(pair.second.column);

            std::size_t seed = 0;
            utils::HashCombine(seed, hash1);
            utils::HashCombine(seed, hash2);
            utils::HashCombine(seed, hash3);
            utils::HashCombine(seed, hash4);
            return seed;
        }
    };

    using BindingMap = std::unordered_map<LocationPair, SymbolRef, BindingHash>;

    struct Document {
        DocumentSymbols                      symbols;
        std::vector<Token>                   tokens;
        std::unique_ptr<TranslationUnitNode> ast;
        BindingMap                           bindings;
    };
}
