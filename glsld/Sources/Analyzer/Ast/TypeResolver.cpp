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

            if (from.is_array() != to.is_array()) {
                return false;
            }

            if (from.block_symbol != nullptr || to.block_symbol != nullptr) {
                return false;
            }

            return false;
        }
    }

    TypeResolver::TypeResolver(const DocumentSymbols& symbols, BindingMap& bindings)
        : symbols_{ symbols }
        , bindings_{ bindings }
    {}

    void TypeResolver::VisitFunctionDeclaration(FunctionDeclarationNode* node) {
        if (node->declared_symbol == nullptr) {
            return;
        }

        auto* function_symbol = node->declared_symbol;
        bindings_.try_emplace(function_symbol->location, function_symbol);
        ExtractTypeInfo(function_symbol->type_info, node->type_spec);

        function_symbol->param_typeinfos.clear();
        for (auto& param_node : node->params) {
            VisitVariableDeclaration(param_node.get());

            TypeInfo param_typeinfo;
            if (param_node->declared_symbol != nullptr) {
                param_typeinfo = param_node->declared_symbol->type_info;
            } else {
                ExtractTypeInfo(param_typeinfo, param_node->type_spec);
            }

            function_symbol->param_typeinfos.push_back(param_typeinfo);
        }

        if (node->body != nullptr) {
            Traverse(node->body.get());
        }
    }

    void TypeResolver::VisitVariableDeclaration(VariableDeclarationNode* node) {
        if (node->declared_symbol == nullptr) {
            return;
        }

        auto* variable_symbol = node->declared_symbol;
        bindings_.try_emplace(variable_symbol->location, variable_symbol);
        ExtractTypeInfo(variable_symbol->type_info, node->type_spec);

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

    void TypeResolver::VisitCallExpression(CallExpressionNode* node) {
        std::vector<TypeInfo> call_arg_types;
        for (const auto& arg : node->args) {
            Traverse(arg.get());
            call_arg_types.push_back(arg->evaluated_type);
        }

        Traverse(node->callee.get()); // 如果是函数列表（未确定重载），不会进行任何操作

        auto* callee_node = static_cast<VariableExpressionNode*>(node->callee.get());
        if (std::holds_alternative<SymbolList>(callee_node->linked_symbols)) {
            const auto& candidates = std::get<SymbolList>(callee_node->linked_symbols);

            const auto* best_match = ResolveOverload(candidates, call_arg_types);
            if (best_match != nullptr) {
                callee_node->linked_symbols   = best_match;
                callee_node->evaluated_type   = best_match->type_info;
                bindings_[callee_node->begin] = best_match;
            }
        } else if (std::holds_alternative<const SymbolInfo*>(callee_node->linked_symbols)) {
            if (const auto* symbol = std::get<const SymbolInfo*>(callee_node->linked_symbols)) {
                callee_node->evaluated_type = symbol->type_info;
                node->evaluated_type = symbol->type_info;
            }
        }
    }

    void TypeResolver::VisitIndexExpression(IndexExpressionNode* node) {
        Traverse(node->base.get());
        node->evaluated_type = node->base->evaluated_type;
    }

    void TypeResolver::VisitVariableExpression(VariableExpressionNode* node) {
        if (std::holds_alternative<const SymbolInfo*>(node->linked_symbols)) {
            if (const auto* symbol = std::get<const SymbolInfo*>(node->linked_symbols)) {
                node->evaluated_type = symbol->type_info; // 根据指向的符号类型推导当前符号类型
            }
        }
    }

    void TypeResolver::VisitMemberAccessExpression(MemberAccessExpressionNode* node) {
        // object.member
        Traverse(node->object.get()); // 递归推导对象类型

        auto object_type = node->object->evaluated_type;
        const auto* block_symbol = object_type.block_symbol;
        if (block_symbol != nullptr  && block_symbol->internal_scope != nullptr) {
            const auto* member_symbol = block_symbol->internal_scope->FindSymbol(node->member->name);

            if (member_symbol != nullptr) {
                node->member->linked_symbols = member_symbol;
                node->evaluated_type = member_symbol->type_info;

                bindings_.try_emplace(node->member->begin, member_symbol);
            }
        }
    }

    void TypeResolver::ExtractTypeInfo(TypeInfo& target, const TypeSpecifier& type_spec) {
        const auto& typename_token = type_spec.typename_token();
        target.typename_token = typename_token;

        target.array_sizes.clear();
        for (const auto& size : type_spec.array_sizes) {
            if (size == nullptr) {
                continue;
            }

            if (size->kind() == AstNodeKind::kLiteralExpression) {
                const auto* raw_node = static_cast<const RawExpressionNode*>(size.get());
                for (const auto& token : raw_node->tokens) {
                    target.array_sizes.push_back(token);
                }
            } else {
                const auto* var_expr = static_cast<const VariableExpressionNode*>(size.get());
                target.array_sizes.push_back(var_expr->evaluated_type.typename_token);

                if (std::holds_alternative<const SymbolInfo*>(var_expr->linked_symbols)) {
                    const auto* size_symbol = std::get<const SymbolInfo*>(var_expr->linked_symbols);
                    bindings_.try_emplace(var_expr->begin, size_symbol);
                }
            }
        }

        if (typename_token.type == TokenType::kIdentifier) {
            auto* type_symbol = current_scope_->FindSymbol(typename_token.text);
            bool  is_block    = type_symbol->kind == SymbolKind::kInterface
                             || type_symbol->kind == SymbolKind::kStruct;

            if (type_symbol != nullptr && is_block) {
                target.block_symbol = type_symbol;
                bindings_.try_emplace(typename_token.location, type_symbol);
            }
        }
    }

    const SymbolInfo* TypeResolver::ResolveOverload(const SymbolList& candidates, std::span<const TypeInfo> call_arg_types) {
        const SymbolInfo* best_match = nullptr;
        int highest_score = -1;

        std::vector<TypeInfo> normalized_call_args(call_arg_types.begin(), call_arg_types.end());
        if (normalized_call_args.empty()) {
            normalized_call_args.push_back(TypeInfo{
                .typename_token = Token{
                    .text = "void",
                    .type = TokenType::kPrimitive
                }
            });
        }

        for (const auto* symbol : candidates) {
            const auto& param_typeinfos = symbol->param_typeinfos;
            if (param_typeinfos.size() != normalized_call_args.size()) {
                continue;
            }

            int current_score = 0;
            bool match_failed = false;

            for (auto i = 0uz; i != normalized_call_args.size(); ++i) {
                const auto& call_type   = normalized_call_args[i];
                const auto& target_type = param_typeinfos[i];

                if (call_type == target_type) {
                    current_score += 2;
                } else if (CanImplicityConvert(call_type, target_type)) {
                    current_score += 1;
                } else {
                    match_failed = true;
                    break;
                }
            }

            if (!match_failed) {
                if (symbol->kind == SymbolKind::kFunctionImpl) {
                    current_score += 1;
                }

                if (current_score > highest_score) {
                    highest_score = current_score;
                    best_match = symbol;
                }
            }
        }

        return best_match;
    }
}
