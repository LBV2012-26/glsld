#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <ankerl/unordered_dense.h>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Base/FileSystem/Source.hpp"

namespace glsld {
    using BindingMap = ankerl::unordered_dense::map<SourceLocation, SymbolReference, LocationHash>;

    struct MacroDefination {
        bool               is_function{};
        Token              original_token;
        std::vector<Token> replacement_list;
        std::vector<Token> params;
    };

    using MacroTraceMap     = ankerl::unordered_dense::map<SourceLocation, Token, LocationHash>;
    using MacroArgsTraceMap = MacroTraceMap;
    using MacroExpansionMap = ankerl::unordered_dense::map<SourceLocation, std::vector<Token>, LocationHash>;
    using MacroTable        = StringHeteroHashMap<MacroDefination>;

    struct InactiveRegion {
        std::size_t begin_line{};
        std::size_t end_line{};
    };

    using InactiveRegionMap = ankerl::unordered_dense::map<const SourceFile*, std::vector<InactiveRegion>, SourceFileHash>;

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
        MacroExpansionMap        macro_expansions;
        MacroTable               macros;
        int                      version{};
    };

    struct InlayHint {
        const SourceLocation* location{ nullptr };
        std::string           label;
    };
}
