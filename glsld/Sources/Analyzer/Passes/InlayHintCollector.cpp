#include "pch.hpp"
#include "InlayHintCollector.hpp"

#include <algorithm>
#include <format>
#include <iterator>
#include <string>
#include <variant>

#include "Analyzer/Ast/Ast.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    InlayHintCollector::InlayHintCollector(const Document& document)
        : AstVisitor(0, nullptr)
    {
        Traverse(document.ast);
    }

    const std::vector<InlayHint>& InlayHintCollector::hints() const {
        return hints_;
    }

    void InlayHintCollector::VisitFunctionDeclaration(FunctionDeclarationNode* node) {
        if (node->declared_symbol == nullptr || node->body == nullptr) {
            return;
        }

        const auto func_name = Utils::UnmangleFunctionName(node->declared_symbol->name);

        hints_.push_back({
            .location = &node->body->end,
            .label    = std::format(" // {}", func_name)
        });

        AstVisitor::VisitFunctionDeclaration(node);
    }

    void InlayHintCollector::VisitInitializerListExpression(InitializerListExpressionNode* node) {
        for (auto i = 0uz; i != node->elements.size(); ++i) {
            if (node->elements[i] == nullptr) {
                continue;
            }

            hints_.push_back({
                .location = &node->elements[i]->begin,
                .label    = std::format("[{}]=", i)
            });
        }

        AstVisitor::VisitInitializerListExpression(node);
    }

    void InlayHintCollector::VisitCallExpression(CallExpressionNode* node) {
        const SymbolInfo* symbol = nullptr;

        if (node->callee == nullptr || node->callee->kind() != AstNodeKind::kVariableExpression) {
            AstVisitor::VisitCallExpression(node);
            return;
        }

        auto* callee_node = static_cast<const VariableExpressionNode*>(node->callee);
        if (std::holds_alternative<const SymbolInfo*>(callee_node->linked_symbols)) {
            symbol = std::get<const SymbolInfo*>(callee_node->linked_symbols);
        }

        if (symbol != nullptr && symbol->node != nullptr &&
            symbol->node->kind() == AstNodeKind::kFunctionDeclaration)
        {
            auto* func_decl = static_cast<const FunctionDeclarationNode*>(symbol->node);

            const auto num_params = func_decl->params.size(); // 函数原始参数
            const auto num_args   = node->args.size();        // 调用参数

            for (auto i = 0uz; i != std::min(num_params, num_args); ++i) {
                const auto& param = func_decl->params[i];
                const auto& arg   = node->args[i];

                if (arg == nullptr) {
                    continue;
                }

                if (param->declared_symbol != nullptr && !param->declared_symbol->name.empty()) {
                    const auto& type_info = param->declared_symbol->type_info;
                    std::string label = param->declared_symbol->name;

                    if (param->type_spec.has_keyword("out") || param->type_spec.has_keyword("inout")) {
                        label.insert(label.begin(), '&');
                    }

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
