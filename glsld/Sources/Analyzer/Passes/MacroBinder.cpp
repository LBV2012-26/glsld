#include "pch.hpp"
#include "MacroBinder.hpp"

#include <algorithm>
#include <string_view>
#include <utility>

namespace glsld {
    MacroBinder::MacroBinder(Document& document, int version_replica, VersionPointer version_pointer)
        : document_{ document }
        , version_replica_{ version_replica }
        , version_pointer_{ version_pointer }
    {
        BindMacroInvocations();
        BindMacroFunctionArguments();
        BindPreprocessorIdentifiers();
    }

    void MacroBinder::BindMacroInvocations() {
        for (auto& token : document_.raw_tokens) {
            if (version_pointer_ != nullptr && version_replica_ != version_pointer_->load(std::memory_order::relaxed)) {
                return;
            }

            auto trace_it = document_.macro_traces.find(token.location);
            if (trace_it == document_.macro_traces.end()) {
                continue;
            }

            auto* macro_symbol = document_.symbols.FindMacroSymbol(trace_it->second);
            if (macro_symbol == nullptr) {
                continue;
            }

            document_.bindings.insert_or_assign(token.location, macro_symbol);
            token.type = TokenType::kIdentifier;
        }
    }

    void MacroBinder::BindMacroFunctionArguments() {
        for (const auto& [location, trace] : document_.macro_args_traces) {
            if (version_pointer_ != nullptr && version_replica_ != version_pointer_->load(std::memory_order::relaxed)) {
                return;
            }

            if (trace.definition.has_value()) {
                auto* macro_symbol = document_.symbols.FindMacroSymbol(*trace.definition);
                if (macro_symbol != nullptr) {
                    document_.bindings.try_emplace(location, macro_symbol);
                }
                continue;
            }

            auto* scope = document_.symbols.FindScopeAt(location);
            if (scope == nullptr) {
                continue;
            }

            auto* symbol = scope->FindSymbol(trace.token.text);
            if (symbol != nullptr) {
                document_.bindings.try_emplace(location, symbol);
                continue;
            }

            auto functions = document_.symbols.FindFunctionsByOriginalName(trace.token.text);
            if (!std::holds_alternative<std::monostate>(functions)) {
                document_.bindings.try_emplace(location, ReferenceSymbol(functions));
            }
        }
    }

    void MacroBinder::BindPreprocessorIdentifiers() {
        if (document_.ast == nullptr) {
            return;
        }

        StringHeteroHashMap<const SymbolInfo*> active_macros;
        const auto& references = document_.ast->preprocessor_references;

        for (const auto* node : references) {
            if (node != nullptr && node->directive == "define" &&
                node->symbol != nullptr && node->begin.line() == 0)
            {
                active_macros.insert_or_assign(node->symbol->name, node->symbol);
            }
        }

        auto BindActiveMacro = [this, &active_macros](const Token& token) -> bool {
            if (token.type != TokenType::kIdentifier) {
                return false;
            }

            auto it = active_macros.find(token.text);
            if (it == active_macros.end()) {
                return false;
            }

            document_.bindings.try_emplace(token.location, it->second);
            return true;
        };

        auto BindRegularSymbol = [this](const Token& token, const Scope* scope) -> void {
            if (token.type != TokenType::kIdentifier || scope == nullptr) {
                return;
            }

            auto* symbol = scope->FindSymbol(token.text);
            if (symbol != nullptr) {
                document_.bindings.try_emplace(token.location, symbol);
                return;
            }

            auto functions = document_.symbols.FindFunctionsByOriginalName(token.text);
            if (!std::holds_alternative<std::monostate>(functions)) {
                document_.bindings.try_emplace(token.location, ReferenceSymbol(functions));
            }
        };

        for (const auto* node : references) {
            if (version_pointer_ != nullptr && version_replica_ != version_pointer_->load(std::memory_order::relaxed)) {
                return;
            }

            if (node == nullptr || node->begin.line() == 0) {
                continue;
            }

            if (node->directive == "define") {
                if (node->symbol == nullptr) {
                    continue;
                }

                active_macros.insert_or_assign(node->symbol->name, node->symbol);
                auto* scope = document_.symbols.FindScopeAt(node->symbol->location);

                for (const auto& token : node->tokens) {
                    if (std::ranges::contains(node->params, token.text)) {
                        continue;
                    }

                    if (!BindActiveMacro(token)) {
                        BindRegularSymbol(token, scope);
                    }
                }

                continue;
            }

            if (node->directive == "undef") {
                if (node->tokens.empty() || node->tokens.front().type != TokenType::kIdentifier) {
                    continue;
                }

                BindActiveMacro(node->tokens.front());
                active_macros.erase(node->tokens.front().text);
                continue;
            }

            if (node->directive != "include" &&
                node->directive != "if"      &&
                node->directive != "elif"    &&
                node->directive != "ifdef"   &&
                node->directive != "ifndef")
            {
                continue;
            }

            for (const auto& token : node->tokens) {
                BindActiveMacro(token);
            }
        }
    }

    SymbolReferenceView MacroBinder::ReferenceSymbol(const SymbolReference& reference) {
        if (std::holds_alternative<std::monostate>(reference)) {
            return std::monostate{};
        }

        if (std::holds_alternative<const SymbolInfo*>(reference)) {
            return std::get<const SymbolInfo*>(reference);
        }

        const auto& symbols = std::get<SymbolList>(reference);
        return document_.arena.CopySpan<const SymbolInfo*>(symbols);
    }
}
