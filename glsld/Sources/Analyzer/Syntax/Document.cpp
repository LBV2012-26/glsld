#include "stdafx.h"
#include "Document.hpp"

#include <utility>

namespace glsld {
    Document::Document(Document&& other) noexcept
        : dependencies{ std::move(other.dependencies) }
        , builtins{ std::move(other.builtins) }
        , symbols{ std::move(other.symbols) }
        , source{ std::move(other.source) }
        , raw_tokens{ std::move(other.raw_tokens) }
        , expanded_tokens{ std::move(other.expanded_tokens) }
        , inactive_regions{ std::move(other.inactive_regions) }
        , ast{ std::move(other.ast) }
        , arena{ std::move(other.arena) }
        , bindings{ std::move(other.bindings) }
        , macro_traces{ std::move(other.macro_traces) }
        , macro_args_traces{ std::move(other.macro_args_traces) }
        , macro_expansions{ std::move(other.macro_expansions) }
        , macro_table{ std::move(other.macro_table) }
        , version{ std::exchange(other.version, 0) }
    {}

    Document& Document::operator=(Document && other) noexcept {
        if (this != &other) {
            dependencies      = std::move(other.dependencies);
            builtins          = std::move(other.builtins);
            symbols           = std::move(other.symbols);
            source            = std::move(other.source);
            raw_tokens        = std::move(other.raw_tokens);
            expanded_tokens   = std::move(other.expanded_tokens);
            inactive_regions  = std::move(other.inactive_regions);
            ast               = std::move(other.ast);
            arena             = std::move(other.arena);
            bindings          = std::move(other.bindings);
            macro_traces      = std::move(other.macro_traces);
            macro_args_traces = std::move(other.macro_args_traces);
            macro_expansions  = std::move(other.macro_expansions);
            macro_table       = std::move(other.macro_table);
            version           = std::exchange(other.version, 0);
        }

        return *this;
    }

    void Document::InjectMacro(std::string_view name, MacroDefination defination) {
        macro_table.insert_or_assign(name, defination);
        pending_macros_.push_back(std::move(defination));
    }

    void Document::InjectMacro(std::string_view name) {
        InjectMacro(name, MacroDefination{
            .is_function = false,
            .original_token = Token{
                .text = std::string(name),
                .type = TokenType::kIdentifier
            },
            .replacement_list = { Token{
                .text = "1",
                .type = TokenType::kNumberLiteral
            } }
        });
    }

    void Document::FinalizeInjectedMacros(const SourceFile* source_file) {
        auto* root = symbols.root_scope();

        for (const auto& pending : pending_macros_) {
            const auto& macro_name = pending.original_token.text;
            if (root->FindSymbolInCurrentScope(macro_name) != nullptr) {
                continue;
            }

            auto location = SourceLocation(source_file, 0, 0);

            auto node = std::make_unique<PreprocessorNode>(root);
            node->directive = "define";
            node->begin     = location;
            node->end       = location;
            node->tokens    = pending.replacement_list;

            auto* symbol = root->AddSymbol(node.get(), macro_name, location, SymbolKind::kMacro);
            symbol->node = node.get();
            node->symbol = symbol;

            ast->preprocessor_references.push_back(node.get());
            injected_nodes_.push_back(std::move(node));
        }

        pending_macros_.clear();
    }
}
