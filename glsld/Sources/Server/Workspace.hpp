#pragma once

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "Analyzer/Syntax/Document.hpp"
#include "Base/Hash.hpp"

namespace glsld {
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
