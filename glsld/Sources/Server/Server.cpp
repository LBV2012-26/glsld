#include "stdafx.h"
#include "Server.hpp"

#include <cstdio>
#include <charconv>
#include <exception>
#include <iostream>
#include <print>
#include <span>
#include <unordered_set>
#include <utility>

#include "Analyzer/Parser.hpp"
#include "Base/Logger.hpp"
#include "Server/JsonResponse.hpp"

namespace glsld {
    namespace {
        nlohmann::json ConvertScopeToDocumentSymbols(const Scope* const scope) {
            nlohmann::json symbols = nlohmann::json::array();

            if (scope == nullptr) {
                return symbols;
            }

            auto ConvertToLspRange = [](const auto& begin, const auto& end) -> nlohmann::json {
                return {
                    { "start", { { "line", begin.line - 1 }, { "character", begin.column - 1 } } },
                    { "end",   { { "line", end.line   - 1 }, { "character", end.column   - 1 } } }
                };
            };

            auto ConvertToSelectionRange = [](const auto& location, std::string_view name) -> nlohmann::json {
                return {
                    { "start", { { "line", location.line - 1 }, { "character", location.column - 1 } } },
                    { "end",   { { "line", location.line - 1 }, { "character", location.column + name.length() - 1 } } }
                };
            };

            // split __Decl_ or __Impl_ on function
            auto SplitSymbolName = [](std::string_view mangled_name) -> std::string_view {
                return mangled_name.starts_with("__Decl_") ? mangled_name.substr(7) :
                       mangled_name.starts_with("__Impl_") ? mangled_name.substr(7) :
                       mangled_name; // common token
            };

            std::unordered_set<const Scope*> handled_scopes;

            for (const auto& [name, info] : scope->symbols()) {
                nlohmann::json symbol_node;
                symbol_node["name"] = SplitSymbolName(info.name);
                symbol_node["kind"] = ConvertSymbolKind(info.kind);
                symbol_node["selectionRange"] = ConvertToSelectionRange(info.location, info.name);

                const Scope* child_scope = nullptr;
                if (info.kind == SymbolKind::kInterface || info.kind == SymbolKind::kStruct) {
                    for (const auto& child : scope->children()) {
                        if (child->interval().first.line == info.location.line) {
                            child_scope = child.get();
                            handled_scopes.emplace(child_scope);
                            break;
                        }
                    }

                    if (info.kind == SymbolKind::kFunctionDecl) {
                        symbol_node["detail"] = "(decl)";
                    }
                }

                if (child_scope != nullptr) {
                    symbol_node["range"] = ConvertToLspRange(info.location, child_scope->interval().second);
                    if (info.kind == SymbolKind::kInterface || info.kind == SymbolKind::kStruct) {
                        auto children = ConvertScopeToDocumentSymbols(child_scope);
                        if (!children.empty()) {
                            symbol_node["children"] = children;
                        }
                    }
                } else {
                    symbol_node["range"] = symbol_node["selectionRange"];
                }

                symbols.push_back(symbol_node);
            }

            // Transparent scope
            for (const auto& child_scope : scope->children()) {
                if (child_scope->kind() == ScopeKind::kTransparent && !handled_scopes.contains(child_scope.get())) {
                    auto transparent_children = ConvertScopeToDocumentSymbols(child_scope.get());
                    for (const auto& child : transparent_children) {
                        symbols.push_back(child);
                    }
                }
            }

            return symbols;
        }

