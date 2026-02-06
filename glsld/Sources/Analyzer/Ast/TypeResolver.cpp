#include "stdafx.h"
#include "TypeResolver.hpp"

#include <variant>

namespace glsld {
    TypeResolver::TypeResolver(const DocumentSymbols& symbols, BindingMap& bindings)
        : symbols_{ symbols }
        , bindings_{ bindings }
    {}

    void TypeResolver::VisitFunctionDeclaration(FunctionDeclarationNode* node) {
        if (node->declared_symbol != nullptr) {
            bindings_.emplace(node->declared_symbol->location, node->declared_symbol);

            const auto& return_typename_token = node->type_spec.typename_token();
            node->declared_symbol->type_info.typename_token = return_typename_token;

            if (return_typename_token.type == TokenType::kIdentifier) {
                auto* type_symbol = current_scope_->FindSymbol(return_typename_token.text);
                if (type_symbol != nullptr && (type_symbol->kind == SymbolKind::kInterface || type_symbol->kind == SymbolKind::kStruct)) {
                    bindings_.emplace(return_typename_token.location, type_symbol);
                }
            }
        }

        AstVisitor::VisitFunctionDeclaration(node);
    }

    void TypeResolver::VisitVariableDeclaration(VariableDeclarationNode* node) {
        if (node->declared_symbol != nullptr) {
            bindings_.emplace(node->declared_symbol->location, node->declared_symbol);

            const auto& typename_token = node->type_spec.typename_token();
            node->declared_symbol->type_info.typename_token = typename_token;

            if (typename_token.type == TokenType::kIdentifier) {
                auto* type_symbol = current_scope_->FindSymbol(typename_token.text);
                if (type_symbol != nullptr && (type_symbol->kind == SymbolKind::kInterface || type_symbol->kind == SymbolKind::kStruct)) {
                    bindings_.emplace(typename_token.location, type_symbol);
                    node->declared_symbol->type_info.block_symbol = type_symbol;
                }
            }
        }

        AstVisitor::VisitVariableDeclaration(node);
    }

    void TypeResolver::VisitInterfaceDeclaration(InterfaceDeclarationNode* node) {
        if (node->declared_symbol != nullptr) {
            bindings_.emplace(node->declared_symbol->location, node->declared_symbol);
        }

        AstVisitor::VisitInterfaceDeclaration(node);
    }

    void TypeResolver::VisitStructDeclaration(StructDeclarationNode* node) {
        if (node->declared_symbol != nullptr) {
            bindings_.emplace(node->declared_symbol->location, node->declared_symbol);
        }

        AstVisitor::VisitStructDeclaration(node);
    }

    void TypeResolver::VisitVariableExpression(VariableExpressionNode* node) {
        if (std::holds_alternative<const SymbolInfo*>(node->linked_symbols)) {
            if (const auto* symbol = std::get<const SymbolInfo*>(node->linked_symbols)) {
                node->evaluated_type = symbol->type_info;
            }
        }
    }

    void TypeResolver::VisitIndexExpression(IndexExpressionNode* node) {
        Traverse(node->base.get());
        node->evaluated_type = node->base->evaluated_type;
        node->evaluated_type.is_array = false;
    }

    void TypeResolver::VisitMemberAccessExpression(MemberAccessExpressionNode* node) {
        // object.member
        Traverse(node->object.get()); // 递归推导对象类型

        TypeInfo object_type = node->object->evaluated_type;
        const auto* struct_symbol = object_type.block_symbol;
        if (struct_symbol != nullptr && struct_symbol->internal_scope != nullptr) {
            auto* member_symbol = struct_symbol->internal_scope->FindSymbol(node->member->name);

            if (member_symbol != nullptr) {
                node->member->linked_symbols = member_symbol;
                node->evaluated_type = member_symbol->type_info;

                bindings_.emplace(node->member->begin, member_symbol);
            }
        }
    }
}
