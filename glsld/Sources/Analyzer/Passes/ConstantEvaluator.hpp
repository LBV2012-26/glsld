#pragma once

#include <cstdint>
#include <concepts>
#include <functional>
#include <memory>
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

namespace glsld {
    CallExpressionNode* FindLengthCall(MemberAccessExpressionNode* node);

    class ConstantEvaluator final : public AstVisitor {
    public:
        using Scalar = std::variant<std::int64_t, std::uint64_t, double, bool>;

        struct Aggregate {
            TypeDescriptor      type_desc;
            std::vector<Scalar> components;
        };

        struct Array;
        using ArrayPtr = std::shared_ptr<const Array>;

        struct Struct;
        using StructPtr = std::shared_ptr<const Struct>;

        using Value = std::variant<Scalar, Aggregate, ArrayPtr, StructPtr>;

        struct Array {
            TypeInfo           type_info;
            std::vector<Value> elements;
        };

        struct StructField {
            const SymbolInfo* symbol{ nullptr };
            Value             value;
        };

        struct Struct {
            TypeInfo                 type_info;
            std::vector<StructField> fields;
        };

        ConstantEvaluator();

        template <typename Ty>
        requires std::same_as<Ty, std::int64_t>
              || std::same_as<Ty, std::uint64_t>
              || std::same_as<Ty, double>
              || std::same_as<Ty, bool>
              || std::same_as<Ty, std::string>
        std::optional<Ty> EvaluateAs(ExpressionNode* node);

    private:
        using EvaluatorFunc = std::function<std::optional<Value>(std::span<const Value>, const TypeInfo&)>;

        void RegisterBuiltins();
        void Register(std::string_view name, EvaluatorFunc func);

        std::optional<Value> Evaluate(ExpressionNode* node);

        enum class ConversionMode {
            kExplicit,
            kImplicit
        };

        std::optional<Value> ConvertValueToType(
            const Value& value,
            const TypeInfo& target_type,
            ConversionMode mode) const;

        std::optional<Value> ForwardArrayChecked(const Value& value, const TypeInfo& target_type) const;
        std::optional<Value> ForwardStructChecked(const Value& value, const TypeInfo& target_type) const;

        std::optional<Value> EvaluateBuiltinFunction(
            std::string_view name,
            std::span<const Value> args,
            const TypeInfo& result_type);

        std::optional<Value> EvaluateAggregateElements(
            std::span<ExpressionNode* const> elements,
            const TypeInfo& target_type);

        std::optional<Value> EvaluateArrayElements(
            std::span<ExpressionNode* const> elements,
            const TypeInfo& target_type);

        std::optional<Value> EvaluateStructElements(
            std::span<ExpressionNode* const> elements,
            const TypeInfo& target_type);

        std::optional<Value> EvaluateConstructor(
            CallExpressionNode* node,
            const TypeInfo& target_type);

        std::optional<std::string> FormatValue(const Value& value, std::size_t indent = 0) const;

        void VisitVariableExpression(VariableExpressionNode* node) override;
        void VisitInitializerListExpression(InitializerListExpressionNode* node) override;
        void VisitCastExpression(CastExpressionNode* node) override;
        void VisitBinaryExpression(BinaryExpressionNode* node) override;
        void VisitUnaryExpression(UnaryExpressionNode* node) override;
        void VisitCallExpression(CallExpressionNode* node) override;
        void VisitIndexExpression(IndexExpressionNode* node) override;
        void VisitRawExpression(RawExpressionNode* node) override;
        void VisitMemberAccessExpression(MemberAccessExpressionNode* node) override;

        ankerl::unordered_dense::set<const SymbolInfo*> visited_symbols_;
        Value current_value_{};
        bool  is_valid_{ true };

        using Registry = ankerl::unordered_dense::map<std::string_view, EvaluatorFunc>;
        inline static Registry registry_;

        inline static bool builtins_registered_{ false };
    };
}

#include "ConstantEvaluator.inl"
