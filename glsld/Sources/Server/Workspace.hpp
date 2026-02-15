#pragma once

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    class Workspace {
    public:
        void UpdateDocument(std::string_view uri, std::string_view context);
        void RemoveDocument(std::string_view uri);

        SymbolList GetDefinitionSymbols(std::string_view uri, SourceLocation location) const;

        const Document* GetDocument(std::string_view uri) const;
        const DocumentSymbols* GetDocumentSymbols(std::string_view uri) const;
        std::span<const Token> GetDocumentTokens(std::string_view uri) const;

    private:
        const SymbolInfo* ResolveFunctionJump(const SymbolInfo* symbol, std::string_view uri) const;

        utils::StringHeteroHashTable<std::string, std::unique_ptr<Document>> documents_;
    };
}
