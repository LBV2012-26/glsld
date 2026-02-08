#include "stdafx.h"
#include "OverloadResolver.hpp"

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

    OverloadResolver::OverloadResolver(const DocumentSymbols& symbols, BindingMap& bindings)
        : symbols_{ symbols }
        , bindings_{ bindings }
    {}

    void OverloadResolver::VisitCallExpression(CallExpressionNode* node) {
        std::vector<TypeInfo> call_arg_types;
        for (const auto& arg : node->args) {
            Traverse(arg.get());
            call_arg_types.push_back(arg->evaluated_type); // 处理参数类型
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
            } // 只有一个符号并且已经推导，直接过
        }
    }

    const SymbolInfo* OverloadResolver::ResolveOverload(const SymbolList& candidates, std::span<const TypeInfo> call_arg_types) {
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
