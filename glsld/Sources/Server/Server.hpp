#pragma once

#include <optional>
#include <string>
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

        void HandleDidOpen(Context& context);
        void HandleDidChange(Context& context);
        void HandleDidClose(Context& context);
        void HandleInitialized(Context& context); // 客户端确认初始化完成
        void HandleExit(Context& context);

        Router router_;
        Workspace workspace_;
        bool running_{ true };
    };
}
