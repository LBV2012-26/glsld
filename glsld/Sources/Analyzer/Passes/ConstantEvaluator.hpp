#pragma once

#include <cstdint>
#include <concepts>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <ankerl/unordered_dense.h>
#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    CallExpressionNode* FindLengthCall(MemberAccessExpressionNode* node);

    class ConstantEvaluator final : public AstVisitor {
    public:
        using ScalarValue = std::variant<std::int64_t, std::uint64_t, double, bool>;

        struct AggregateValue {
            TypeDescriptor           type_desc;
            std::vector<ScalarValue> components;
        };

        using ValueType = std::variant<ScalarValue, AggregateValue>;

        ConstantEvaluator();

        template <typename Ty>
        requires std::same_as<Ty, std::int64_t>
              || std::same_as<Ty, std::uint64_t>
              || std::same_as<Ty, double>
              || std::same_as<Ty, bool>
              || std::same_as<Ty, std::string>
        std::optional<Ty> EvaluateAs(ExpressionNode* node);
    private:
        using EvaluatorFunc = std::function<std::optional<ValueType>(std::span<const ValueType>, const TypeInfo&)>;

        void RegisterBuiltins();
        void Register(std::string_view name, EvaluatorFunc func);

        std::optional<ValueType> Evaluate(ExpressionNode* node);

        enum class ConversionMode {
            kExplicit,
            kImplicit
        };

        std::optional<ValueType> ConvertValueToType(
            const ValueType& value,
            const TypeInfo& target_type,
            ConversionMode mode) const;

        std::optional<ValueType> EvaluateBuiltinFunction(
            std::string_view name,
            std::span<const ValueType> args,
            const TypeInfo& result_type);

        std::optional<ValueType> EvaluateConstructor(CallExpressionNode* node, const TypeInfo& target_type);
        std::optional<std::string> FormatValue(const ValueType& value) const;

        void VisitVariableExpression(VariableExpressionNode* node) override;
        void VisitCastExpression(CastExpressionNode* node) override;
        void VisitBinaryExpression(BinaryExpressionNode* node) override;
        void VisitUnaryExpression(UnaryExpressionNode* node) override;
        void VisitCallExpression(CallExpressionNode* node) override;
        void VisitIndexExpression(IndexExpressionNode* node) override;
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
