#include "pch.hpp"
#include "InlayHintCollector.hpp"

#include <algorithm>
#include <format>
#include <iterator>
#include <optional>
#include <string>
#include <variant>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    InlayHintCollector::InlayHintCollector(const Document& document)
        : AstVisitor(0, nullptr)
    {
        Traverse(document.ast);

        std::erase_if(hints_, [&](const InlayHint& hint) -> bool {
            return document.macro_expansions.contains(*hint.location);
        });

        CollectMacroArgumentHints(document);
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
        std::optional<SymbolList> fields;

        if (!node->evaluated_type.is_array() && node->evaluated_type.block_symbol != nullptr) {
            fields = Utils::CollectStructFieldsOrdered(node->evaluated_type.block_symbol);
        }

        for (auto i = 0uz; i != node->elements.size(); ++i) {
            if (node->elements[i] == nullptr) {
                continue;
            }

            std::string label;
            if (fields.has_value() && i < fields->size()) {
                label = std::format(".{}=", (*fields)[i]->name);
            } else {
                label = std::format("[{}]=", i);
            }

            hints_.push_back({
                .location = &node->elements[i]->begin,
                .label    = std::move(label)
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
                const auto& argv  = node->args[i];

                if (argv == nullptr) {
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
                        .location = &argv->begin,
                        .label    = label + ":"
                    });
                }
            }
        } else if (symbol != nullptr && symbol->kind == SymbolKind::kStruct) {
            const auto fields = Utils::CollectStructFieldsOrdered(symbol);

            for (auto i = 0uz; i != std::min(fields->size(), node->args.size()); ++i) {
                const auto* field = (*fields)[i];
                const auto* argv  = node->args[i];

                if (field == nullptr || argv == nullptr || field->name.empty()) {
                    continue;
                }

                hints_.push_back({
                    .location = &argv->begin,
                    .label    = field->name + ":"
                });
            }
        }

        AstVisitor::VisitCallExpression(node);
    }

    void InlayHintCollector::CollectMacroArgumentHints(const Document& document) {
        const auto& tokens = document.raw_tokens;

        for (auto i = 0uz; i < tokens.size(); ++i) {
            const Token* definition_token = nullptr;

            if (const auto trace = document.macro_traces.find(tokens[i].location);
                trace != document.macro_traces.end())
            {
                definition_token = &trace->second;
            }
            else if (const auto arg_trace = document.macro_args_traces.find(tokens[i].location);
                     arg_trace != document.macro_args_traces.end() && arg_trace->second.definition.has_value())
            {
                definition_token = &*arg_trace->second.definition;
            }

            if (definition_token == nullptr) {
                continue;
            }

            const auto* symbol = document.symbols.FindMacroSymbol(*definition_token);
            if (symbol == nullptr || symbol->node == nullptr ||
                symbol->node->kind() != AstNodeKind::kPreprocessor)
            {
                continue;
            }

            const auto* definition = static_cast<const PreprocessorNode*>(symbol->node);
            if (definition->params.empty() ||
                i + 1 >= tokens.size() ||
                tokens[i + 1].type != TokenType::kOpenParen)
            {
                continue;
            }

            auto param_index    = 0uz;
            auto paren_level    = 0uz;
            auto bracket_level  = 0uz;
            auto brace_level    = 0uz;
            bool argument_begin = true;

            for (auto j = i + 2; j < tokens.size(); ++j) {
                const auto& token = tokens[j];

                if (token.type == TokenType::kCloseParen &&
                    paren_level == 0 &&
                    bracket_level == 0 &&
                    brace_level == 0)
                {
                    break;
                }

                if (token.type == TokenType::kComma &&
                    paren_level == 0 &&
                    bracket_level == 0 &&
                    brace_level == 0)
                {
                    ++param_index;
                    argument_begin = true;
                    continue;
                }

                if (argument_begin) {
                    if (param_index < definition->params.size()) {
                        hints_.push_back({
                            .location = &token.location,
                            .label    = std::string(definition->params[param_index]) + ":"
                        });
                    }

                    argument_begin = false;
                }

                switch (token.type) {
                case TokenType::kOpenParen:
                    ++paren_level;
                    break;
                case TokenType::kCloseParen:
                    --paren_level;
                    break;
                case TokenType::kOpenBracket:
                    ++bracket_level;
                    break;
                case TokenType::kCloseBracket:
                    --bracket_level;
                    break;
                case TokenType::kOpenBrace:
                    ++brace_level;
                    break;
                case TokenType::kCloseBrace:
                    --brace_level;
                    break;
                default:
                    break;
                }
            }
        }
    }
}
