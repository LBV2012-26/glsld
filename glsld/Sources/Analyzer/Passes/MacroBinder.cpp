#include "stdafx.h"
#include "MacroBinder.hpp"

#include <string_view>
#include <utility>

namespace glsld {
    MacroBinder::MacroBinder(Document& document, int version_replica, std::shared_ptr<const std::atomic<int>> version_pointer)
        : document_{ document }
        , version_replica_{ version_replica }
        , version_pointer_{ version_pointer }
    {
        BindMacroInvocations();
        BindMacroBodyIdentifiers();
        BindMacroFunctionArguments();
        BindMacroAfterDirective();
    }

    void MacroBinder::BindMacroInvocations() {
        for (auto& token : document_.raw_tokens) {
            if (version_pointer_ != nullptr && version_replica_ != version_pointer_->load()) {
                return;
            }

            auto trace_it = document_.macro_traces.find(token.location);
            if (trace_it != document_.macro_traces.end()) {
                if (const auto* macro_symbol = document_.symbols.root_scope()->FindSymbol(trace_it->second.text)) {
                    document_.bindings.insert_or_assign(token.location, macro_symbol);
                    token.type = TokenType::kIdentifier;
                }
            }
        }
    }

    void MacroBinder::BindMacroBodyIdentifiers() {
        const auto& references = document_.ast->preprocessor_references;
        for (const auto* node : references) {
            if (version_pointer_ != nullptr && version_replica_ != version_pointer_->load()) {
                return;
            }

            if (node->directive != "define") {
                continue;
            }

            for (const auto& token : node->tokens) {
                if (node->symbol == nullptr) {
                    break;
                }

                const auto* scope = document_.symbols.FindScopeAt(node->symbol->location);
                if (scope == nullptr) {
                    continue;
                }

                const auto* symbol = scope->FindSymbol(token.text);
                if (symbol != nullptr) {
                    document_.bindings.try_emplace(token.location, symbol);
                } else {
                    auto symbol_list = document_.symbols.FindFunctionsByOriginalName(token.text);
                    if (!std::holds_alternative<std::monostate>(symbol_list)) {
                        document_.bindings.try_emplace(token.location, std::move(symbol_list));
                    }
                }
            }
        }
    }

    void MacroBinder::BindMacroFunctionArguments() {
        for (const auto& [location, token] : document_.macro_args_traces) {
            if (version_pointer_ != nullptr && version_replica_ != version_pointer_->load()) {
                return;
            }

            const auto* scope = document_.symbols.FindScopeAt(location);
            if (scope == nullptr) {
                continue;
            }

            const auto* symbol = scope->FindSymbol(token.text);
            if (symbol != nullptr) {
                document_.bindings.try_emplace(location, symbol);
            } else {
                auto symbol_list = document_.symbols.FindFunctionsByOriginalName(token.text);
                if (!std::holds_alternative<std::monostate>(symbol_list)) {
                    document_.bindings.try_emplace(location, std::move(symbol_list));
                }
            }
        }
    }

    void MacroBinder::BindMacroAfterDirective() {
        if (document_.ast == nullptr) {
            return;
        }

        const auto* root_scope = document_.symbols.root_scope();
        auto TryBindMacroIdentifier = [root_scope, this](const Token& token) -> void {
            if (token.type != TokenType::kIdentifier) {
                return;
            }

            const auto* symbol = root_scope->FindSymbol(token.text);
            if (symbol == nullptr || symbol->kind != SymbolKind::kMacro) {
                return;
            }

            document_.bindings.try_emplace(token.location, symbol);
        };

        const auto& references = document_.ast->preprocessor_references;
        for (const auto* node : references) {
            if (version_pointer_ != nullptr && version_replica_ != version_pointer_->load()) {
                return;
            }

            if (node == nullptr) {
                continue;
            }

            if (node->directive != "include" && node->directive != "if" && node->directive != "elif" &&
                node->directive != "ifdef"   && node->directive != "ifndef")
            {
                continue;
            }

            const auto& tokens = node->tokens;
            for (auto i = 0uz; i != tokens.size(); ++i) {
                const auto& token = tokens[i];

                if (token.text == "defined") {
                    auto j = i + 1;
                    if (j < tokens.size() && tokens[j].type == TokenType::kOpenParen) {
                        ++j;
                    }

                    if (j < tokens.size() && tokens[j].type == TokenType::kIdentifier) {
                        TryBindMacroIdentifier(tokens[j]);
                        ++j;
                    }

                    continue;
                }

                TryBindMacroIdentifier(token);
            }
        }
    }
}
