#include "stdafx.h"
#include "LspServer.hpp"

#include <cstddef>
#include <charconv>
#include <exception>
#include <format>
#include <iostream>
#include <stdexcept>
#include <span>
#include <string_view>
#include <thread>
#include <utility>

#include "Analyzer/Syntax/Parser.hpp"
#include "Base/FileSystem/Source.hpp"
#include "Server/FunctionProviders.hpp"
#include "Server/JsonResponse.hpp"
#include "Utils/Utils.hpp"

#ifdef _DEBUG
#include "Base/Logger.hpp"
#endif

namespace glsld {
    namespace {
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
        , workspace_{ thread_pool_ }
    {
        RegisterHandlers();
    }

    void LspServer::Run() {
        thread_pool_.Submit([this]() -> void { WorkerLoop(); });
        thread_pool_.Submit([this]() -> void { SubmitLoop(); });

        while (running_.load() && std::cin.good()) {
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
                    cancellation_tokens_[task.id] = token;
                }

                task.cancelled_token = std::move(token);
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
        while (running_.load()) {
            LspTask task;
            {
                std::unique_lock lock(task_mutex_);
                task_condition_.wait(lock, [this]() -> bool {
                    return !task_queue_.empty() || !running_.load();
                });

                if (!running_.load()) {
                    return;
                }

                task = std::move(task_queue_.front());
                task_queue_.pop();
            }

            Context context{
                .method           = std::move(task.method),
                .request_id       = task.id,
                .params           = std::move(task.params),
                .cancelled_token  = std::move(task.cancelled_token)
            };

#ifdef _DEBUG
            GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Received {}: {}", task.is_request ? "request" : "notification", context.method);
            GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Method: {} with ID: {}", context.method, context.request_id.has_value() ? context.request_id->dump() : "null");
#endif

            try {
                router_.Dispatch(context, task.is_request);
            } catch (const std::exception& e) {
                context.error = std::make_pair(-32603, std::format("Internal Error: {}", e.what()));
            }

            if (!task.is_request) {
                continue;
            }

            LspSubmitItem item;
            item.id = task.id;

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

            {
                std::lock_guard lock(cancellation_mutex_);
                cancellation_tokens_.erase(task.id);
            }
        }
    }

    void LspServer::SubmitLoop() {
        while (running_.load()) {
            LspSubmitItem item;
            {
                std::unique_lock lock(submit_mutex_);
                submit_condition_.wait(lock, [this]() -> bool {
                    return !submit_queue_.empty() || !running_.load();
                });

                if (!running_.load()) {
                    return;
                }

                item = std::move(submit_queue_.front());
                submit_queue_.pop();
            }

            if (item.kind == LspSubmitItem::Kind::kResponse) {
                SendResponse(*item.id, item.payload);
            } else if (item.kind == LspSubmitItem::Kind::kError) {
                SendError(*item.id, item.error_code, item.error_message);
            } else if (item.kind == LspSubmitItem::Kind::kNotification) {
                SendNotification(item.notify_method, item.payload);
            }
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
#ifdef _DEBUG
            GLSLD_LOG_DEBUG(GLSLD_LOG_ROOT(), "Cancelled request with id: {}", key.dump());
#endif
        }
#ifdef _DEBUG
        else {
            GLSLD_LOG_ERROR(GLSLD_LOG_ROOT(), "Cancelled target {} not found.", key.dump());
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

            response_array.push_back(std::move(result));
        }

        return response_array;
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
            const auto* symbol = symbols.front();

            markdown = "```glsl\n";
            markdown += FormatSymbol(symbol);
            markdown += "\n```";
            markdown += "\n---\n";

            std::string defined_at;
            if (symbol->location.uri() == uri) {
                defined_at = "this file";
            } else {
                defined_at = utils::UriToPath(symbol->location.uri()).filename().generic_string();
            }

            markdown += std::format("Defined in {}, line {}", defined_at, symbol->location.line());
        } else {
            markdown = std::format("```glsl\nAmbiguous call (+{} candidates)\n---\n", symbols.size());
            for (const auto* symbol : symbols) {
                markdown += FormatSymbol(symbol);
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

            auto label   = FormatSymbol(symbol);
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
            return GetFieldCompletionItems(context, snapshot, target);
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
            std::lock_guard lock(update_mutex_);
            pending_updates_[uri] = {
                .text     = text,
                .deadline = deadline
            };

            document_versions_[uri] = std::make_shared<std::atomic<int>>(version);
        }

        thread_pool_.Submit([this, uri, version]() -> void {
            UpdateWorker(uri, version, true);
        });
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
        auto deadline = std::chrono::steady_clock::now() + 200ms;
        auto uri      = NormalizeUri(origin_uri);

        {
            std::lock_guard lock(update_mutex_);
            pending_updates_[uri] = {
                .text     = new_text,
                .deadline = deadline
            };

            auto it = document_versions_.find(uri);
            if (it == document_versions_.end()) {
                document_versions_.try_emplace(uri, std::make_shared<std::atomic<int>>(version));

#ifdef _DEBUG
                GLSLD_LOG_INFO(
                    GLSLD_LOG_ROOT(),
                    "Document {} version initialized to {}, pending update scheduled.",
                    uri,
                    version
                );
#endif
            } else {
#ifdef _DEBUG
                GLSLD_LOG_INFO(
                    GLSLD_LOG_ROOT(),
                    "Document {} version updated from {} to {}, pending update scheduled.",
                    uri,
                    it->second->load(std::memory_order::relaxed),
                    version
                );
#endif
                it->second->store(version, std::memory_order::relaxed);
            }
        }

        thread_pool_.Submit([this, uri, version]() -> void {
            UpdateWorker(uri, version, false);
        });
    }

