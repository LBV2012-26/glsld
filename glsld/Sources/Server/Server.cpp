#include "stdafx.h"
#include "Server.hpp"

#include <cstdio>
#include <charconv>
#include <exception>
#include <iostream>
#include <print>
#include <span>
#include <utility>

#include "Analyzer/Syntax/Parser.hpp"
#include "Base/Logger.hpp"
#include "Server/JsonResponse.hpp"
#include "Server/LspProviders.hpp"

namespace glsld {
    namespace {
        SourceLocation ConvertToParserPosition(const auto& position) {
            std::size_t line      = position["line"];
            std::size_t character = position["character"];

            SourceLocation target{
                .line   = line + 1,
                .column = character + 1
            };

            return target;
        }

        nlohmann::json ConvertToLspPosition(SourceLocation location) {
            return {
                { "line",      location.line   - 1 },
                { "character", location.column - 1 }
            };
        }
    }

    LspServer::LspServer() {
        RegisterHandlers();

        worker_thread_ = std::jthread([this]() -> void {
            UpdateWorker();
        });
    }

    void LspServer::Run() {
        while (running_.load() && std::cin.good()) {
            auto message_opt = ReadMessage();
            if (!message_opt) {
                break;
            }

            auto& message = *message_opt;

            std::string method;
            if (message.contains("method")) {
                method = message["method"];
            }

            nlohmann::json params;
            if (message.contains("params")) {
                params = message["params"];
            }

            Context context{
                .method = method,
                .params = params
            };

            bool is_request = message.contains("id");

            try {
                router_.Dispatch(context, is_request);
                if (is_request) {
                    int id = message["id"];
                    if (context.error) {
                        SendError(id, context.error->first, context.error->second);
                    } else if (context.response) {
                        SendResponse(id, *context.response);
                    } else {
                        SendResponse(id, nullptr);
                    }
                }
            } catch (const std::exception& e) {
                // TODO
            }
        }
    }

    void LspServer::RegisterHandlers() {
        router_.RegisterRequest("initialize", [this](Context& context) -> nlohmann::json {
            return HandleInitialize(context);
        });

        router_.RegisterRequest("shutdown", [this](Context& context) -> nlohmann::json {
            return HandleShutdown(context);
        });

        router_.RegisterRequest("textDocument/documentSymbol", [this](Context& context) -> nlohmann::json {
            return HandleDocumentSymbol(context);
        });

        router_.RegisterRequest("textDocument/semanticTokens/full", [this](Context& context) -> nlohmann::json {
            return HandleSemanticTokens(context);
        });

        router_.RegisterRequest("textDocument/definition", [this](Context& context) -> nlohmann::json {
            return HandleDefinition(context);
        });

        router_.RegisterRequest("textDocument/hover", [this](Context& context) -> nlohmann::json {
            return HandleHover(context);
        });

        router_.RegisterRequest("textDocument/inlayHint", [this](Context& context) -> nlohmann::json {
            return HandleInlayHints(context);
        });

        router_.RegisterNotification("textDocument/didOpen", [this](Context& context) -> void {
            HandleDidOpen(context);
        });

        router_.RegisterNotification("textDocument/didChange", [this](Context& context) -> void {
            HandleDidChange(context);
        });

        router_.RegisterNotification("textDocument/didClose", [this](Context& context) -> void {
            HandleDidClose(context);
        });

        router_.RegisterNotification("initialized", [this](Context& context) -> void {
            HandleInitialized(context);
        });

        router_.RegisterNotification("exit", [this](Context& context) -> void {
            HandleExit(context);
        });
    }

    std::optional<nlohmann::json> LspServer::ReadMessage() {
        int length = 0;
        while (true) {
            std::string line;
            std::getline(std::cin, line);

            if (line.empty() && std::cin.eof())
                return std::nullopt;
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            if (line.empty())
                break;

            if (line.starts_with("Content-Length: ")) {
                std::string num_str = line.substr(16);
                std::from_chars(num_str.data(), num_str.data() + num_str.size(), length);
            }
        }

        if (length <= 0) {
            return std::nullopt;
        }

        std::vector<char> buffer(length);
        std::cin.read(buffer.data(), length);
        if (std::cin.gcount() != length) {
            return std::nullopt;
        }

        try {
            return nlohmann::json::parse(buffer.begin(), buffer.end());
        } catch (...) {
            return std::nullopt;
        }
    }

    void LspServer::SendMessage(const nlohmann::json& message) {
        std::string content = message.dump();
        std::print("Content-Length: {}\r\n\r\n{}", content.length(), content);
        std::fflush(stdout);
    }

