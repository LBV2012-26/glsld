#include "stdafx.h"
#include "Workspace.hpp"

#include <mutex>
#include <utility>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Passes/MacroBinder.hpp"
#include "Analyzer/Passes/SymbolLinker.hpp"
#include "Analyzer/Passes/TypeResolver.hpp"
#include "Analyzer/Syntax/Parser.hpp"
#include "Base/Config.hpp"

namespace glsld {
    Workspace::Workspace(ThreadPool& thread_pool)
        : thread_pool_{ thread_pool }
        , include_loader_{ source_table_, thread_pool }
    {
        // auto include_dirs = Config::Lookup("include_dirs", std::vector<std::filesystem::path>(), "Include directories for GLSL source files");
    }

    void Workspace::UpdateDocument(
        std::string_view uri,
        std::string_view context,
        int version_replica,
        std::shared_ptr<const std::atomic<int>> version,
        bool open_document)
    {
        auto document = std::make_shared<Document>();
        document->version = version_replica;

        auto Cancelled = [version_replica, &version]() -> bool {
            return version_replica != version->load();
        };

        const auto* source_file = source_table_.InternByUri(uri);

        try {
            Parser parser(source_table_, source_file, context, include_loader_, include_dirs_, version_replica, version, *document);
            parser.Parse();

            if (document->ast == nullptr) { // 如果版本更改，会返回 nullptr
                return;
            }
        } catch (const std::runtime_error&) { // 版本更改，Lexer 中止
            return;
        }

        if (Cancelled()) return;
        SymbolLinker linker(*document, version_replica, version);
        linker.Traverse(document->ast.get());

        if (Cancelled()) return;
        TypeResolver resolver(*document, version_replica, version);
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
