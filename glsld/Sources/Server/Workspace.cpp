#include "stdafx.h"
#include "Workspace.hpp"

#include <mutex>
#include <utility>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Passes/MacroBinder.hpp"
#include "Analyzer/Passes/SymbolLinker.hpp"
#include "Analyzer/Passes/TypeResolver.hpp"
#include "Analyzer/Syntax/Parser.hpp"

namespace glsld {
    void Workspace::UpdateDocument(std::string_view uri, std::string_view context, int version_replica,
                                   std::shared_ptr<const std::atomic<int>> version, bool open_document)
    {
        auto document = std::make_shared<Document>();
        document->version = version_replica;

        auto Cancelled = [version_replica, &version]() -> bool {
            return version_replica != version->load();
        };

        try {
            Parser parser(context, *document, version_replica, version);
            parser.Parse();

            if (document->ast == nullptr) { // 如果版本更改，会返回 nullptr
                return;
            }

        } catch (const std::runtime_error&) { // 版本更改，Lexer 中止
            return;
        }

        if (Cancelled()) return;
        SymbolLinker linker(document->symbols, document->bindings, version_replica, version);
        linker.Traverse(document->ast.get());

        if (Cancelled()) return;
        TypeResolver resolver(document->symbols, document->bindings, version_replica, version);
        resolver.Traverse(document->ast.get());

        if (Cancelled()) return;
        MacroBinder binder(*document);
        binder.BindMacro();

        {
            std::unique_lock lock(mutex_);
            if (!open_document) {
                if (!documents_.contains(uri)) {
                    return;
                }

                *documents_[std::string(uri)] = std::move(*document);
            } else {
                documents_[std::string(uri)] = std::move(document);
            }
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
