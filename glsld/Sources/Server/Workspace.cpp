#include "stdafx.h"
#include "Workspace.hpp"

#include <mutex>
#include <utility>

#include "Analyzer/Passes/MacroBinder.hpp"
#include "Analyzer/Passes/SymbolLinker.hpp"
#include "Analyzer/Passes/TypeResolver.hpp"
#include "Analyzer/Syntax/MetadataManager.hpp"
#include "Analyzer/Syntax/Parser.hpp"
#include "Base/Config.hpp"
#include "Base/Logger.hpp"

namespace glsld {
    Workspace::Workspace(ThreadPool& thread_pool)
        : thread_pool_{ thread_pool }
        , include_loader_{ source_table_, thread_pool }
    {
        // TODO: initialize include_dirs from config file
    }

    void Workspace::UpdateDocument(
        std::string_view uri,
        std::string_view source,
        int version_replica,
        std::shared_ptr<const std::atomic<int>> version_pointer,
        bool open_document)
    {
        auto document = std::make_shared<Document>();
        document->source  = std::string(source);
        document->version = version_replica;

        const auto* source_file = source_table_.InternByUri(uri);
        ProcessSource(source_file, source, version_replica, version_pointer, *document);

        UpdateDependencies(uri, document);

        {
            std::unique_lock lock(mutex_);
            if (!open_document) {
                if (!documents_.contains(uri)) {
                    return;
                }

                *documents_.at(std::string(uri)) = std::move(*document);
            } else {
                documents_.try_emplace(std::string(uri), std::move(document));
            }
        }
    }

    void Workspace::RemoveDocument(std::string_view uri) {
        std::unique_lock lock(mutex_);
        documents_.erase(uri);
        RemoveDependencies(uri);
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

    void Workspace::ProcessSource(
        const SourceFile* source_file,
        std::string_view source,
        int version_replica,
        std::shared_ptr<const std::atomic<int>> version_pointer,
        Document& document)
    {
        Lexer lexer(source_file, source, include_loader_, include_dirs_);
        std::vector<Token> raw_tokens;
        raw_tokens.reserve(source.length() / 5);

        do {
            if (version_pointer != nullptr && version_replica != version_pointer->load(std::memory_order::relaxed)) {
                return;
            }

            raw_tokens.push_back(lexer.AcquireNextToken());
        } while (raw_tokens.back().type != TokenType::kEndOfFile);

        MetadataManager::GetInstance().AttachBuiltinMetadata(document, raw_tokens, include_dirs_);

        Parser parser(source_table_, source_file, std::move(raw_tokens), include_loader_, include_dirs_, version_replica, version_pointer, document);
        if (document.ast == nullptr) { // 如果版本更改，会返回 nullptr
            GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Version changed during document update (replica: {}, current: {}), cancelling parse.",
                            version_replica, version_pointer->load());
            return;
        }

        auto Cancelled = [version_replica, &version_pointer]() -> bool {
            if (version_replica != version_pointer->load()) {
                GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Version changed during document update (replica: {}, current: {}), cancelling update.",
                                version_replica, version_pointer->load());
                return true;
            }

            return false;
        };

        if (Cancelled()) return;
        SymbolLinker linker(document, version_replica, version_pointer);

        if (Cancelled()) return;
        TypeResolver resolver(document, version_replica, version_pointer);

        if (Cancelled()) return;
        MacroBinder binder(document, version_replica, version_pointer);
    }

    void Workspace::UnregisterDependencies(std::string_view uri) {
        auto forward_it = forward_dependencies_.find(uri);
        if (forward_it == forward_dependencies_.end()) {
            return;
        }

        for (const auto& dependency : forward_it->second) {
            auto reverse_it = reverse_dependencies_.find(dependency);
            if (reverse_it == reverse_dependencies_.end()) {
                continue;
            }

            reverse_it->second.erase(uri);
            if (reverse_it->second.empty()) {
                reverse_dependencies_.erase(reverse_it);
            }
        }
    }

    void Workspace::UpdateDependencies(std::string_view uri, std::shared_ptr<const Document> document) {
        UnregisterDependencies(uri);

        forward_dependencies_[std::string(uri)] = document->dependencies;
        for (const auto& dependency : document->dependencies) {
            reverse_dependencies_[dependency].emplace(uri);
        }
    }

    void Workspace::RemoveDependencies(std::string_view uri) {
        UnregisterDependencies(uri);
        forward_dependencies_.erase(uri);
    }
}