    void LspServer::HandleDidSave(Context& context) {
        const auto& document   = context.params["textDocument"];
        const auto& origin_uri = document["uri"];

        auto uri = NormalizeUri(origin_uri);
        workspace_.InvalidateInclude(uri);

        auto affected_uris = workspace_.GetAffectedDocuments(uri);
        for (const auto& affected_uri : affected_uris) {
            if (!document_versions_.contains(affected_uri)) {
                continue;
            }

            int new_version = 0;

            {
                std::shared_lock lock(update_mutex_);
                auto it = document_versions_.find(affected_uri);
                if (it == document_versions_.end()) {
                    continue;
                }

                int old_version = it->second->load(std::memory_order::relaxed);
                new_version = it->second->fetch_add(1, std::memory_order::relaxed) + 1;
#ifdef _DEBUG
                GLSLD_LOG_INFO(
                    GLSLD_LOG_ROOT(),
                    "Document {} affected by the change in {}, version updated from {} to {}",
                    affected_uri,
                    uri,
                    old_version,
                    new_version
                );
#endif
            }

            thread_pool_.Submit([this, affected_uri, new_version]() -> void {
                auto snapshot = workspace_.GetDocumentSnapshot(affected_uri);
                if (snapshot != nullptr) {
                    Update(affected_uri, snapshot->source, new_version, false);
                }
            });
        }
    }

    void LspServer::HandleDidClose(Context& context) {
        const auto& origin_uri = context.params["textDocument"]["uri"];
        auto uri = NormalizeUri(origin_uri);
        workspace_.RemoveDocument(uri);

        {
            std::lock_guard lock(update_mutex_);
            pending_updates_.erase(uri);

            auto it = document_versions_.find(uri);
            if (it != document_versions_.end()) {
                it->second->store(-1, std::memory_order::relaxed);
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
    }

    void LspServer::HandleInitialized(Context& context) {}

    void LspServer::HandleExit(Context& context) {
        running_.store(false);
    }

    void LspServer::UpdateWorker(const std::string& uri, int version_replica, bool open_document) {
        std::shared_lock lock(update_mutex_);
        auto& update = pending_updates_.at(uri);
        if (std::chrono::steady_clock::now() < update.deadline) {
            std::this_thread::sleep_until(update.deadline);
        }

        lock.unlock();

        {
            std::shared_lock lock(update_mutex_);
            auto it = document_versions_.find(uri);
            if (it == document_versions_.end()) {
                return;
            }

            if (version_replica != it->second->load(std::memory_order::relaxed)) {
                return;
            }
        }

        std::string text = std::move(update.text);
        {
            std::lock_guard lock(update_mutex_);
            pending_updates_.erase(uri);
        }

        Update(uri, text, version_replica, open_document);
    }

    void LspServer::Update(const std::string& uri, std::string_view text, int version_replica, bool open_document) {
        workspace_.UpdateDocument(uri, text, version_replica, document_versions_.at(uri), open_document);
        ready_condition_.notify_all();

        LspSubmitItem item{
            .payload = {
                { "uri", uri },
                { "version", version_replica },
                { "diagnostics", nlohmann::json::array() } // 发送空数组清空错误
            },
            .notify_method = "textDocument/publishDiagnostics",
            .kind          = LspSubmitItem::Kind::kNotification
        };

        EnqueueSubmit(std::move(item));
    }

    std::shared_ptr<const Document>
    LspServer::ValidateAndGetDocument(const Context& context, const std::string& uri) const
    {
        while (true) {
            if (context.cancelled()) {
                return nullptr;
            }

            int target_version = 0;
            {
                std::shared_lock lock(update_mutex_);
                auto it = document_versions_.find(uri);
                if (it != document_versions_.end()) {
                    target_version = it->second->load(std::memory_order::relaxed);
                }
            }

            auto snapshot = workspace_.GetDocumentSnapshot(uri);
            if (snapshot != nullptr && snapshot->version >= target_version) {
                return snapshot;
            }

            std::unique_lock lock(ready_mutex_);
            using namespace std::chrono_literals;
            ready_condition_.wait_for(lock, 25ms, [&context]() -> bool {
                return context.cancelled();
            });
        }
    }
}