        std::vector<std::uint32_t> SemanticData(const DocumentSymbols& symbols, std::span<const Token> tokens) {
            std::vector<std::uint32_t> data;
            std::uint32_t last_line = 0;
            std::uint32_t last_char = 0;
            std::uint32_t modifiers = 0;

            for (const auto& token : tokens) {
                int type_index = -1;

                switch (token.type) {
                case TokenType::kIdentifier: {
                    auto* scope  = symbols.FindScopeAt(token.location);
                    auto* symbol = scope->FindSymbolForHighlighting(token.text);

                    if (symbol != nullptr) {
                        switch (symbol->kind) {
                        case SymbolKind::kInterface:
                            type_index = 4;
                            break;
                        case SymbolKind::kStruct:
                            type_index = 5;
                            break;
                        case SymbolKind::kParameter:
                            type_index = 7;
                            break;
                        case SymbolKind::kVariable:
                            type_index = 8;
                            break;
                        case SymbolKind::kFunctionDecl:
                        case SymbolKind::kFunctionImpl:
                            type_index = 12;
                            break;
                        case SymbolKind::kMacro:
                            type_index = 14;
                            break;
                        case SymbolKind::kPreprocessor:
                            type_index = 15;
                            break;
                        }

                        if (token.location.line   == symbol->location.line &&
                            token.location.column == symbol->location.column)
                        {
                            modifiers |= (1 << 0); // declaration
                        }

                        if (token.type == TokenType::kIdentifier && scope->kind() == ScopeKind::kTransparent) {
                            modifiers |= (1 << 3); // static
                        }
                    }

                    break;
                }
                case TokenType::kPrimitive:
                    type_index = 23;
                    break;
                case TokenType::kBuiltInType:
                    type_index = 1;
                    break;
                case TokenType::kKeyword:
                case TokenType::kPreprocessor:
                case TokenType::kSharp:
                    type_index = 15;
                    break;
                case TokenType::kNumberLiteral:
                    type_index = 19;
                    break;
                }

                if (type_index != -1) {
                    std::size_t line       = token.location.line   - 1;
                    std::size_t character  = token.location.column - 1;
                    std::size_t length     = token.text.length();

                    std::size_t delta_line = line - last_line;
                    std::size_t delta_char = (delta_line == 0) ? (character - last_char) : character;

                    data.push_back(static_cast<std::uint32_t>(delta_line));
                    data.push_back(static_cast<std::uint32_t>(delta_char));
                    data.push_back(static_cast<std::uint32_t>(length));
                    data.push_back(static_cast<std::uint32_t>(type_index));
                    data.push_back(modifiers);

                    last_line = static_cast<std::uint32_t>(line);
                    last_char = static_cast<std::uint32_t>(character);
                }
            }

            return data;
        }
    }

    LspServer::LspServer() {
        RegisterHandlers();
    }

    void LspServer::Run() {
        while (running_ && std::cin.good()) {
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

        capabilities["textDocumentSync"] = 1;
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

        // 3. 支持跳转定义 (未来实现)
        // capabilities["definitionProvider"] = true;

        // 4. 支持补全 (未来实现)
        // capabilities["completionProvider"] = {
        //     {"resolveProvider", false},
        //     {"triggerCharacters", {".", "#"}}
        // };

        return {
            { "capabilities", capabilities },
            { "serverInfo", {
                { "name", "glsl-analyzer" },
                { "version", "0.1.0" }
            } }
        };
    }

    nlohmann::json LspServer::HandleShutdown(Context& context) {
        return nullptr;
    }

    nlohmann::json LspServer::HandleDocumentSymbol(Context& context) {
        std::string uri = context.params["textDocument"]["uri"];

        auto it = documents_.find(uri);
        if (it == documents_.end()) {
            return nlohmann::json::array();
        }

        auto& symbols = it->second;
        return ConvertScopeToDocumentSymbols(symbols.root_scope());
    }

    nlohmann::json LspServer::HandleSemanticTokens(Context& context) {
        std::string uri = context.params["textDocument"]["uri"];

        auto symbol_it = documents_.find(uri);
        if (symbol_it == documents_.end()) {
            return nlohmann::json::array();
        }

        const auto& symbols = symbol_it->second;

        auto token_it = tokens_.find(uri);
        if (token_it == tokens_.end()) {
            return nlohmann::json::array();
        }

        const auto& tokens = token_it->second;

        auto data = SemanticData(symbols, tokens);

        return { { "data", data } };
    }

    void LspServer::HandleDidOpen(Context& context) {
        const auto& document  = context.params["textDocument"];
        std::string uri       = document["uri"];
        std::string_view text = document["text"];

        DocumentSymbols symbols;
        Parser parser(text, symbols);
        parser.Parse();

        documents_[uri] = std::move(symbols);
        tokens_[uri]    = parser.tokens();
    }

    void LspServer::HandleDidChange(Context& context) {
        std::string uri = context.params["textDocument"]["uri"];

        const auto& changes = context.params["contentChanges"];
        if (changes.empty() || !changes[0].contains("text")) {
            return;
        }
        std::string new_text = changes[0]["text"];
        // TODO
    }

    void LspServer::HandleDidClose(Context& context) {
        std::string_view uri = context.params["textDocument"]["uri"];
        documents_.erase(uri);

        SendNotification("textDocument/publishDiagnostics", {
            { "uri", uri },
            { "diagnostics", nlohmann::json::array() } // 发送空数组清空错误
        });
    }

    void LspServer::HandleInitialized(Context& context) {}

    void LspServer::HandleExit(Context& context) {
        running_ = false;
    }
}
