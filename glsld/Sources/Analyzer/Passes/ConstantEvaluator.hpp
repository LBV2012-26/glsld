#pragma once

#include <cstdint>
#include <concepts>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>

#include <ankerl/unordered_dense.h>
#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    CallExpressionNode* FindLengthCall(MemberAccessExpressionNode* node);

    class ConstantEvaluator final : public AstVisitor {
    public:
        using ValueType = std::variant<std::int64_t, std::uint64_t, double, bool>;

        ConstantEvaluator();

        template <typename Ty>
        requires std::same_as<Ty, std::int64_t>
              || std::same_as<Ty, std::uint64_t>
              || std::same_as<Ty, double>
              || std::same_as<Ty, bool>
              || std::same_as<Ty, std::string>
        std::optional<Ty> EvaluateAs(ExpressionNode* node);
    private:
        using EvaluatorFunc = std::function<std::optional<ValueType>(std::span<const ValueType>)>;

        void RegisterBuiltins();
        void Register(std::string_view name, EvaluatorFunc func);

        std::optional<ValueType> Evaluate(ExpressionNode* node);
        std::optional<ValueType> ConvertValueToType(const ValueType& value, const TypeInfo& target_type) const;
        std::optional<ValueType> EvaluateBuiltinFunction(std::string_view name, std::span<const ValueType> args);

        void VisitVariableExpression(VariableExpressionNode* node) override;
        void VisitBinaryExpression(BinaryExpressionNode* node) override;
        void VisitUnaryExpression(UnaryExpressionNode* node) override;
        void VisitCallExpression(CallExpressionNode* node) override;
        void VisitRawExpression(RawExpressionNode* node) override;
        void VisitMemberAccessExpression(MemberAccessExpressionNode* node) override;

        ankerl::unordered_dense::set<const SymbolInfo*> visited_symbols_;
        ValueType current_value_{};
        bool      is_valid_{ true };

        using Registry = ankerl::unordered_dense::map<std::string_view, EvaluatorFunc>;
        inline static Registry registry_;

        inline static bool builtins_registered_{ false };
    };
}

#include "ConstantEvaluator.inl"
