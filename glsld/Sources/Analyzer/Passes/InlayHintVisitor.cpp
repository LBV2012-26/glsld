#include "pch.hpp"
#include "InlayHintVisitor.hpp"

#include <algorithm>
#include <format>
#include <iterator>
#include <string>
#include <variant>
#include "Analyzer/Ast/Ast.hpp"

namespace glsld {
    InlayHintVisitor::InlayHintVisitor(const Document& document)
        : AstVisitor(0, nullptr)
    {
        Traverse(document.ast.get());
    }

    const std::vector<InlayHint>& InlayHintVisitor::hints() const {
        return hints_;
    }

    void InlayHintVisitor::VisitInitializerListExpression(InitializerListExpressionNode* node) {
        for (auto i = 0uz; i != node->elements.size(); ++i) {
            if (node->elements[i] == nullptr) {
                continue;
            }

            hints_.push_back({
                .location = &node->elements[i]->begin,
                .label = std::format("[{}]=", i)
            });
        }

        AstVisitor::VisitInitializerListExpression(node);
    }

    void InlayHintVisitor::VisitCallExpression(CallExpressionNode* node) {
        const SymbolInfo* symbol = nullptr;

        if (node->callee == nullptr || node->callee->kind() != AstNodeKind::kVariableExpression) {
            AstVisitor::VisitCallExpression(node);
            return;
        }

        const auto* callee_node = static_cast<const VariableExpressionNode*>(node->callee.get());
        if (std::holds_alternative<const SymbolInfo*>(callee_node->linked_symbols)) {
            symbol = std::get<const SymbolInfo*>(callee_node->linked_symbols);
        }

        if (symbol != nullptr && symbol->node != nullptr &&
            symbol->node->kind() == AstNodeKind::kFunctionDeclaration)
        {
            const auto* func_decl = static_cast<const FunctionDeclarationNode*>(symbol->node);

            auto num_params = func_decl->params.size(); // 函数原始参数
            auto num_args   = node->args.size();        // 调用参数

            for (auto i = 0uz; i != std::min(num_params, num_args); ++i) {
                const auto& param = func_decl->params[i];
                const auto& arg   = node->args[i];

                if (arg == nullptr) {
                    continue;
                }

                if (param->declared_symbol != nullptr && !param->declared_symbol->name.empty()) {
                    const auto& type_info = param->declared_symbol->type_info;
                    std::string label = param->declared_symbol->name;
                    if (!type_info.array_sizes.empty()) {
                        for (auto array_size : type_info.array_sizes) {
                            if (array_size.has_value()) {
                                std::format_to(std::back_inserter(label), "[{}]", *array_size);
                            } else {
                                label += "[]";
                            }
                        }
                    }

                    hints_.push_back({
                        .location = &arg->begin,
                        .label    = label + ":"
                    });
                }
            }
        }

        AstVisitor::VisitCallExpression(node);
    }
}