    nlohmann::json LspServer::HandleInitialize(Context& context) {
        nlohmann::json capabilities;

        capabilities["textDocumentSync"]       = 1;
        capabilities["documentSymbolProvider"] = true;
        capabilities["hoverProvider"]          = true;
        capabilities["inlayHintProvider"]      = true;

        static const std::vector<std::string> kTokenTypes{
            "namespace",    // 0
            "type",         // 1
            "class",        // 2
            "enum",         // 3
            "interface",    // 4
            "struct",       // 5
            "typeParameter",// 6
            "parameter",    // 7
            "variable",     // 8
            "property",     // 9
            "enumMember",   // 10
            "event",        // 11
            "function",     // 12
            "method",       // 13
            "macro",        // 14
            "keyword",      // 15
            "modifier",     // 16
            "comment",      // 17
            "string",       // 18
            "number",       // 19
            "regexp",       // 20
            "operator",     // 21
            "decorator",    // 22
            "primitive"     // 23
        };

        static const std::vector<std::string> kTokenModifiers{
            "declaration",   // 1 (bit 0)
            "definition",    // 2 (bit 1)
            "readonly",      // 4 (bit 2)
            "static",        // 8 (bit 3)
            "deprecated",    // ...
            "abstract",
            "async",
            "modification",
            "documentation",
            "defaultLibrary"
        };

        capabilities["semanticTokensProvider"] = {
            {
                "legend", {
                    { "tokenTypes", kTokenTypes },
                    { "tokenModifiers", kTokenModifiers }
                }
            },
            { "full", true }
        };

        capabilities["definitionProvider"] = true;

        // 4. 支持补全 (未来实现)
        // capabilities["completionProvider"] = {
        //     {"resolveProvider", false},
        //     {"triggerCharacters", {".", "#"}}
        // };

        return {
            { "capabilities", capabilities },
            { "serverInfo", {
                { "name", "glsld" },
                { "version", "0.1.0" }
            } }
        };
    }

    nlohmann::json LspServer::HandleShutdown(Context& context) {
        return nullptr;
    }

    nlohmann::json LspServer::HandleDocumentSymbol(Context& context) {
        const auto& uri = context.params["textDocument"]["uri"];

        const auto [snapshot, error_response] = ValidateAndGetDocument(uri);
        if (snapshot == nullptr) {
            return error_response;
        }

        return ConvertScopeToDocumentSymbols(snapshot->symbols.root_scope());
    }

    nlohmann::json LspServer::HandleSemanticTokens(Context& context) {
        const auto& uri = context.params["textDocument"]["uri"];

        const auto [snapshot, error_response] = ValidateAndGetDocument(uri);
        if (snapshot == nullptr) {
            return error_response;
        }

        auto data = GetSemanticData(snapshot);
        return { { "data", data } };
    }

    nlohmann::json LspServer::HandleDefinition(Context& context) {
        const auto& uri      = context.params["textDocument"]["uri"];
        const auto& position = context.params["position"];

        auto target = ConvertToParserPosition(position);

        const auto [snapshot, error_response] = ValidateAndGetDocument(uri);
        if (snapshot == nullptr) {
            return error_response;
        }

        auto symbols = GetDefinitionSymbols(snapshot, target);
        if (symbols.empty()) {
            return nlohmann::json::array();
        }

        auto response_array = nlohmann::json::array();

        for (const auto& symbol : symbols) {
            std::size_t start_line  = symbol->location.line   - 1;
            std::size_t start_char  = symbol->location.column - 1;
            std::size_t name_length = symbol->name.length();

            nlohmann::json result;

            result["uri"]                         = uri;
            result["range"]["start"]["line"]      = start_line;
            result["range"]["start"]["character"] = start_char;
            result["range"]["end"]["line"]        = start_line;
            result["range"]["end"]["character"]   = start_char + name_length;

            response_array.push_back(result);
        }

        return response_array;
    }

    nlohmann::json LspServer::HandleHover(Context& context) {
        const auto& uri      = context.params["textDocument"]["uri"];
        const auto& position = context.params["position"];

        const auto [snapshot, error_response] = ValidateAndGetDocument(uri);
        if (snapshot == nullptr) {
            return error_response;
        }

        auto target  = ConvertToParserPosition(position);
        auto symbols = GetDefinitionSymbols(snapshot, target);
        if (symbols.empty()) {
            return {};
        }

        nlohmann::json response;
        std::string markdown;

        if (symbols.size() == 1) {
            const auto* symbol = symbols.front();
            markdown = "```glsl\n";
            markdown += FormatSymbol(symbol);
            markdown += "\n```";
        } else {
            markdown = std::format("```glsl\nAmbiguous call (+{} candidates)\n---\n", symbols.size());
            for (const auto* symbol : symbols) {
                markdown += FormatSymbol(symbol);
                markdown += "\n";
            }

            markdown += "\n```";
        }

        response["contents"]["kind"] = "markdown";
        response["contents"]["value"] = markdown;

        return response;
    }

