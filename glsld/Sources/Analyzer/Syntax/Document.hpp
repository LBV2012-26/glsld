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
    struct LocationHash {
        std::size_t operator()(SourceLocation location) const;
    };

    using BindingMap        = std::unordered_map<SourceLocation, SymbolReference, LocationHash>;
    using MacroTraceMap     = std::unordered_map<SourceLocation, Token, LocationHash>;
    using MacroArgsTraceMap = MacroTraceMap;

    struct InactiveRegion {
        std::size_t begin_line;
        std::size_t end_line;
    };

    struct FileEntry {
        std::uint32_t index{};
        std::string   uri;
        std::string   normalized_path;
    };

    struct FileTable {
        std::vector<FileEntry>               files;
        StringHeteroHashTable<std::uint32_t> path_index_map;
        StringHeteroHashTable<std::uint32_t> uri_index_map;

        std::uint32_t Intern(std::string_view uri, std::string_view normalized_path);
        const FileEntry* FindByIndex(std::uint32_t index) const;
        std::optional<std::uint32_t> FindByUri(std::string_view uri) const;
    };

    struct Document {
        DocumentSymbols                      symbols;
        FileTable                            files;
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
