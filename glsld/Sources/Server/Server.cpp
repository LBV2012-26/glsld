#include "stdafx.h"
#include "Server.hpp"

#include <cstddef>
#include <cstdio>
#include <charconv>
#include <exception>
#include <iostream>
#include <print>
#include <span>
#include <string_view>
#include <thread>
#include <utility>

#include "Analyzer/Syntax/Parser.hpp"
#include "Base/Logger.hpp"
#include "Server/FunctionProviders.hpp"
#include "Server/JsonResponse.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    namespace {
        SourceLocation ConvertToParserPosition(auto source_ref, const auto& position) {
            std::size_t line      = position["line"];
            std::size_t character = position["character"];

            return SourceLocation(source_ref, line + 1, character + 1);
        }

        nlohmann::json ConvertToLspPosition(const SourceLocation& location) {
            return {
                { "line",      location.line()   - 1 },
                { "character", location.column() - 1 }
            };
        }
    }

    LspServer::LspServer()
        : thread_pool_{ std::thread::hardware_concurrency() }
        , workspace_{ thread_pool_ }
    {
        RegisterHandlers();
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

        router_.RegisterRequest("textDocument/signatureHelp", [this](Context& context) -> nlohmann::json {
            return HandleSignatureHelp(context);
        });

        router_.RegisterRequest("textDocument/completion", [this](Context& context) -> nlohmann::json {
            return HandleCompletion(context);
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
            "defaultLibrary",
            "inactive"
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
        capabilities["hoverProvider"]      = true;
        capabilities["inlayHintProvider"]  = true;

        capabilities["signatureHelpProvider"] = {
            { "triggerCharacters", { "(", "," } }
        };

        capabilities["completionProvider"] = {
            { "triggerCharacters", { ".", "\"", "<", "/" }}
        };

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
        const auto snapshot = ValidateAndGetDocument(uri);

        return ConvertScopeToDocumentSymbols(uri, snapshot->symbols.root_scope());
    }

    nlohmann::json LspServer::HandleSemanticTokens(Context& context) {
        const auto& uri = context.params["textDocument"]["uri"];
        const auto snapshot = ValidateAndGetDocument(uri);

        auto data = GetSemanticData(uri, snapshot);
        return { { "data", data } };
    }

    nlohmann::json LspServer::HandleDefinition(Context& context) {
        const auto& uri      = context.params["textDocument"]["uri"];
        const auto& position = context.params["position"];

        const auto snapshot = ValidateAndGetDocument(uri);

        auto target  = ConvertToParserPosition(FileTable::Intern(uri), position);

        if (auto include = GotoInclude(snapshot, target, workspace_.include_dirs())) {
            nlohmann::json result;

            result["uri"]                         = *include;
            result["range"]["start"]["line"]      = 0;
            result["range"]["start"]["character"] = 0;
            result["range"]["end"]["line"]        = 0;
            result["range"]["end"]["character"]   = 0;

            return result;
        }

        auto symbols = GetDefinitionSymbols(snapshot, target, true);
        if (symbols.empty()) {
            return nlohmann::json::array();
        }

        auto response_array = nlohmann::json::array();

        for (const auto& symbol : symbols) {
            std::size_t start_line  = symbol->location.line()   - 1;
            std::size_t start_char  = symbol->location.column() - 1;

            std::string symbol_name = symbol->name;
            if (symbol->kind == SymbolKind::kFunctionDecl || symbol->kind == SymbolKind::kFunctionImpl) {
                symbol_name = utils::UnmangleFunctionName(symbol_name);
            }

            std::size_t name_length = symbol_name.length();

            nlohmann::json result;

            result["uri"]                         = symbol->location.uri();
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

        const auto snapshot = ValidateAndGetDocument(uri);

        auto target  = ConvertToParserPosition(FileTable::Intern(uri), position);
        auto symbols = GetDefinitionSymbols(snapshot, target, false);
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
        const auto snapshot = ValidateAndGetDocument(uri);

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

    nlohmann::json LspServer::HandleSignatureHelp(Context& context) {
        const auto& uri      = context.params["textDocument"]["uri"];
        const auto& position = context.params["position"];

        const auto snapshot = ValidateAndGetDocument(uri);
        auto target = ConvertToParserPosition(FileTable::Intern(uri), position);

        auto signature_help = GetSignatureHelp(snapshot, target);
        if (!signature_help) {
            return {};
        }

        auto ExtractParameterOffsets = [](std::string_view label) -> auto {
            std::vector<std::pair<std::size_t, std::size_t>> offsets;

            auto begin = label.find('(');
            auto end   = label.find(')');

            if (begin == std::string_view::npos || end == std::string_view::npos || end <= begin) {
                return offsets;
            }

            auto current = begin + 1;
            auto inner = label.substr(current, end - current);
            if (inner.empty() || inner == "void") {
                return offsets;
            }

            for (auto i = current; i != end; ++i) {
                if (label[i] == ',') {
                    offsets.emplace_back(current, i);
                    current = i + 1;

                    while (current < end && label[current] == ' ') {
                        ++current;
                    }
                }
            }

            if (current < end) {
                offsets.emplace_back(current, end);
            }

            return offsets;
        };

        nlohmann::json response = nlohmann::json::array();
        for (const auto* symbol : signature_help->candidates) {
            nlohmann::json item;

            auto label = FormatSymbol(symbol);
            item["label"] = label;

            auto offsets = ExtractParameterOffsets(label);
            nlohmann::json params = nlohmann::json::array();
            for (const auto& offset : offsets) {
                params.push_back({
                    { "label", { offset.first, offset.second } }
                });
            }
            item["parameters"] = params;
            response.push_back(item);
        }

        if (response.empty()) {
            return {};
        }

        return {
            { "signatures", response },
            { "activeSignature", signature_help->active_signature_index },
            { "activeParameter", signature_help->active_param_index }
        };
    }

    nlohmann::json LspServer::HandleCompletion(Context& context) {
        const auto& uri = context.params["textDocument"]["uri"];
        const auto& position = context.params["position"];

        const auto snapshot = ValidateAndGetDocument(uri);
        auto target = ConvertToParserPosition(FileTable::Intern(uri), position);

        if (context.params["context"]["triggerCharacter"] == ".") {
            return GetFieldCompletionItems(snapshot, target);
        }

        if (context.params["context"]["triggerKind"] == 1 ||
            context.params["context"]["triggerCharacter"] == "\"" ||
            context.params["context"]["triggerCharacter"] == "<"  ||
            context.params["context"]["triggerCharacter"] == "/")
        {
            return GetIncludeCompletionItems(snapshot, target, workspace_.include_dirs());
        }

        return GetCompletionItems(snapshot, target);
    }

    void LspServer::HandleDidOpen(Context& context) {
        const auto& document  = context.params["textDocument"];
        std::string_view uri  = document["uri"];
        std::string_view text = document["text"];

        auto version = std::make_shared<std::atomic<int>>(document["version"]);

        workspace_.UpdateDocument(uri, text, version->load(), version, true);
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

        using namespace std::chrono_literals;
        auto deadline = std::chrono::steady_clock::now() + 50ms;

        {
            std::lock_guard lock(update_mutex_);
            pending_updates_[uri] = {
                .text     = new_text,
                .deadline = deadline
            };

            if (!document_versions_.contains(uri)) {
                document_versions_[uri] = std::make_shared<std::atomic<int>>(version);
            } else {
                document_versions_[uri]->store(version);
            }
        }

        thread_pool_.Submit([this, uri, version]() -> void {
            UpdateWorker(uri, version);
        });
    }

    void LspServer::HandleDidClose(Context& context) {
        std::string_view uri = context.params["textDocument"]["uri"];
        workspace_.RemoveDocument(uri);

        {
            std::lock_guard lock(update_mutex_);
            pending_updates_.erase(uri);
            document_versions_.erase(uri);
        }

        SendNotification("textDocument/publishDiagnostics", {
            { "uri", uri },
            { "diagnostics", nlohmann::json::array() } // 发送空数组清空错误
        });
    }

    void LspServer::HandleInitialized(Context& context) {}

    void LspServer::HandleExit(Context& context) {
        running_.store(false);
    }

    void LspServer::UpdateWorker(const std::string& uri, int version_replica) {
        auto& update = pending_updates_[uri];
        if (std::chrono::steady_clock::now() < update.deadline) {
            std::this_thread::sleep_until(update.deadline);
        }

        auto current_version = document_versions_[uri];
        if (version_replica != current_version->load()) {
            return;
        }

        std::string text = std::move(update.text);
        {
            std::lock_guard lock(update_mutex_);
            pending_updates_.erase(uri);
        }

        Update(uri, text, version_replica);
        ready_condition_.notify_one();
    }

    void LspServer::Update(const std::string& uri, std::string_view text, int version_replica) {
        workspace_.UpdateDocument(uri, text, version_replica, document_versions_[uri]);

        // TODO
        SendNotification("textDocument/publishDiagnostics", {
            { "uri", uri },
            { "version", version_replica },
            { "diagnostics", nlohmann::json::array() } // 发送空数组清空错误
        });
    }

    std::shared_ptr<const Document> LspServer::ValidateAndGetDocument(const std::string& uri) const {
        int target_version = 0;
        {
            std::lock_guard lock(update_mutex_);
            auto it = document_versions_.find(uri);
            if (it != document_versions_.end()) {
                target_version = it->second->load();
            }
        }

        const auto snapshot = workspace_.GetDocumentSnapshot(uri);
        if (snapshot == nullptr || snapshot->version < target_version) {
            std::unique_lock lock(update_mutex_);
            using namespace std::chrono_literals;
            ready_condition_.wait_for(lock, 2s);
        }

        return snapshot;
    }
}
