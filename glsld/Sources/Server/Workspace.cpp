#include "stdafx.h"
#include "Workspace.hpp"

#include <mutex>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Passes/OverloadResolver.hpp"
#include "Analyzer/Passes/SymbolLinker.hpp"
#include "Analyzer/Passes/TypeResolver.hpp"
#include "Analyzer/Syntax/Parser.hpp"

namespace glsld {
    void Workspace::UpdateDocument(std::string_view uri, std::string_view context, int version) {
        auto document = std::make_shared<Document>();
        document->version = version;

        Parser parser(context, document->symbols);
        document->ast = parser.Parse();
        document->tokens = parser.tokens();

        SymbolLinker linker(document->symbols, document->bindings);
        linker.Traverse(document->ast.get());

        TypeResolver collector(document->symbols, document->bindings);
        collector.Traverse(document->ast.get());

        OverloadResolver resolver(document->symbols, document->bindings);
        resolver.Traverse(document->ast.get());

        {
            std::unique_lock lock(mutex_);
            documents_[std::string(uri)] = std::move(document);
        }
    }

    void Workspace::RemoveDocument(std::string_view uri) {
        std::unique_lock lock(mutex_);
        documents_.erase(uri);
    }

    std::shared_ptr<Document> Workspace::GetDocumentSnapshot(std::string_view uri) const {
        std::shared_lock lock(mutex_);

        auto it = documents_.find(uri);
        if (it != documents_.end()) {
            return it->second;
        }

        return nullptr;
    }
}
