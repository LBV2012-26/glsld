#pragma once

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "Analyzer/SymbolTable.hpp"
#include "Analyzer/Token.hpp"
#include "Server/Context.hpp"
#include "Server/Router.hpp"
#include "Utils/Utils.hpp"

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

        void HandleDidOpen(Context& context);
        void HandleDidChange(Context& context);
        void HandleDidClose(Context& context);
        void HandleInitialized(Context& context); // 客户端确认初始化完成
        void HandleExit(Context& context);

        utils::StringHeteroHashTable<std::string, DocumentSymbols> documents_;
        utils::StringHeteroHashTable<std::string, std::vector<Token>> tokens_;
        Router router_;
        bool running_{ true };
    };
}
