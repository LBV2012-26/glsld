#pragma once

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <stop_token>
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
            kServerRequest,
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
        void EnqueueUpdate(const std::string& uri);
        void CancelRequest(const nlohmann::json& message);

        nlohmann::json HandleInitialize(Context& context);
        nlohmann::json HandleShutdown(Context& context);
        nlohmann::json HandleDocumentSymbol(Context& context);
        nlohmann::json HandleSemanticTokens(Context& context);
        nlohmann::json HandleDefinition(Context& context);
        nlohmann::json HandleReferences(Context& context);
        nlohmann::json HandleRename(Context& context);
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
        void HandleConfigure(Context& context);
        void HandleRemoveConfiguration(Context& context);
        void HandleChangeVariant(Context& context);
        void HandleRemoveVariant(Context& context);

        void ApplyShaderConfigs(const nlohmann::json& glsld);
        void ApplyVariantConfigs(const nlohmann::json& glsld);
        void ApplyIndexConfigs(const nlohmann::json& glsld);

        void RefreshDocument(std::string_view uri);
        void RebuildDocuments();
        void UpdateWorker(std::string_view uri);
        void PickupPendingUpdate(std::string_view uri);
        void Update(std::string_view uri, std::string_view text, int version_replica, VersionPointer version_pointer, bool open_document);

        void SubmitDiagnositcTask(
            std::string_view uri,
            std::string_view source,
            std::string_view filename,
            int version_replica,
            VersionPointer version_pointer);

        std::shared_ptr<const Document> ValidateAndGetDocument(const Context& context, std::string_view uri);

        struct PendingUpdate {
            std::string                           text;
            std::chrono::steady_clock::time_point deadline;
            int                                   version_replica{};
            bool                                  open_document{ false };
        };

        std::stop_source                    stop_source_;
        std::atomic<int>                    server_request_id_{};
        Router                              router_;
        ThreadPool                          thread_pool_;
        ThreadPool                          update_pool_;
        Workspace                           workspace_;
        StringHeteroHashSet                 document_uris_;
        DiagnosticEngine                    diagnostic_engine_;
        std::condition_variable_any         ready_condition_;
        std::mutex                          ready_mutex_;

        std::vector<std::filesystem::path>  workspace_roots_;

        StringHeteroHashMap<PendingUpdate>  pending_updates_;
        std::shared_mutex                   pending_mutex_;
        StringHeteroHashMap<VersionPointer> document_versions_;   // [Uri, Version]
        std::shared_mutex                   version_mutex_;

        StringHeteroHashSet                 include_affected_uris_;
        std::shared_mutex                   affected_mutex_;

        std::mutex                          task_mutex_;
        std::condition_variable_any         task_condition_;
        std::queue<LspTask>                 task_queue_;

        std::mutex                          submit_mutex_;
        std::condition_variable_any         submit_condition_;
        std::queue<LspSubmitItem>           submit_queue_;

        std::mutex                          update_mutex_;
        std::condition_variable_any         update_condition_;
        std::queue<std::function<void()>>   update_queue_;

        CancellationTokenTable              cancellation_tokens_; // [Request ID, Token]
        std::mutex                          cancellation_mutex_;
    };
}
