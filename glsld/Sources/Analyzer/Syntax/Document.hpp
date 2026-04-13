#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Base/FileSystem/Source.hpp"

namespace glsld {
    using BindingMap        = std::unordered_map<SourceLocation, SymbolReference, LocationHash>;
    using MacroTraceMap     = std::unordered_map<SourceLocation, Token, LocationHash>;
    using MacroArgsTraceMap = MacroTraceMap;

    struct InactiveRegion {
        std::size_t begin_line{};
        std::size_t end_line{};
    };

    using InactiveRegionMap = std::unordered_map<const SourceFile*, std::vector<InactiveRegion>, SourceFileHash>;

    struct Document {
        using AstRoot = std::unique_ptr<TranslationUnitNode>;

        std::vector<std::string> dependencies; // [URI]
        DocumentSymbols          symbols;
        std::string              source;
        std::vector<Token>       raw_tokens;
        std::vector<Token>       expanded_tokens;
        InactiveRegionMap        inactive_regions;
        AstRoot                  ast;
        BindingMap               bindings;
        MacroTraceMap            macro_traces;
        MacroArgsTraceMap        macro_args_traces;
        int                      version{};
    };

    struct InlayHint {
        const SourceLocation* location{ nullptr };
        std::string           label;
    };
}
