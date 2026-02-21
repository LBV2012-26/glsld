#include "stdafx.h"
#include "MacroBinder.hpp"

#include <string_view>

namespace glsld {
    MacroBinder::MacroBinder(Document& document)
        : document_{ document }
    {}

    void MacroBinder::BindMacro() {
        BindMacroInvocations();
        BindMacroBodyIdentifiers();
    }

    void MacroBinder::BindMacroInvocations() {
        for (auto& token : document_.raw_tokens) {
            auto trace_it = document_.macro_traces.find(token.location);
            if (trace_it != document_.macro_traces.end()) {
                std::string_view macro_name = trace_it->second.text;
                if (const auto* macro_symbol = document_.symbols.root_scope()->FindSymbol(macro_name)) {
                    document_.bindings.insert_or_assign(token.location, macro_symbol);
                    token.type = TokenType::kIdentifier;
                }
            }
        }
    }

    void MacroBinder::BindMacroBodyIdentifiers() {
        const auto& preprocessor_references = document_.ast->preprocessor_references;
        for (const auto* node : preprocessor_references) {
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

                const auto* symbol = scope->FindSymbolForHighlighting(token.text);
                if (symbol != nullptr) {
                    document_.bindings.try_emplace(token.location, symbol);
                }
            }
        }
    }
}
