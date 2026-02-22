#include "stdafx.h"
#include "InlayHintVisitor.hpp"

#include <algorithm>
#include <variant>
#include "Analyzer/Ast/Ast.hpp"

namespace glsld {
    InlayHintVisitor::InlayHintVisitor()
        : AstVisitor(0, nullptr)
    {}

    void InlayHintVisitor::VisitCallExpression(CallExpressionNode* node) {
        const SymbolInfo* symbol = nullptr;
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

                if (param->declared_symbol != nullptr && !param->declared_symbol->name.empty()) {
                    const auto& type_info = param->declared_symbol->type_info;
                    std::string label = param->declared_symbol->name;
                    if (!type_info.array_sizes.empty()) {
                        for (const auto& array_size : type_info.array_sizes) {
                            label += "[" + array_size.text + "]";
                        }
                    }

                    hints_.push_back({
                        .location = arg->begin,
                        .label    = label + ":"
                    });
                }
            }
        }

        AstVisitor::VisitCallExpression(node);
    }

    const std::vector<InlayHint>& InlayHintVisitor::hints() const {
        return hints_;
    }
}
