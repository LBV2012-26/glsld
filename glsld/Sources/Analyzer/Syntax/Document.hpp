#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Token.hpp"

namespace glsld {
    using BindingMap        = std::unordered_map<SourceLocation, SymbolReference, LocationHash>;
    using MacroTraceMap     = std::unordered_map<SourceLocation, Token, LocationHash>;
    using MacroArgsTraceMap = MacroTraceMap;

    struct InactiveRegion {
        SourceReference source_ref{ nullptr };
        std::size_t     begin_line{};
        std::size_t     end_line{};
    };

    class FileTable {
    public:
        SourceReference Intern(std::string_view filename, std::string_view uri);

    private:
        StringHeteroHashTable<SourceReference> sources_;
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

        static inline FileTable              file_table;
    };

    struct InlayHint {
        SourceLocation location;
        std::string    label;
    };
}
