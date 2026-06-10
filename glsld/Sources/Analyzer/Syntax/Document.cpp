#include "stdafx.h"
#include "Document.hpp"

#include <utility>

namespace glsld {
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
