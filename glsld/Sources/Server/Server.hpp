#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Base/Hash.hpp"
#include "Base/ThreadPool.hpp"
#include "Server/Context.hpp"
#include "Server/Router.hpp"
#include "Server/Workspace.hpp"

namespace glsld {
    class LspServer {
    public:
        LspServer();

        void Run();

    private:
        struct PendingUpdate {
            std::string text;
            std::chrono::steady_clock::time_point deadline;
        };

        void RegisterHandlers();

        std::optional<nlohmann::json> ReadMessage();
        void SendMessage(const nlohmann::json& message);

        nlohmann::json HandleInitialize(Context& context);
        nlohmann::json HandleShutdown(Context& context);
        nlohmann::json HandleDocumentSymbol(Context& context);
        nlohmann::json HandleSemanticTokens(Context& context);
        nlohmann::json HandleDefinition(Context& context);
        nlohmann::json HandleHover(Context& context);
        nlohmann::json HandleInlayHints(Context& context);
        nlohmann::json HandleCompletion(Context& context);

        void HandleDidOpen(Context& context);
        void HandleDidChange(Context& context);
        void HandleDidClose(Context& context);
        void HandleInitialized(Context& context);
        void HandleExit(Context& context);

        void UpdateWorker(const std::string& uri, int version_replica);
        void Update(const std::string& uri, std::string_view text, int version_replica);
        std::shared_ptr<const Document> ValidateAndGetDocument(const std::string& uri) const;

        std::atomic<bool>               running_{ true };
        mutable std::condition_variable ready_condition_;
        mutable std::mutex              update_mutex_;
        Router                          router_;
        Workspace                       workspace_;
        ThreadPool                      thread_pool_;

        StringHeteroHashTable<std::string, PendingUpdate> pending_updates_;
        StringHeteroHashTable<std::string, std::shared_ptr<std::atomic<int>>> document_versions_;
    };
}
