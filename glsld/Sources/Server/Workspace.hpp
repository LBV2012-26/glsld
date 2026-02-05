#pragma once

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Syntax/SymbolTable.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    struct Document {
        DocumentSymbols symbols;
        std::vector<Token> tokens;
        std::unique_ptr<TranslationUnitNode> ast;
    };

    class Workspace {
    public:
        void UpdateDocument(std::string_view uri, std::string_view context);
        void RemoveDocument(std::string_view uri);

        std::vector<const SymbolInfo*> GetDefinitionSymbols(std::string_view uri, SourceLocation location) const;

        const DocumentSymbols* GetDocumentSymbols(std::string_view uri) const;
        std::span<const Token> GetDocumentTokens(std::string_view uri) const;

    private:
        utils::StringHeteroHashTable<std::string, std::unique_ptr<Document>> documents_;
    };
}
