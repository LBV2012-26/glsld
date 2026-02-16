#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <nlohmann/json.hpp>

#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Server/Context.hpp"
#include "Server/Router.hpp"
#include "Server/Workspace.hpp"

namespace glsld {
    class LspServer {
    public:
        LspServer();

        void Run();

    private:
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

        void HandleDidOpen(Context& context);
        void HandleDidChange(Context& context);
        void HandleDidClose(Context& context);
        void HandleInitialized(Context& context); // 客户端确认初始化完成
        void HandleExit(Context& context);

        void UpdateWorker();
        void Update(std::string_view uri, std::string_view text);

        struct PendingUpdate {
            std::string text;
            std::chrono::steady_clock::time_point deadline;
        };

        Router router_;
        Workspace workspace_;
        std::atomic<bool> running_{ true };
        std::mutex update_mutex_;
        std::condition_variable update_condition_;
        utils::StringHeteroHashTable<std::string, PendingUpdate> pending_updates_;
        std::jthread worker_thread_;
    };
}
