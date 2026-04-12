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
#include "Base/Logger.hpp"

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
        document->source_code = std::string(context);
        document->version     = version_replica;

        const auto* source_file = source_table_.InternByUri(uri);

        try {
            Parser parser(source_table_, source_file, context, include_loader_, include_dirs_, version_replica, version, *document);

            if (document->ast == nullptr) { // 如果版本更改，会返回 nullptr
                GLSLD_LOG_INFO(
                    GLSLD_LOG_ROOT(),
                    "Version changed during document update (replica: {}, current: {}), cancelling parse.",
                    version_replica,
                    version->load()
                );

                return;
            }
        } catch (const std::runtime_error&) { // 版本更改，Lexer 中止
            GLSLD_LOG_INFO(
                GLSLD_LOG_ROOT(),
                "Version changed during document update (replica: {}, current: {}), cancelling lex.",
                version_replica,
                version->load()
            );
        }

        auto Cancelled = [version_replica, &version]() -> bool {
            if (version_replica != version->load()) {
                GLSLD_LOG_INFO(
                    GLSLD_LOG_ROOT(),
                    "Version changed during document update (replica: {}, current: {}), cancelling update.",
                    version_replica,
                    version->load()
                );

                return true;
            }

            return false;
        };

        if (Cancelled()) return;
        SymbolLinker linker(*document, version_replica, version);

        if (Cancelled()) return;
        TypeResolver resolver(*document, version_replica, version);

        if (Cancelled()) return;
        MacroBinder binder(*document, version_replica, version);

        UpdateDependencies(uri, document);

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

    std::vector<std::string> Workspace::GetAffectedDocuments(std::string_view changed_uri) const {
        std::vector<std::string> results;
        auto it = reverse_dependencies_.find(changed_uri);
        if (it != reverse_dependencies_.end()) {
            results.assign(it->second.begin(), it->second.end());
        }

        return results;
    }

    void Workspace::UpdateDependencies(std::string_view uri, std::shared_ptr<const Document> document) {
        auto it = forward_dependencies_.find(uri);
        if (it != forward_dependencies_.end()) {
            for (const auto& dependency : it->second) {
                reverse_dependencies_[dependency].erase(uri);
            }
        }

        forward_dependencies_[std::string(uri)] = document->dependencies;
        for (const auto& dependency : document->dependencies) {
            reverse_dependencies_[dependency].emplace(uri);
        }
    }
}
