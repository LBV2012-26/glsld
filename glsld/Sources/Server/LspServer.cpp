#include "stdafx.h"
#include "LspServer.hpp"

#include <cstddef>
#include <algorithm>
#include <charconv>
#include <exception>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "Base/FileSystem/Source.hpp"
#include "Base/Logger.hpp"
#include "Server/FunctionProviders.hpp"
#include "Server/JsonResponse.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    namespace {
        static constexpr int kDocumentClosedVersion = -114514;

        SourceLocation ConvertToParserPosition(const SourceFile* source_file, const auto& position) {
            std::size_t line      = position["line"];
            std::size_t character = position["character"];

            return SourceLocation(source_file, line + 1, character + 1);
        }

        nlohmann::json ConvertToLspPosition(const SourceLocation& location) {
            return {
                { "line",      location.line()   - 1 },
                { "character", location.column() - 1 }
            };
        }

        std::string NormalizeUri(std::string_view uri) {
            auto filename       = utils::UriToPath(uri);
            auto normalized_uri = utils::PathToUri(filename);
            return normalized_uri;
        }
    }

    LspServer::LspServer()
        : thread_pool_{ std::thread::hardware_concurrency() }
        , update_pool_{ std::thread::hardware_concurrency() }
        , workspace_{ thread_pool_ }
    {
        RegisterHandlers();

        diagnostic_engine_.SetCallback([this](std::string_view uri, int version, std::vector<Diagnostic> diagnostic) -> void {
            int current = [&]() -> int {
                std::shared_lock lock(pending_mutex_);
                auto it = document_versions_.find(uri);
                return it != document_versions_.end() ? it->second->load(std::memory_order::relaxed) : -1;
            }();

            if (current != version) {
                return;  // 过时，丢弃
            }

            nlohmann::json info = nlohmann::json::array();
            for (const auto& diagnostic : diagnostic) {
                info.push_back({
                    { "range", {
                        { "start", { { "line", diagnostic.line }, { "character", diagnostic.character } } },
                        { "end",   { { "line", diagnostic.end_line }, { "character", diagnostic.end_character } } }
                    } },
                    { "severity", std::to_underlying(diagnostic.severity) },
                    { "message",  diagnostic.message }
                });
            }

            EnqueueSubmit(LspSubmitItem{
                .payload       = { { "uri", uri }, { "diagnostics", std::move(info) } },
                .notify_method = "textDocument/publishDiagnostics",
                .kind          = LspSubmitItem::Kind::kNotification
            });
        });
    }

    void LspServer::Run() {
        std::jthread worker_thread([this]() -> void { WorkerLoop(); });
        std::jthread submit_thread([this]() -> void { SubmitLoop(); });
        std::jthread update_thread([this]() -> void { UpdateLoop(); });

        worker_thread.detach();
        submit_thread.detach();
        update_thread.detach();

        while (running_.load(std::memory_order::relaxed) && std::cin.good()) {
            auto message = ReadMessage();
            if (!message.has_value()) {
                break;
            }

            if (message->contains("method") && message->at("method") == "$/cancelRequest") {
                CancelRequest(*message);
                continue;
            }

            LspTask task{
                .method     = message->value("method", ""),
                .params     = message->value("params", nlohmann::json::object()),
                .is_request = message->contains("id")
            };

            if (task.is_request) {
                task.id    = message->at("id");
                auto token = std::make_shared<std::atomic<bool>>(false);
                {
                    std::lock_guard lock(cancellation_mutex_);
                    cancellation_tokens_[*task.id] = token;
                }

                task.cancellation_token = std::move(token);
            }

            EnqueueTask(std::move(task));
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

        router_.RegisterRequest("textDocument/references", [this](Context& context) -> nlohmann::json {
            return HandleReferences(context);
        });

        router_.RegisterRequest("textDocument/rename", [this](Context& context) -> nlohmann::json {
            return HandleRename(context);
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

        router_.RegisterNotification("textDocument/didSave", [this](Context& context) -> void {
            HandleDidSave(context);
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

        router_.RegisterNotification("workspace/didChangeConfiguration", [this](Context& context) -> void {
            HandleConfigure(context);
        });

        router_.RegisterNotification("glsld/removeConfiguration", [this](Context& context) -> void {
            HandleRemoveConfiguration(context);
        });

        router_.RegisterNotification("glsld/selectVariant", [this](Context& context) -> void {
            HandleChangeVariant(context);
        });

        router_.RegisterNotification("glsld/removeVariant", [this](Context& context) -> void {
            HandleRemoveVariant(context);
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

    void LspServer::WorkerLoop() {
        while (running_.load(std::memory_order::relaxed)) {
            LspTask task;
            {
                std::unique_lock lock(task_mutex_);
                task_condition_.wait(lock, [this]() -> bool {
                    return !task_queue_.empty() || !running_.load(std::memory_order::relaxed);
                });

                if (!running_.load(std::memory_order::relaxed)) {
                    return;
                }

                task = std::move(task_queue_.front());
                task_queue_.pop();
            }

            Context context{
                .method             = std::move(task.method),
                .request_id         = std::move(task.id),
                .params             = std::move(task.params),
                .cancellation_token = std::move(task.cancellation_token)
            };

            if (task.is_request) {
                thread_pool_.Submit([this, context = std::move(context)]() mutable -> void {
                    GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Received request: {}", context.method);
                    GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Method: {} with ID: {}",
                                    context.method, context.request_id.has_value() ? context.request_id->dump() : "null");

                    try {
                        router_.Dispatch(context, true);
                    } catch (const std::exception& e) {
                        context.error = std::make_pair(-32603, std::format("Internal Error: {}", e.what()));
                    }

                    LspSubmitItem item;
                    item.id = context.request_id;

                    if (context.cancelled()) {
                        item.kind          = LspSubmitItem::Kind::kError;
                        item.error_code    = -32800;
                        item.error_message = "Request cancelled.";
                    } else if (context.error.has_value()) {
                        item.kind          = LspSubmitItem::Kind::kError;
                        item.error_code    = context.error->first;
                        item.error_message = context.error->second;
                    } else {
                        item.kind          = LspSubmitItem::Kind::kResponse;
                        item.payload       = context.response.value_or(nlohmann::json(nullptr));
                    }

                    EnqueueSubmit(std::move(item));
                    GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Request {} completed, erased cancellation token.",
                                    context.request_id.value_or(nlohmann::json("null")).dump());

                    {
                        std::lock_guard lock(cancellation_mutex_);
                        cancellation_tokens_.erase(context.request_id);
                    }
                });
            } else {
                GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Received notification: {}", context.method);
                GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Method: {}", context.method);

                try {
                    router_.Dispatch(context, false);
                } catch (const std::exception& e) {
                    context.error = std::make_pair(-32603, std::format("Internal Error: {}", e.what()));
                }
            }
        }
    }

    void LspServer::SubmitLoop() {
        while (running_.load(std::memory_order::relaxed)) {
            LspSubmitItem item;
            {
                std::unique_lock lock(submit_mutex_);
                submit_condition_.wait(lock, [this]() -> bool {
                    return !submit_queue_.empty() || !running_.load(std::memory_order::relaxed);
                });

                if (!running_.load(std::memory_order::relaxed)) {
                    return;
                }

                item = std::move(submit_queue_.front());
                submit_queue_.pop();
            }

            if (item.kind == LspSubmitItem::Kind::kServerRequest) {
                SendServerRequest(*item.id, item.notify_method, item.payload);
            } else if (item.kind == LspSubmitItem::Kind::kResponse) {
                SendResponse(*item.id, item.payload);
            } else if (item.kind == LspSubmitItem::Kind::kError) {
                SendError(*item.id, item.error_code, item.error_message);
            } else if (item.kind == LspSubmitItem::Kind::kNotification) {
                SendNotification(item.notify_method, item.payload);
            }
        }
    }

    void LspServer::UpdateLoop() {
        while (running_.load(std::memory_order::relaxed)) {
            std::function<void()> work;
            {
                std::unique_lock lock(update_mutex_);
                update_condition_.wait(lock, [this]() -> bool {
                    return !update_queue_.empty() || !running_.load(std::memory_order::relaxed);
                });

                if (!running_.load(std::memory_order::relaxed)) {
                    return;
                }

                work = std::move(update_queue_.front());
                update_queue_.pop();
            }

            update_pool_.Submit(work);
        }
    }

    void LspServer::EnqueueTask(LspTask task) {
        {
            std::lock_guard lock(task_mutex_);
            task_queue_.push(std::move(task));
        }
        task_condition_.notify_one();
    }

    void LspServer::EnqueueSubmit(LspSubmitItem item) {
        {
            std::lock_guard lock(submit_mutex_);
            submit_queue_.push(std::move(item));
        }
        submit_condition_.notify_one();
    }

    void LspServer::EnqueueUpdate(const std::string& uri) {
        {
            std::lock_guard lock(update_mutex_);
            update_queue_.push([this, uri]() -> void {
                UpdateWorker(uri);
            });
        }
        update_condition_.notify_one();
    }

    void LspServer::CancelRequest(const nlohmann::json& message) {
        if (!message.contains("params")) {
            return;
        }

        const auto& params = message["params"];

        if (!params.contains("id")) {
            return;
        }

        const auto& key = params["id"];
        std::lock_guard lock(cancellation_mutex_);

        auto it = cancellation_tokens_.find(key);
        if (it != cancellation_tokens_.end()) {
            it->second->store(true, std::memory_order::relaxed);
            GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Cancelled request with params: {}.", params.dump());
        } else {
            GLSLD_LOG_ERROR(GLSLD_LOG_ROOT(), "Cancelled target {} not found.", key.dump());
        }

#ifdef _DEBUG
        for (const auto& [id, token] : cancellation_tokens_) {
            GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Pending cancellation token for request {}. Token status: {}.",
                            id.dump(), token->load(std::memory_order::relaxed));
        }
#endif
    }

    // Request Handlers
    // ----------------
    nlohmann::json LspServer::HandleInitialize(Context& context) {
        nlohmann::json capabilities;

        capabilities["textDocumentSync"] = {
            { "openClose", true },
            { "change", 1 },
            { "save", {
                { "includeText", false }
            } }
        };

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

        capabilities["workspace"] = {
            { "semanticTokens", {
                { "refreshSupport", true }
            } }
        };

        capabilities["definitionProvider"] = true;
        capabilities["referencesProvider"] = true;
        capabilities["renameProvider"]     = true;
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
        return {};
    }

    nlohmann::json LspServer::HandleDocumentSymbol(Context& context) {
        ABORT_IF_CANCELLED();
        const auto& origin_uri = context.params["textDocument"]["uri"];
        auto        uri        = NormalizeUri(origin_uri);
        const auto  snapshot   = ValidateAndGetDocument(context, uri);

        ABORT_IF_CANCELLED();
        if (snapshot == nullptr) {
            throw std::runtime_error("Document closed or not found.");
        }

        return ConvertScopeToDocumentSymbols(context, uri, snapshot->symbols.root_scope());
    }

    nlohmann::json LspServer::HandleSemanticTokens(Context& context) {
        ABORT_IF_CANCELLED();
        const auto& origin_uri = context.params["textDocument"]["uri"];
        auto        uri        = NormalizeUri(origin_uri);
        const auto  snapshot   = ValidateAndGetDocument(context, uri);

        if (snapshot == nullptr) {
            throw std::runtime_error("Document closed or not found.");
        }

        ABORT_IF_CANCELLED();
        const auto* source_file = workspace_.GetSource(uri);
        auto data = GetSemanticData(context, source_file, snapshot);

        return { { "data", data } };
    }

    nlohmann::json LspServer::HandleDefinition(Context& context) {
        ABORT_IF_CANCELLED();
        const auto& origin_uri = context.params["textDocument"]["uri"];
        const auto& position   = context.params["position"];
        auto        uri        = NormalizeUri(origin_uri);
        const auto  snapshot   = ValidateAndGetDocument(context, uri);

        if (snapshot == nullptr) {
            throw std::runtime_error("Document closed or not found.");
        }

        auto target = ConvertToParserPosition(workspace_.InternSource(uri), position);

        ABORT_IF_CANCELLED();
        if (auto include = GotoInclude(context, snapshot, target, workspace_.include_dirs())) {
            nlohmann::json result;

            result["uri"]                         = *include;
            result["range"]["start"]["line"]      = 0;
            result["range"]["start"]["character"] = 0;
            result["range"]["end"]["line"]        = 0;
            result["range"]["end"]["character"]   = 0;

            return result;
        }

        ABORT_IF_CANCELLED();
        auto symbols = GetDefinitionSymbols(context, snapshot, target, true);
        if (symbols.empty()) {
            return {};
        }

        auto response_array = nlohmann::json::array();

        for (const auto& symbol : symbols) {
            ABORT_IF_CANCELLED();
            auto start_line  = symbol->location.line()   - 1;
            auto start_char  = symbol->location.column() - 1;

            auto symbol_name = symbol->name;
            if (symbol->kind == SymbolKind::kFunctionDecl || symbol->kind == SymbolKind::kFunctionImpl) {
                symbol_name = utils::UnmangleFunctionName(symbol_name);
            }

            auto name_length = symbol_name.length();

            nlohmann::json result;

            result["uri"]                         = symbol->location.uri();
            result["range"]["start"]["line"]      = start_line;
            result["range"]["start"]["character"] = start_char;
            result["range"]["end"]["line"]        = start_line;
            result["range"]["end"]["character"]   = start_char + name_length;

            response_array.push_back(std::move(result));
        }

        return response_array;
    }

    nlohmann::json LspServer::HandleReferences(Context& context) {
        ABORT_IF_CANCELLED();
        const auto& origin_uri = context.params["textDocument"]["uri"];
        const auto& position   = context.params["position"];
        auto        uri        = NormalizeUri(origin_uri);
        const auto  snapshot   = ValidateAndGetDocument(context, uri);

        if (snapshot == nullptr) {
            throw std::runtime_error("Document closed or not found.");
        }

        auto target = ConvertToParserPosition(workspace_.InternSource(uri), position);

        ABORT_IF_CANCELLED();
        auto [locations, symbol] = GetReferences(context, snapshot, target, workspace_.global_index());
        if (symbol == nullptr) {
            return {};
        }

        nlohmann::json response = nlohmann::json::array();
        auto name_length = symbol != nullptr ? static_cast<int>(symbol->name.size()) : 1;

        for (const auto& location : locations) {
            response.push_back({
                { "uri", location.uri() },
                { "range", {
                    { "start", {{ "line", location.line() - 1 }, { "character", location.column() - 1 } } },
                    { "end",   {{ "line", location.line() - 1 }, { "character", location.column() - 1 + name_length } } }
                } }
            });
        }

        return response;
    }

    nlohmann::json LspServer::HandleRename(Context& context) {
        ABORT_IF_CANCELLED();
        const auto& origin_uri = context.params["textDocument"]["uri"];
        const auto& position   = context.params["position"];
        auto        uri        = NormalizeUri(origin_uri);
        const auto  snapshot   = ValidateAndGetDocument(context, uri);

        if (snapshot == nullptr) {
            throw std::runtime_error("Document closed or not found.");
        }

        auto target   = ConvertToParserPosition(workspace_.InternSource(uri), position);
        auto new_name = context.params["newName"].get<std::string>();

        ABORT_IF_CANCELLED();
        auto [locations, symbol] = GetReferences(context, snapshot, target, workspace_.global_index());
        if (symbol == nullptr) {
            return {};
        }

        nlohmann::json changes = nlohmann::json::object();

        for (const auto& location : locations) {
            if (location.uri().contains("Database/Meta/Builtin") || location.uri().contains("Database/Meta/Extensions")) {
                continue;
            }

            const auto& symbol_name = symbol->kind == SymbolKind::kFunctionDecl || symbol->kind == SymbolKind::kFunctionImpl
                                    ? utils::UnmangleFunctionName(symbol->name)
                                    : symbol->name;

            auto& edits = changes[location.uri()];
            edits.push_back({
                { "range", {
                    { "start", {{ "line", location.line() - 1 }, { "character", location.column() - 1 } } },
                    { "end",   {{ "line", location.line() - 1 }, { "character", location.column() - 1 + static_cast<int>(symbol_name.size()) } } }
                } },
                { "newText", new_name }
            });
        }

        return { { "changes", changes } };
    }

    nlohmann::json LspServer::HandleHover(Context& context) {
        ABORT_IF_CANCELLED();
        const auto& origin_uri = context.params["textDocument"]["uri"];
        const auto& position   = context.params["position"];
        auto        uri        = NormalizeUri(origin_uri);
        const auto  snapshot   = ValidateAndGetDocument(context, uri);

        if (snapshot == nullptr) {
            throw std::runtime_error("Document closed or not found.");
        }

        auto target = ConvertToParserPosition(workspace_.InternSource(uri), position);

        ABORT_IF_CANCELLED();
        auto symbols = GetDefinitionSymbols(context, snapshot, target, false);
        if (symbols.empty()) {
            return {};
        }

        std::string markdown;

        ABORT_IF_CANCELLED();
        if (symbols.size() == 1) {
            markdown = BuildHoverMarkdown(symbols.front(), snapshot, target, uri);
        } else {
            markdown = std::format("Ambiguous call (+{} candidates)\n\n---\n```glsl\n", symbols.size());
            for (const auto* symbol : symbols) {
                markdown += FormatFunctionSymbol(symbol, snapshot).full_spec;
                markdown += "\n";
            }

            markdown += "\n```";
        }

        nlohmann::json response;
        response["contents"]["kind"] = "markdown";
        response["contents"]["value"] = markdown;

        return response;
    }

    nlohmann::json LspServer::HandleInlayHints(Context& context) {
        ABORT_IF_CANCELLED();
        const auto& origin_uri = context.params["textDocument"]["uri"];
        auto        uri        = NormalizeUri(origin_uri);
        const auto  snapshot   = ValidateAndGetDocument(context, uri);

        if (snapshot == nullptr) {
            throw std::runtime_error("Document closed or not found.");
        }

        auto hints = GetInlayHints(context, snapshot);

        nlohmann::json response = nlohmann::json::array();
        for (auto& hint : hints) {
            ABORT_IF_CANCELLED();

            nlohmann::json result;

            result["position"]     = ConvertToLspPosition(*hint.location);
            result["label"]        = std::move(hint.label);
            result["kind"]         = 2;
            result["paddingRight"] = true;

            response.push_back(std::move(result));
        }

        return response;
    }

    namespace {
        auto ExtractParameterOffsets(std::string_view label) {
            std::vector<std::pair<std::size_t, std::size_t>> offsets;

            auto begin = label.find('(');
            auto end   = label.find(')');

            if (begin == std::string_view::npos || end == std::string_view::npos || end <= begin) {
                return offsets;
            }

            auto current = begin + 1;
            auto inner   = label.substr(current, end - current);
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
        }
    }

    nlohmann::json LspServer::HandleSignatureHelp(Context& context) {
        ABORT_IF_CANCELLED();
        const auto& origin_uri = context.params["textDocument"]["uri"];
        const auto& position   = context.params["position"];
        auto        uri        = NormalizeUri(origin_uri);
        const auto  snapshot   = ValidateAndGetDocument(context, uri);

        if (snapshot == nullptr) {
            throw std::runtime_error("Document closed or not found.");
        }

        auto target = ConvertToParserPosition(workspace_.InternSource(uri), position);

        ABORT_IF_CANCELLED();
        auto signature_help = GetSignatureHelp(context, snapshot, target);
        if (!signature_help.has_value()) {
            return {};
        }

        nlohmann::json response = nlohmann::json::array();
        for (const auto* symbol : signature_help->candidates) {
            ABORT_IF_CANCELLED();

            auto label   = FormatFunctionSymbol(symbol, snapshot).full_spec;
            auto offsets = ExtractParameterOffsets(label);

            nlohmann::json params = nlohmann::json::array();
            for (const auto& offset : offsets) {
                params.push_back({
                    { "label", { offset.first, offset.second } }
                });
            }

            nlohmann::json item;
            item["label"]      = label;
            item["parameters"] = std::move(params);

            response.push_back(std::move(item));
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
        ABORT_IF_CANCELLED();
        const auto& origin_uri = context.params["textDocument"]["uri"];
        const auto& position   = context.params["position"];
        auto        uri        = NormalizeUri(origin_uri);
        const auto  snapshot   = ValidateAndGetDocument(context, uri);

        if (snapshot == nullptr) {
            throw std::runtime_error("Document closed or not found.");
        }

        auto target = ConvertToParserPosition(workspace_.InternSource(uri), position);

        if (context.params["context"]["triggerCharacter"] == ".") {
            return GetFieldCompletionItems(context, snapshot, target, workspace_.type_member_index());
        }

        if (context.params["context"]["triggerCharacter"] == "\"" ||
            context.params["context"]["triggerCharacter"] == "<"  ||
            context.params["context"]["triggerCharacter"] == "/")
        {
            return GetIncludeCompletionItems(context, snapshot, target, workspace_.include_dirs());
        }

        if (auto include_items = GetIncludeCompletionItems(context, snapshot, target, workspace_.include_dirs());
            !include_items.empty())
        {
            return include_items;
        }

        if (auto extension_items = GetExtensionCompletionItems(context, snapshot, target);
            !extension_items.empty())
        {
            return extension_items;
        }

        return GetCompletionItems(context, snapshot, target);
    }

    // Notification Handlers
    // ---------------------
    void LspServer::HandleDidOpen(Context& context) {
        const auto& document   = context.params["textDocument"];
        const auto& origin_uri = document["uri"];
        const auto& text       = document["text"];
        int version            = document["version"];

        auto deadline = std::chrono::steady_clock::now();
        auto uri      = NormalizeUri(origin_uri);

        {
            std::scoped_lock lock(pending_mutex_, version_mutex_);
            pending_updates_[uri] = {
                .text            = text,
                .deadline        = deadline,
                .version_replica = version,
                .open_document   = true
            };

            document_versions_[uri] = std::make_shared<std::atomic<int>>(version);
        }

        EnqueueUpdate(uri);
        document_uris_.insert(std::move(uri));
    }

    void LspServer::HandleDidChange(Context& context) {
        const auto& document   = context.params["textDocument"];
        const auto& origin_uri = document["uri"];
        int version            = document["version"];

        const auto& changes = context.params["contentChanges"];
        if (changes.empty() || !changes[0].contains("text")) {
            return;
        }
        const auto& new_text = changes[0]["text"];

        using namespace std::chrono_literals;
        auto deadline = std::chrono::steady_clock::now() + 0ms;
        auto uri      = NormalizeUri(origin_uri);

        {
            std::scoped_lock lock(pending_mutex_, version_mutex_);
            pending_updates_[uri] = {
                .text            = new_text,
                .deadline        = deadline,
                .version_replica = version,
                .open_document   = false
            };

            auto it = document_versions_.find(uri);
            if (it == document_versions_.end()) {
                document_versions_.try_emplace(uri, std::make_shared<std::atomic<int>>(version));
                GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Document {} version initialized to {}, pending update scheduled.", uri, version);
            } else {
                GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Document {} version updated from {} to {}, pending update scheduled.",
                                uri, it->second->load(std::memory_order::relaxed), version);
                it->second->store(version, std::memory_order::relaxed);
            }
        }

        EnqueueUpdate(uri);
    }

    void LspServer::HandleDidSave(Context& context) {
        const auto& document   = context.params["textDocument"];
        const auto& origin_uri = document["uri"];

        auto uri = NormalizeUri(origin_uri);
        workspace_.InvalidateInclude(uri);

        auto affected_uris = workspace_.GetAffectedDocuments(uri);

        std::scoped_lock lock(version_mutex_, affected_mutex_);
        for (const auto& affected_uri : affected_uris) {
            if (!document_versions_.contains(affected_uri)) {
                continue;
            }

            include_affected_uris_.insert(affected_uri);
        }
    }

    void LspServer::HandleDidClose(Context& context) {
        const auto& origin_uri = context.params["textDocument"]["uri"];
        auto uri = NormalizeUri(origin_uri);
        workspace_.RemoveDocument(uri);

        {
            std::scoped_lock lock(pending_mutex_, version_mutex_);
            pending_updates_.erase(uri);

            auto it = document_versions_.find(uri);
            if (it != document_versions_.end()) {
                it->second->store(kDocumentClosedVersion, std::memory_order::relaxed);
                document_versions_.erase(it);
            }
        }

        LspSubmitItem item{
            .payload = {
                { "uri", std::move(uri) },
                { "diagnostics", nlohmann::json::array() }
            },
            .notify_method = "textDocument/publishDiagnostics",
            .kind          = LspSubmitItem::Kind::kNotification
        };

        EnqueueSubmit(std::move(item));
        document_uris_.erase(uri);
        include_affected_uris_.erase(uri);
    }

    void LspServer::HandleInitialized(Context& context) {}

    void LspServer::HandleExit(Context& context) {
        running_.store(false);
    }

    void LspServer::HandleConfigure(Context& context) {
        const auto& settings = context.params["settings"];
        if (!settings.contains("glsld")) {
            return;
        }

        const auto& glsld = settings["glsld"];
        if (!glsld.contains("shaderConfig")) {
            return;
        }

        const auto& shader_config = glsld["shaderConfig"];
        if (!shader_config.is_object()) {
            return;
        }

        for (const auto& [key, value] : shader_config.items()) {
            if (!value.is_object()) {
                continue;
            }

            ExtraShaderConfig config;

            if (value.contains("version") && value["version"].is_string())
                config.version = value["version"].get<std::string>();
            if (value.contains("shaderStage") && value["shaderStage"].is_string())
                config.shader_stage = value["shaderStage"].get<std::string>();
            if (value.contains("targetEnv") && value["targetEnv"].is_string())
                config.target_env = value["targetEnv"].get<std::string>();
            if (value.contains("targetSpv") && value["targetSpv"].is_string())
                config.target_spv = value["targetSpv"].get<std::string>();

            auto normalized_uri = NormalizeUri(key);

            workspace_.Configure(std::move(normalized_uri), std::move(config));
            RefreshDocument(normalized_uri);
        }
    }

    void LspServer::HandleRemoveConfiguration(Context& context) {
        const auto& origin_uri = context.params["uri"];
        auto uri = NormalizeUri(origin_uri);

        workspace_.RemoveConfiguration(uri);
        RefreshDocument(uri);
    }

    void LspServer::HandleChangeVariant(Context& context) {
        const auto& macros = context.params["macros"];
        const auto& scope  = context.params.value("scope", "global");

        ActiveVariant variant;
        variant.variant_name = context.params["variant"];

        for (const auto& [name, value] : macros.items()) {
            variant.macros[name] = MacroDefination{
                .is_function = false,
                .original_token = Token{
                    .text = name,
                    .type = TokenType::kIdentifier
                },
                .replacement_list = { Token{
                    .text = value.is_string() ? value.get<std::string>() : "1",
                    .type = TokenType::kNumberLiteral
                } }
            };
        }

        if (scope == "global") {
            workspace_.ChangeVariant(VariantType::kShared, std::move(variant));
            RebuildDocuments();
        } else {
            const auto& origin_uri = context.params["textDocument"]["uri"];
            auto uri = NormalizeUri(origin_uri);
            workspace_.ChangeVariant(VariantType::kPerFile, std::move(variant), uri);
            RefreshDocument(uri);
        }
    }

    void LspServer::HandleRemoveVariant(Context& context) {
        const auto& scope = context.params.value("scope", "global");

        if (scope == "global") {
            workspace_.RemoveVariant(VariantType::kShared);
            RebuildDocuments();
        } else {
            const auto& origin_uri = context.params["textDocument"]["uri"];
            auto uri = NormalizeUri(origin_uri);
            workspace_.RemoveVariant(VariantType::kPerFile, uri);
            RefreshDocument(uri);
        }
    }

    void LspServer::RefreshDocument(std::string_view uri) {
        VersionPointer version_pointer;
        int version_replica = 0;
        {
            std::shared_lock lock(version_mutex_);
            auto it = document_versions_.find(uri);
            if (it == document_versions_.end()) {
                return;
            }

            version_pointer = it->second;
            version_replica = version_pointer->load(std::memory_order::relaxed);
        }

        auto snapshot = workspace_.GetDocumentSnapshot(uri);
        if (snapshot == nullptr) {
            return;
        }

        Update(uri, snapshot->source, version_replica, version_pointer, false);

        LspSubmitItem item{
            .id            = server_request_id_.fetch_add(1, std::memory_order::relaxed),
            .notify_method = "workspace/semanticTokens/refresh",
            .kind          = LspSubmitItem::Kind::kServerRequest
        };

        EnqueueSubmit(std::move(item));
    }

    void LspServer::RebuildDocuments() {
        std::vector<StringHeteroHashMap<VersionPointer>::iterator> exists;
        auto total = 0uz;

        std::shared_lock lock(version_mutex_);

        for (const auto& uri : document_uris_) {
            auto it = document_versions_.find(uri);
            if (it != document_versions_.end()) {
                exists.push_back(std::move(it));
                ++total;
            }
        }

        if (total == 0) {
            return;
        }

        auto counter = std::make_shared<std::atomic<std::size_t>>(0);

        for (const auto& it : exists) {
            const auto& uri      = it->first;
            auto version_pointer = it->second;
            auto version_replica = version_pointer->load(std::memory_order::relaxed);

            update_pool_.Submit([this, counter, total, uri, version_replica, version_pointer]() -> void {
                auto snapshot = workspace_.GetDocumentSnapshot(uri);
                if (snapshot != nullptr) {
                    Update(uri, snapshot->source, version_replica, version_pointer, false);
                }

                if (counter->fetch_add(1, std::memory_order::relaxed) + 1 == total) {
                    LspSubmitItem item{
                        .id            = server_request_id_.fetch_add(1, std::memory_order::relaxed),
                        .notify_method = "workspace/semanticTokens/refresh",
                        .kind          = LspSubmitItem::Kind::kServerRequest
                    };

                    EnqueueSubmit(std::move(item));
                }
            });
        }
    }

    void LspServer::UpdateWorker(std::string_view uri) {
        std::chrono::steady_clock::time_point deadline;
        {
            std::shared_lock lock(pending_mutex_);
            auto it = pending_updates_.find(uri);
            if (it == pending_updates_.end()) {
                return;
            }

            deadline = it->second.deadline;
        }

        if (std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_until(deadline);
        }

        PickupPendingUpdate(uri);
    }

    void LspServer::PickupPendingUpdate(std::string_view uri) {
        std::string    text;
        VersionPointer version_pointer;
        int            version_replica = 0;
        bool           open_document   = false;
        {
            std::scoped_lock lock(pending_mutex_, version_mutex_);

            auto pending_it = pending_updates_.find(uri);
            if (pending_it == pending_updates_.end()) {
                return;
            }

            auto version_it = document_versions_.find(uri);
            if (version_it == document_versions_.end()) {
                return;
            }

            auto& update = pending_it->second;
            if (update.version_replica != version_it->second->load(std::memory_order::relaxed)) {
                return;
            }

            text            = std::move(update.text);
            version_pointer = version_it->second;
            version_replica = update.version_replica;
            open_document   = update.open_document;

            pending_updates_.erase(pending_it);
        }

        Update(uri, text, version_replica, version_pointer, open_document);
    }

    void LspServer::Update(std::string_view uri, std::string_view text, int version_replica, VersionPointer version_pointer, bool open_document) {
        workspace_.UpdateDocument(uri, text, version_replica, version_pointer, open_document);
        ready_condition_.notify_all();
        SubmitDiagnositcTask(uri, text, utils::UriToPath(uri).generic_string(), version_replica, version_pointer);
    }

    namespace {
        bool StartsWithVersion(std::string_view source) {
            auto trimmed = source;
            // 跳过空白行
            while (!trimmed.empty() && (trimmed.front() == ' '  || trimmed.front() == '\t' ||
                                        trimmed.front() == '\r' || trimmed.front() == '\n'))
            {
                trimmed.remove_prefix(1);
            }
            // 跳过 // 注释
            while (trimmed.starts_with("//")) {
                auto eol = trimmed.find('\n');
                if (eol == std::string_view::npos) {
                    break;
                }

                trimmed.remove_prefix(eol + 1);
                while (!trimmed.empty() && (trimmed.front() == ' '  || trimmed.front() == '\t' ||
                                            trimmed.front() == '\r' || trimmed.front() == '\n'))
                {
                    trimmed.remove_prefix(1);
                }
            }
            // 跳过 /* */ 注释
            while (trimmed.starts_with("/*")) {
                auto end = trimmed.find("*/", 2);
                if (end == std::string_view::npos) {
                    break;
                }

                trimmed.remove_prefix(end + 2);
                while (!trimmed.empty() && (trimmed.front() == ' '  || trimmed.front() == '\t' ||
                                            trimmed.front() == '\r' || trimmed.front() == '\n'))
                {
                    trimmed.remove_prefix(1);
                }
            }

            return trimmed.starts_with("#version");
        }
    }

    void LspServer::SubmitDiagnositcTask(
        std::string_view uri,
        std::string_view source,
        std::string_view filename,
        int version_replica,
        VersionPointer version_pointer)
    {
        if (filename.contains("Database/Meta/Builtin") || filename.contains("Database/Meta/Extensions")) {
            return;
        }

        DiagnosticTask task{
            .uri             = std::string(uri),
            .filename        = std::string(filename),
            .include_dirs    = workspace_.include_dirs(),
            .version_replica = version_replica,
            .version_pointer = version_pointer
        };

        const auto& shader_configs = workspace_.shader_configs();
        auto it = shader_configs.find(uri);

        if (StartsWithVersion(source) || it == shader_configs.end()) {
            task.source = std::string(source);
        } else if (it->second.version.has_value()) {
            task.source = std::format("#version {}\n#line 1\n{}\n\nvoid main() {{}}\n", *it->second.version, source);
        }

        if (it != shader_configs.end()) {
            task.shader_stage = it->second.shader_stage.value_or("");
            task.target_env   = it->second.target_env.value_or("");
            task.target_spv   = it->second.target_spv.value_or("");
        }

        diagnostic_engine_.Submit(std::move(task));
    }

    std::shared_ptr<const Document> LspServer::ValidateAndGetDocument(const Context& context, std::string_view uri) {
        bool need_update = false;
        {
            std::unique_lock lock(affected_mutex_);
            if (include_affected_uris_.contains(uri)) {
                need_update = true;
                include_affected_uris_.erase(uri);
            }
        }

        if (need_update) {
            PickupPendingUpdate(uri);
        }

        while (true) {
            GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Checking context cancellation for document {}. Request ID: {}.", uri, context.request_id->dump());
            if (context.cancelled()) {
                GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Validation for document {} exit beacuse request {} cancelled.", uri, context.request_id->dump());
                return nullptr;
            }

            int expected_version = 0;
            {
                std::shared_lock lock(version_mutex_);
                auto it = document_versions_.find(uri);
                if (it != document_versions_.end()) {
                    expected_version = it->second->load(std::memory_order::relaxed);
                } else {
                    expected_version = kDocumentClosedVersion;
                }
            }

            auto snapshot = workspace_.GetDocumentSnapshot(uri);
            GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Expected version of document {} is {}, snapshot version is {}. Request ID: {}.",
                            uri, expected_version, snapshot != nullptr ? snapshot->version : -1919810, context.request_id->dump());

            if (snapshot != nullptr && snapshot->version >= expected_version) {
                GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Document {} is ready with version {}. Request ID: {}.", uri, snapshot->version, context.request_id->dump());
                return snapshot;
            } else if (expected_version == kDocumentClosedVersion) {
                GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Document {} is closed. Request ID: {}.", uri, context.request_id->dump());
                return nullptr;
            }

            std::unique_lock lock(ready_mutex_);
            ready_condition_.wait(lock, [this, &context, &uri, expected_version]() -> bool {
                if (context.cancelled()) {
                    GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Waiting for document {} exit because request {} cancelled while waiting.", uri, context.request_id->dump());
                    return true;
                }

                auto snapshot = workspace_.GetDocumentSnapshot(uri);
                if (snapshot != nullptr && snapshot->version >= expected_version) {
                    GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Document {} updated to version {}, expected version was {}. Waking up. Request ID: {}.",
                                    uri, snapshot->version, expected_version, context.request_id->dump());
                    return true;
                }

                if (expected_version == kDocumentClosedVersion) {
                    GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Document {} is closed while waiting. Request ID: {}.", uri, context.request_id->dump());
                    return true;
                }

                return false;
            });
        }
    }
}
