#include "pch.hpp"
#include "ConstantEvaluator.hpp"

#include <charconv>
#include <format>
#include <system_error>
#include <type_traits>
#include <utility>

#include "Analyzer/Syntax/Token.hpp"

namespace glsld {
    ConstantEvaluator::ConstantEvaluator()
        : AstVisitor(0, nullptr)
    {}

    std::optional<ConstantEvaluator::ValueType> ConstantEvaluator::Evaluate(ExpressionNode* node) {
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
        if (node->name == "true") {
            current_value_ = true;
            return;

        } else if (node->name == "false") {
            current_value_ = false;
            return;
        }

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
            auto result = Evaluate(var_decl->init);

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

        auto left_result  = Evaluate(node->left);
        auto right_result = Evaluate(node->right);

        if (!left_result || !right_result) {
            is_valid_ = false;
            return;
        }

        auto PromoteArithmetic = [](const auto& lhs, const auto& rhs)
            -> std::optional<std::pair<ValueType, ValueType>>
        {
            if (std::holds_alternative<bool>(lhs) || std::holds_alternative<bool>(rhs)) {
                return std::nullopt;
            }

            if (lhs.index() == rhs.index()) {
                return std::make_pair(lhs, rhs);
            }

            auto ToDouble = [](const auto& value) -> double {
                return std::visit([](auto&& arg) -> double {
                    return static_cast<double>(arg);
                }, value);
            };

            if (std::holds_alternative<double>(lhs) || std::holds_alternative<double>(rhs)) {
                return std::make_pair(ToDouble(lhs), ToDouble(rhs));
            }

            return std::nullopt;
        };

        switch (node->op) {
        case TokenType::kPlus:
        case TokenType::kMinus:
        case TokenType::kStar:
        case TokenType::kSlash: {
            auto promoted = PromoteArithmetic(*left_result, *right_result);
            if (!promoted.has_value()) {
                is_valid_ = false;
                return;
            }

            const auto& [promoted_left, promoted_right] = *promoted;
            std::visit([this, op = node->op](auto&& lhs, auto&& rhs) -> void {
                using LhsTy = std::decay_t<decltype(lhs)>;
                using RhsTy = std::decay_t<decltype(rhs)>;
                if constexpr (!std::same_as<LhsTy, bool> && !std::same_as<RhsTy, bool>) {
                    switch (op) {
                    case TokenType::kPlus:
                        current_value_ = lhs + rhs;
                        break;
                    case TokenType::kMinus:
                        current_value_ = lhs - rhs;
                        break;
                    case TokenType::kStar:
                        current_value_ = lhs * rhs;
                        break;
                    case TokenType::kSlash:
                        if (rhs == 0.0) {
                            is_valid_ = false;
                        } else {
                            current_value_ = lhs / rhs;
                        }
                        break;
                    default:
                        is_valid_ = false;
                        break;
                    }
                }
            }, promoted_left, promoted_right);

            break;
        }

        case TokenType::kPercent:
            if (left_result->index() != right_result->index() ||
                std::holds_alternative<bool>(*left_result) ||
                std::holds_alternative<double>(*left_result))
            {
                is_valid_ = false;
                return;
            }

            std::visit([this](auto&& lhs, auto&& rhs) -> void {
                using LhsTy = std::decay_t<decltype(lhs)>;
                using RhsTy = std::decay_t<decltype(rhs)>;
                if constexpr (std::is_integral_v<LhsTy> && !std::same_as<LhsTy, bool> && std::same_as<LhsTy, RhsTy>) {
                    if (rhs == 0) {
                        is_valid_ = false;
                    } else {
                        current_value_ = lhs % rhs;
                    }
                } else {
                    is_valid_ = false;
                }
            }, *left_result, *right_result);

            break;
        case TokenType::kAmpersand:
        case TokenType::kVerticalBar:
        case TokenType::kCaret:
        case TokenType::kLeftShift:
        case TokenType::kRightShift: {
            if (left_result->index() != right_result->index() ||
                std::holds_alternative<bool>(*left_result) ||
                std::holds_alternative<double>(*left_result))
            {
                is_valid_ = false;
                return;
            }

            std::visit([this, op = node->op](auto&& lhs, auto&& rhs) -> void {
                using LhsTy = std::decay_t<decltype(lhs)>;
                using RhsTy = std::decay_t<decltype(rhs)>;
                if constexpr (std::is_integral_v<LhsTy> && !std::same_as<LhsTy, bool> && std::same_as<LhsTy, RhsTy>) {
                    switch (op) {
                    case TokenType::kAmpersand:
                        current_value_ = lhs & rhs;
                        break;
                    case TokenType::kVerticalBar:
                        current_value_ = lhs | rhs;
                        break;
                    case TokenType::kCaret:
                        current_value_ = lhs ^ rhs;
                        break;
                    case TokenType::kLeftShift:
                        current_value_ = lhs << rhs;
                        break;
                    case TokenType::kRightShift:
                        current_value_ = lhs >> rhs;
                        break;
                    default:
                        is_valid_ = false;
                        break;
                    }
                } else {
                    is_valid_ = false;
                }
            }, * left_result, * right_result);

            break;
        }

        default:
            is_valid_ = false;
            break;
        }
    }

    void ConstantEvaluator::VisitUnaryExpression(UnaryExpressionNode* node) {
        if (node->operand == nullptr) {
            is_valid_ = false;
            return;
        }

        auto result = Evaluate(node->operand);
        if (!result.has_value()) {
            is_valid_ = false;
            return;
        }

        switch (node->op) {
        case TokenType::kMinus:
            if (std::holds_alternative<bool>(*result)) {
                is_valid_ = false;
            } else {
                std::visit([this](auto&& value) -> void {
                    using Ty = std::decay_t<decltype(value)>;
                    if constexpr ((std::is_integral_v<Ty> && std::is_signed_v<Ty>) || std::is_floating_point_v<Ty>) {
                        current_value_ = -value;
                    } else {
                        is_valid_ = false;
                    }
                }, *result);
            }

            break;
        case TokenType::kPlus:
            current_value_ = *result;
            break;

        case TokenType::kExclamation:
            if (!std::holds_alternative<bool>(*result)) {
                is_valid_ = false;
            } else {
                current_value_ = !std::get<bool>(*result);
            }

            break;
        case TokenType::kTilde:
            if (std::holds_alternative<bool>(*result) || std::holds_alternative<double>(*result)) {
                is_valid_ = false;
            } else {
                std::visit([this](auto&& value) -> void {
                    using Ty = std::decay_t<decltype(value)>;
                    if constexpr (std::is_integral_v<Ty> && !std::same_as<Ty, bool>) {
                        current_value_ = ~value;
                    } else {
                        is_valid_ = false;
                    }
                }, *result);
            }

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

        const auto& text = node->tokens.front().text;
        if (text.find_first_of(".eEpPfF") != std::string::npos) {
            double value = 0.0;
            auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
            if (ec == std::errc{}) {
                current_value_ = value;
                return;
            }
        } else if (text.find_first_of("uU") != std::string::npos) {
            std::uint64_t value = 0;
            auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
            if (ec == std::errc{}) {
                current_value_ = value;
                return;
            }
        } else {
            std::int64_t value = 0;
            auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
            if (ec == std::errc{}) {
                current_value_ = value;
                return;
            }
        }

        is_valid_ = false;
    }
}
