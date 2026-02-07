#include "stdafx.h"
#include "TypeResolver.hpp"

#include <variant>
#include <vector>

namespace glsld {
    namespace {
        bool CanImplicityConvert(const TypeInfo& from, const TypeInfo& to) {
            if (from.typename_token.text == from.typename_token.text) {
                return true;
            }

            if (from.is_array != to.is_array) {
                return false;
            }

            if (from.block_symbol != nullptr || to.block_symbol != nullptr) {
                return false;
            }
        }
    }

    TypeResolver::TypeResolver(const DocumentSymbols& symbols, BindingMap& bindings)
        : symbols_{ symbols }
        , bindings_{ bindings }
    {}

    void TypeResolver::VisitFunctionDeclaration(FunctionDeclarationNode* node) {
        if (node->declared_symbol != nullptr) {
            bindings_.try_emplace(node->declared_symbol->location, node->declared_symbol);

            const auto& return_typename_token = node->type_spec.typename_token();
            node->declared_symbol->type_info.typename_token = return_typename_token;

            if (return_typename_token.type == TokenType::kIdentifier) {
                auto* type_symbol = current_scope_->FindSymbol(return_typename_token.text);
                if (type_symbol != nullptr && (type_symbol->kind == SymbolKind::kInterface || type_symbol->kind == SymbolKind::kStruct)) {
                    bindings_.try_emplace(return_typename_token.location, type_symbol);
                }
            }
        }

        AstVisitor::VisitFunctionDeclaration(node);
    }

    void TypeResolver::VisitVariableDeclaration(VariableDeclarationNode* node) {
        if (node->declared_symbol != nullptr) {
            bindings_.try_emplace(node->declared_symbol->location, node->declared_symbol);

            const auto& typename_token = node->type_spec.typename_token();
            node->declared_symbol->type_info.typename_token = typename_token;

            if (typename_token.type == TokenType::kIdentifier) {
                auto* type_symbol = current_scope_->FindSymbol(typename_token.text);
                if (type_symbol != nullptr && (type_symbol->kind == SymbolKind::kInterface || type_symbol->kind == SymbolKind::kStruct)) {
                    bindings_.try_emplace(typename_token.location, type_symbol);
                    node->declared_symbol->type_info.block_symbol = type_symbol;
                }
            }
        }

        AstVisitor::VisitVariableDeclaration(node);
    }

    void TypeResolver::VisitInterfaceDeclaration(InterfaceDeclarationNode* node) {
        if (node->declared_symbol != nullptr) {
            bindings_.try_emplace(node->declared_symbol->location, node->declared_symbol);
        }

        AstVisitor::VisitInterfaceDeclaration(node);
    }

    void TypeResolver::VisitStructDeclaration(StructDeclarationNode* node) {
        if (node->declared_symbol != nullptr) {
            bindings_.try_emplace(node->declared_symbol->location, node->declared_symbol);
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

    void TypeResolver::VisitCallExpression(CallExpressionNode* node) {
        Traverse(node->callee.get());

        std::vector<std::string> arg_types;
        for (const auto& arg : node->args) {
            Traverse(arg.get());
            arg_types.push_back(arg->evaluated_type.typename_token.text);
        }

        if (node->callee->kind() == AstNodeKind::kVariableExpr) {
            auto* callee_expr = static_cast<VariableExpressionNode*>(node->callee.get());
            node->evaluated_type = ResolveOverload(callee_expr, arg_types);
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

                bindings_.try_emplace(node->member->begin, member_symbol);
            }
        }
    }

    TypeInfo TypeResolver::ResolveOverload(VariableExpressionNode* callee, std::span<const std::string> arg_types) {
        const auto& linked_symbols = callee->linked_symbols;
        if (!std::holds_alternative<std::vector<const SymbolInfo*>>(linked_symbols)) {
            return {};
        }

        const auto& candidates = std::get<std::vector<const SymbolInfo*>>(linked_symbols);

        std::string target_signature;
        for (auto i = 0uz; i != arg_types.size(); ++i) {
            target_signature += arg_types[i] + (i == arg_types.size() - 1 ? "" : ", ");
        }

        const SymbolInfo* best_match = nullptr;
        for (const auto* symbol : candidates) {
            if (symbol->name.find("(" + target_signature + ")") != std::string::npos) {
                if (best_match == nullptr || symbol->kind == SymbolKind::kFunctionImpl) {
                    best_match = symbol;
                }
            }
        }

        if (best_match != nullptr) {
            callee->linked_symbols = best_match;
            bindings_[callee->begin] = best_match;
            return best_match->type_info;
        }

        return {};
    }

    const SymbolInfo* TypeResolver::ResolveOverload(std::span<const SymbolInfo*> candidates, std::span<const TypeInfo> arg_types) {
        //const SymbolInfo* best_match = nullptr;
        //int highest_score = -1;

        //for (const auto* symbol : candidates) {
        //    std::span<const Token> param_typename_tokens = symbol->param_typename_tokens;
        //    if (param_typename_tokens.empty()) {
        //        continue;
        //    }

        //    int current_score = 0;
        //    bool match_failed = false;
        //    for (auto i = 0uz; i != arg_types.size(); ++i) {
        //        if (arg_types[i].typename_token.text == param_typename_tokens[i].text) {
        //            current_score += 2;
        //        } else if (CanImplicityConvert(arg_types[i], param_typename_tokens[i]))
        //    }
        //}

        return nullptr;
    }
}
