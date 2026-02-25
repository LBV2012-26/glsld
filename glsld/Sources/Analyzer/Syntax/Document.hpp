#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    struct LocationHash {
        std::size_t operator()(SourceLocation location) const {
            std::size_t hash1 = std::hash<std::size_t>{}(location.line);
            std::size_t hash2 = std::hash<std::size_t>{}(location.column);

            std::size_t seed = 0;
            HashCombine(seed, hash1);
            HashCombine(seed, hash2);
            return seed;
        }
    };

    using BindingMap        = std::unordered_map<SourceLocation, SymbolReference, LocationHash>;
    using MacroTraceMap     = std::unordered_map<SourceLocation, Token, LocationHash>;
    using MacroArgsTraceMap = MacroTraceMap;

    struct InactiveRegion {
        std::size_t begin_line;
        std::size_t end_line;
    };

    struct Document {
        DocumentSymbols                      symbols;
        std::vector<Token>                   raw_tokens;
        std::vector<Token>                   expanded_tokens;
        std::vector<InactiveRegion>          inactive_regions;
        std::unique_ptr<TranslationUnitNode> ast;
        BindingMap                           bindings;
        MacroTraceMap                        macro_traces;
        MacroArgsTraceMap                    macro_args_traces;
        int                                  version{};
    };

    struct InlayHint {
        SourceLocation location;
        std::string    label;
    };
}
