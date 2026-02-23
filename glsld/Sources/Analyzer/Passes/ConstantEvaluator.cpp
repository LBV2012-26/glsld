#include "stdafx.h"
#include "ConstantEvaluator.hpp"

#include <charconv>
#include <variant>
#include "Analyzer/Syntax/Token.hpp"

namespace glsld {
    ConstantEvaluator::ConstantEvaluator()
        : AstVisitor(0, nullptr)
    {}

    std::optional<std::int64_t> ConstantEvaluator::Evaluate(ExpressionNode* node) {
        if (node == nullptr) {
            return std::nullopt;
        }

        Traverse(node);

        if (is_valid_) {
            return current_value_;
        }

        return std::nullopt;
    }

    void ConstantEvaluator::VisitVariableExpression(VariableExpressionNode* node) {
        const SymbolInfo* symbol = nullptr;

        if (std::holds_alternative<const SymbolInfo*>(node->linked_symbols)) {
            symbol = std::get<const SymbolInfo*>(node->linked_symbols);
        }

        if (symbol == nullptr || symbol->kind != SymbolKind::kVariable) {
            is_valid_ = false;
            return;
        }

        if (visited_symbols_.contains(symbol)) {
            is_valid_ = false;
            return;
        }

        if (const auto* var_decl = dynamic_cast<const VariableDeclarationNode*>(symbol->node)) {
            if (!var_decl->type_spec.has_keyword("const") || var_decl->init == nullptr) {
                is_valid_ = false;
                return;
            }

            visited_symbols_.emplace(symbol);
            auto result = Evaluate(var_decl->init.get());

            if (result.has_value()) {
                current_value_ = *result;
            } else {
                is_valid_ = false;
            }

            visited_symbols_.erase(symbol);
        } else {
            is_valid_ = false;
        }
    }

    void ConstantEvaluator::VisitBinaryExpression(BinaryExpressionNode* node) {
        if (node->left == nullptr || node->right == nullptr) {
            is_valid_ = false;
            return;
        }

        auto left_result  = Evaluate(node->left.get());
        auto right_result = Evaluate(node->right.get());

        if (!left_result || !right_result) {
            is_valid_ = false;
            return;
        }

        switch (node->op) {
        case TokenType::kPlus:
            current_value_ = *left_result + *right_result;
            break;
        case TokenType::kMinus:
            current_value_ = *left_result - *right_result;
            break;
        case TokenType::kStar:
            current_value_ = *left_result * *right_result;
            break;
        case TokenType::kSlash:
            if (*right_result == 0) {
                is_valid_ = false;
                return;
            } else {
                current_value_ = *left_result / *right_result;
            }

            break;
        case TokenType::kPercent:
            if (*right_result == 0) {
                is_valid_ = false;
                return;
            } else {
                current_value_ = *left_result % *right_result;
            }

            break;
        case TokenType::kAmpersand:
            current_value_ = *left_result & *right_result;
            break;
        case TokenType::kVerticalBar:
            current_value_ = *left_result | *right_result;
            break;
        case TokenType::kCaret:
            current_value_ = *left_result ^ *right_result;
            break;
        case TokenType::kLeftShift:
            current_value_ = *left_result << *right_result;
            break;
        case TokenType::kRightShift:
            current_value_ = *left_result >> *right_result;
            break;
        default:
            is_valid_ = false;
            break;
        }
    }

    void ConstantEvaluator::VisitRawExpression(RawExpressionNode* node) {
        if (node->tokens.size() != 1 || node->tokens.front().type != TokenType::kNumberLiteral) {
            is_valid_ = false;
            return;
        }

        const auto& token = node->tokens.front();
        std::from_chars(token.text.data(), token.text.data() + token.text.size(), current_value_);
    }
}
