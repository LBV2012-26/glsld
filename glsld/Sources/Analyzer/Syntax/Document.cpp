#include "pch.hpp"
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

    void Document::PrepareInjectedMacros(const SourceFile* source_file) {
        SourceLocation location(source_file, 0, 0);

        for (auto& [name, definition] : pending_macros_) {
            definition.original_token.location = location;

            auto it = macro_table.find(name);
            if (it != macro_table.end()) {
                it->second.original_token.location = location;
            }
        }
    }

    void Document::InjectMacro(std::string_view name, MacroDefinition definition) {
        definition.original_token.text = name;
        macro_table.insert_or_assign(name, definition);
        pending_macros_.insert_or_assign(name, std::move(definition));
    }

    void Document::InjectMacro(std::string_view name) {
        InjectMacro(name, MacroDefinition{
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
        SourceLocation location(source_file, 0, 0);

        for (const auto& [name, definition] : pending_macros_) {
            auto node = std::make_unique<PreprocessorNode>(root);

            node->directive = "define";
            node->begin     = location;
            node->end       = location;
            node->tokens    = definition.replacement_list;
            node->symbol    = symbols.AddMacroSymbol(node.get(), name, location);

            ast->preprocessor_references.push_back(node.get());
            injected_nodes_.push_back(std::move(node));
        }

        pending_macros_.clear();
    }
}
