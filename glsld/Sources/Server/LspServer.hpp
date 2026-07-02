#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <string>

#include <ankerl/unordered_dense.h>
#include <nlohmann/json.hpp>

#include "Analyzer/Syntax/Document.hpp"
#include "Base/Hash.hpp"
#include "Base/ThreadPool.hpp"
#include "Server/Context.hpp"
#include "Server/DiagnosticEngine.hpp"
#include "Server/Router.hpp"
#include "Server/Workspace.hpp"

namespace glsld {
    struct LspTask {
        std::string                   method;
        nlohmann::json                params;
        std::optional<nlohmann::json> id;
        CancellationToken             cancellation_token{ nullptr };
        bool                          is_request{ false };
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
        using VersionPointer         = std::shared_ptr<std::atomic<int>>;
        using CancellationTokenTable = ankerl::unordered_dense::map<nlohmann::json, CancellationToken>; // [Request ID, Token]

        void RegisterHandlers();
        std::optional<nlohmann::json> ReadMessage();

        void WorkerLoop();
        void SubmitLoop();
        void UpdateLoop();
        void EnqueueTask(LspTask task);
        void EnqueueSubmit(LspSubmitItem item);
        void EnqueueUpdate(const std::string& uri, int version_replica, bool open_document);
        void CancelRequest(const nlohmann::json& message);
        void Configure(Context& context);

        nlohmann::json HandleInitialize(Context& context);
        nlohmann::json HandleShutdown(Context& context);
        nlohmann::json HandleDocumentSymbol(Context& context);
        nlohmann::json HandleSemanticTokens(Context& context);
        nlohmann::json HandleDefinition(Context& context);
        nlohmann::json HandleReferences(Context& context);
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
        void Update(const std::string& uri, std::string_view text, int version_replica, VersionPointer version_pointer, bool open_document);

        void SubmitDiagnositcTask(
            const std::string& uri,
            std::string_view source,
            std::string_view filename,
            int version_replica,
            VersionPointer version_pointer);

        std::shared_ptr<const Document> ValidateAndGetDocument(const Context& context, const std::string& uri) const;

        struct PendingUpdate {
            std::string                           text;
            std::chrono::steady_clock::time_point deadline;
        };

        std::atomic<bool>                   running_{ true };
        Router                              router_;
        ThreadPool                          thread_pool_;
        ThreadPool                          update_pool_;
        Workspace                           workspace_;
        DiagnosticEngine                    diagnostic_engine_;
        mutable std::condition_variable     ready_condition_;
        mutable std::mutex                  ready_mutex_;

        StringHeteroHashMap<PendingUpdate>  pending_updates_;
        StringHeteroHashMap<VersionPointer> document_versions_;   // [Uri, Version]
        mutable std::shared_mutex           pending_update_mutex_;

        StringHeteroHashMap<VersionPointer> document_revisions_;  // [Uri, Revision], for background include affected document update
        std::shared_mutex                   revision_mutex_;

        std::mutex                          task_mutex_;
        std::condition_variable             task_condition_;
        std::queue<LspTask>                 task_queue_;

        std::mutex                          submit_mutex_;
        std::condition_variable             submit_condition_;
        std::queue<LspSubmitItem>           submit_queue_;

        std::mutex                          update_mutex_;
        std::condition_variable             update_condition_;
        std::queue<std::function<void()>>   update_queue_;

        CancellationTokenTable              cancellation_tokens_; // [Request ID, Token]
        std::mutex                          cancellation_mutex_;
    };
}
