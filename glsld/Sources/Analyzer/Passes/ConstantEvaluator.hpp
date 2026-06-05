#pragma once

#include <cstdint>
#include <concepts>
#include <optional>
#include <string>
#include <variant>

#include <ankerl/unordered_dense.h>
#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Symbol.hpp"

namespace glsld {
    class ConstantEvaluator final : public AstVisitor {
    public:
        ConstantEvaluator();

        template <typename Ty>
        requires std::same_as<Ty, std::int64_t>
              || std::same_as<Ty, std::uint64_t>
              || std::same_as<Ty, double>
              || std::same_as<Ty, bool>
              || std::same_as<Ty, std::string>
        std::optional<Ty> EvaluateAs(ExpressionNode* node);

    private:
        using ValueType = std::variant<std::int64_t, std::uint64_t, double, bool>;

        std::optional<ValueType> Evaluate(ExpressionNode* node);

        void VisitVariableExpression(VariableExpressionNode* node) override;
        void VisitBinaryExpression(BinaryExpressionNode* node) override;
        void VisitUnaryExpression(UnaryExpressionNode* node) override;
        void VisitRawExpression(RawExpressionNode* node) override;

        ankerl::unordered_dense::set<const SymbolInfo*> visited_symbols_;
        ValueType                                       current_value_{};
        bool                                            is_valid_{ true };
    };
}

#include "ConstantEvaluator.inl"
