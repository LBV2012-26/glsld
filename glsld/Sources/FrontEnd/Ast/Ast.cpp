#include "pch.hpp"
#include "Ast.hpp"
#include <algorithm>

namespace glsld {
    TypeSpec::TypeSpec(Arena* arena)
        : arena{ arena }
    {}

    bool TypeSpec::has_keyword(std::string_view name) const {
        return std::ranges::any_of(specifiers, [name](const auto& token) -> bool {
            return token.text == name;
        });
    }

    FunctionTypeSpec::FunctionTypeSpec(Arena* arena)
        : return_type{ arena }
        , param_types{ ArenaAllocator<TypeSpec>(arena) }
    {}

    AstNode::AstNode(Arena* arena, Scope* scope)
        : arena{ arena }
        , located_scope{ scope }
    {}

    const Token* NamespaceDeclarationNode::name() const {
        if (names.empty()) {
            return nullptr;
        }

        return &names.back();
    }
}