    nlohmann::json LspServer::HandleInlayHints(Context& context) {
        const auto& uri = context.params["textDocument"]["uri"];

        const auto [snapshot, error_response] = ValidateAndGetDocument(uri);
        if (snapshot == nullptr) {
            return error_response;
        }

        auto hints = GetInlayHints(snapshot);

        nlohmann::json response = nlohmann::json::array();
        for (const auto& hint : hints) {
            nlohmann::json result;

            result["position"]     = ConvertToLspPosition(hint.location);
            result["label"]        = hint.label;
            result["kind"]         = 2;
            result["paddingRight"] = true;

            response.push_back(result);
        }

        return response;
    }

    void LspServer::HandleDidOpen(Context& context) {
        const auto& document  = context.params["textDocument"];
        std::string_view uri  = document["uri"];
        std::string_view text = document["text"];
        int version           = document["version"];

        workspace_.UpdateDocument(uri, text, version);
    }

    void LspServer::HandleDidChange(Context& context) {
        const auto& document = context.params["textDocument"];
        const auto& uri = document["uri"];
        int version = document["version"];

        const auto& changes = context.params["contentChanges"];
        if (changes.empty() || !changes[0].contains("text")) {
            return;
        }
        const auto& new_text = changes[0]["text"];

        {
            using namespace std::chrono_literals;
            std::unique_lock lock(update_mutex_);

            latest_received_versions_[uri] = version;
            auto deadline = std::chrono::steady_clock::now() + 200ms;

            pending_updates_[uri] = {
                .text     = new_text,
                .deadline = deadline,
                .version  = version
            };
        }

        update_condition_.notify_one();
    }

    void LspServer::HandleDidClose(Context& context) {
        std::string_view uri = context.params["textDocument"]["uri"];
        workspace_.RemoveDocument(uri);

        SendNotification("textDocument/publishDiagnostics", {
            { "uri", uri },
            { "diagnostics", nlohmann::json::array() } // 发送空数组清空错误
        });
    }

    void LspServer::HandleInitialized(Context& context) {}

    void LspServer::HandleExit(Context& context) {
        running_.store(false);
    }

    void LspServer::UpdateWorker() {
        while (running_.load()) {
            std::unique_lock lock(update_mutex_);

            update_condition_.wait(lock, [this]() -> bool {
                return !pending_updates_.empty() || !running_.load();
            });

            if (!running_.load()) {
                break;
            }

            auto now = std::chrono::steady_clock::now();

            for (auto it = pending_updates_.begin(); it != pending_updates_.end();) {
                if (now >= it->second.deadline) {
                    std::string uri  = it->first;
                    std::string text = std::move(it->second.text);
                    int version      = it->second.version;
                    it = pending_updates_.erase(it);

                    lock.unlock();
                    Update(uri, text, version);
                    lock.lock();
                } else {
                    update_condition_.wait_until(lock, it->second.deadline);
                    break;
                }
            }
        }
    }

    void LspServer::Update(std::string_view uri, std::string_view text, int version) {
        workspace_.UpdateDocument(uri, text, version);

        // TODO
        SendNotification("textDocument/publishDiagnostics", {
            { "uri", uri },
            { "version", version },
            { "diagnostics", nlohmann::json::array() } // 发送空数组清空错误
        });
    }

    std::pair<std::shared_ptr<const Document>, nlohmann::json> LspServer::ValidateAndGetDocument(const std::string& uri) const {
        int target_version = 0;
        {
            std::lock_guard lock(update_mutex_);
            if (latest_received_versions_.contains(uri)) {
                target_version = latest_received_versions_.at(uri);
            }
        }

        const auto snapshot = workspace_.GetDocumentSnapshot(uri);
        if (snapshot == nullptr || snapshot->version < target_version) {
            nlohmann::json error_response;
            error_response = {
                { "code", -32801 },
                { "message", "Document is out of date, parsing in progress." }
            };
            return { nullptr, error_response };
        }

        return { snapshot, {} };
    }
}
