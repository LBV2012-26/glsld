#pragma once

#include <cstdint>
#include <optional>
#include <unordered_set>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Symbol.hpp"

namespace glsld {
    class ConstantEvaluator final : public AstVisitor {
    public:
        ConstantEvaluator();

        std::optional<std::int64_t> Evaluate(ExpressionNode* node);

    private:
        void VisitVariableExpression(VariableExpressionNode* node) override;
        void VisitBinaryExpression(BinaryExpressionNode* node) override;
        void VisitRawExpression(RawExpressionNode* node) override;

        std::unordered_set<const SymbolInfo*> visited_symbols_;
        std::int64_t                          current_value_{};
        bool                                  is_valid_{ true };
    };
}
