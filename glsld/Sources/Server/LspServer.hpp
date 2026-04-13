#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <string>
#include <unordered_map>
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
    struct LspTask {
        std::string                        method;
        nlohmann::json                     params;
        std::optional<nlohmann::json>      id;

        std::shared_ptr<std::atomic<bool>> cancelled_token{
            std::make_shared<std::atomic<bool>>(false)
        };

        bool                               is_request{ false };
    };

    struct LspSubmitItem {
        enum class Kind {
            kResponse,
            kNotification,
            kError
        };

        std::optional<nlohmann::json> id;
        nlohmann::json                payload;
        std::string                   error_message;
        std::string                   notify_method;
        int                           error_code{};
        Kind                          kind{ Kind::kResponse };
    };

    class LspServer {
    public:
        LspServer();

        void Run();

    private:
        void RegisterHandlers();
        std::optional<nlohmann::json> ReadMessage();

        void WorkerLoop();
        void SubmitLoop();
        void EnqueueTask(LspTask task);
        void EnqueueSubmit(LspSubmitItem item);
        void CancelRequest(const nlohmann::json& message);

        nlohmann::json HandleInitialize(Context& context);
        nlohmann::json HandleShutdown(Context& context);
        nlohmann::json HandleDocumentSymbol(Context& context);
        nlohmann::json HandleSemanticTokens(Context& context);
        nlohmann::json HandleDefinition(Context& context);
        nlohmann::json HandleHover(Context& context);
        nlohmann::json HandleInlayHints(Context& context);
        nlohmann::json HandleSignatureHelp(Context& context);
        nlohmann::json HandleCompletion(Context& context);

        void HandleDidOpen(Context& context);
        void HandleDidChange(Context& context);
        void HandleDidSave(Context& context);
        void HandleDidClose(Context& context);
        void HandleInitialized(Context& context);
        void HandleExit(Context& context);

        void UpdateWorker(const std::string& uri, int version_replica, bool open_document);
        void Update(const std::string& uri, std::string_view text, int version_replica, bool open_document);
        std::shared_ptr<const Document> ValidateAndGetDocument(const Context& context, const std::string& uri) const;

        std::atomic<bool>                                     running_{ true };
        Router                                                router_;
        ThreadPool                                            thread_pool_;
        Workspace                                             workspace_;
        mutable std::condition_variable                       ready_condition_;
        mutable std::shared_mutex                             update_mutex_;
        mutable std::mutex                                    ready_mutex_;

        struct PendingUpdate {
            std::string                           text;
            std::chrono::steady_clock::time_point deadline;
        };

        StringHeteroHashMap<PendingUpdate>                    pending_updates_;
        using VersionPointer = std::shared_ptr<std::atomic<int>>;
        StringHeteroHashMap<VersionPointer>                   document_versions_;

        std::mutex                                            task_mutex_;
        std::condition_variable                               task_condition_;
        std::queue<LspTask>                                   task_queue_;

        std::mutex                                            submit_mutex_;
        std::condition_variable                               submit_condition_;
        std::queue<LspSubmitItem>                             submit_queue_;

        using CancellationToken = std::shared_ptr<std::atomic<bool>>;
        std::unordered_map<nlohmann::json, CancellationToken> cancellation_tokens_;
        std::mutex                                            cancellation_mutex_;
    };
}
