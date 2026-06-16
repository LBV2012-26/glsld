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
        , macros{ std::move(other.macros) }
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
            macros            = std::move(other.macros);
            version           = std::exchange(other.version, 0);
        }

        return *this;
    }

    void Document::InjectMacro(std::string_view name, MacroDefination defination) {
        macros.insert_or_assign(name, std::move(defination));
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
}
