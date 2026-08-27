#include "pch.hpp"
#include "ConstantEvaluator.hpp"

#include <cmath>
#include <cstddef>
#include <algorithm>
#include <charconv>
#include <concepts>
#include <format>
#include <limits>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Analyzer/Syntax/Token.hpp"
#include "Base/FileSystem/Source.hpp"
#include "Base/MathMeta.hpp"

namespace glsld {
    CallExpressionNode* FindLengthCall(MemberAccessExpressionNode* node) {
        if (node == nullptr || node->member == nullptr ||
            node->member->kind() != AstNodeKind::kCallExpression)
        {
            return nullptr;
        }

        auto* call = static_cast<CallExpressionNode*>(node->member);
        if (!call->args.empty() || call->callee == nullptr ||
            call->callee->kind() != AstNodeKind::kVariableExpression)
        {
            return nullptr;
        }

        auto* callee = static_cast<VariableExpressionNode*>(call->callee);
        if (callee->name != "length" ||
            callee->original_token.type != TokenType::kBuiltInFunction)
        {
            return nullptr;
        }

        return call;
    }

    ConstantEvaluator::ConstantEvaluator()
        : AstVisitor(0, nullptr)
    {
        if (!builtins_registered_) {
            builtins_registered_ = true;
            RegisterBuiltins();
        }
    }

    namespace {
        using Scalar    = ConstantEvaluator::ScalarValue;
        using Aggregate = ConstantEvaluator::AggregateValue;
        using Value     = ConstantEvaluator::ValueType;

        const Scalar* GetScalar(const Value& value) {
            return std::get_if<Scalar>(&value);
        }

        template<typename Ty>
        const Ty* GetScalarIf(const Value& value) {
            const auto* scalar = GetScalar(value);
            if (scalar == nullptr) {
                return nullptr;
            }
            return std::get_if<Ty>(scalar);
        }

        template <typename Ty>
        inline constexpr bool is_optional_v = false;

        template <typename Ty>
        inline constexpr bool is_optional_v<std::optional<Ty>> = true;

        template <typename Ty>
        concept IsOptional = is_optional_v<std::remove_cvref_t<Ty>>;

        template <typename Ty>
        std::optional<Value> WrapReturnValue(Ty&& value) {
            using Decayed = std::decay_t<Ty>;

            if constexpr (IsOptional<Decayed>) {
                if (!value.has_value()) {
                    return std::nullopt;
                }

                return WrapReturnValue(*std::forward<Ty>(value));
            } else {
                return Value{
                    std::forward<Ty>(value)
                };
            }
        }

        template <typename Tuple, std::size_t... Is>
        std::optional<Tuple> ExtractArgs(std::span<const Value> args, std::index_sequence<Is...>) {
            if (args.size() < sizeof...(Is)) {
                return std::nullopt;
            }

            Tuple result{};
            bool ok = (... && ([&]() -> bool {
                const auto* scalar = GetScalar(args[Is]);
                if (scalar == nullptr) {
                    return false;
                }

                using TargetType = std::decay_t<std::tuple_element_t<Is, Tuple>>;
                const auto* value = std::get_if<TargetType>(scalar);
                if (value == nullptr) {
                    return false;
                }

                std::get<Is>(result) = *value;
                return true;
            }()));

            if (ok) {
                return result;
            }

            return std::nullopt;
        }

        template <typename Return, typename... Args>
        auto WrapSingleSignature(Return(*func)(Args...)) {
            return [func](std::span<const Value> args, const TypeInfo&) -> std::optional<Value> {
                if (args.size() != sizeof...(Args)) {
                    return std::nullopt;
                }

                using ArgumentTuple = std::tuple<std::decay_t<Args>...>;
                const auto extracted = ExtractArgs<ArgumentTuple>(args, std::index_sequence_for<Args...>{});
                if (!extracted.has_value()) {
                    return std::nullopt;
                }

                if constexpr (std::same_as<Return, void>) {
                    std::apply(func, *extracted);
                    return std::nullopt;
                } else {
                    auto result = std::apply(func, *extracted);
                    return WrapReturnValue(std::move(result));
                }
            };
        }

        template <typename... Evaluators>
        auto MakeOverloader(Evaluators... funcs) {
            return [overloads = std::make_tuple(std::move(funcs)...)](
                std::span<const Value> args,
                const TypeInfo& result_type
            ) -> std::optional<Value> {
                std::optional<Value> result;
                std::apply([&](const auto&... evaluator) {
                    (... || ([&]() -> bool {
                        result = evaluator(args, result_type);
                        return result.has_value();
                    }()));
                }, overloads);

                return result;
            };
        }

        std::size_t ComponentCount(const TypeDescriptor& type_desc) {
            if (type_desc.vector_count <= 0 || type_desc.vector_length <= 0) {
                return 0;
            }

            return static_cast<std::size_t>(type_desc.vector_count * type_desc.vector_length);
        }

        bool IsAggregateType(const TypeDescriptor& type_desc) {
            return type_desc.vector_count > 1 || type_desc.vector_length > 1;
        }

        template <typename ScalarEvaluator>
        auto MakeComponentWise(ScalarEvaluator func) {
            return [func = std::move(func)](
                std::span<const Value> args,
                const TypeInfo& result_type
            ) -> std::optional<Value> {
                const bool all_scalar = std::ranges::all_of(args, [](const auto& argv) -> bool {
                    return std::holds_alternative<Scalar>(argv);
                });

                if (all_scalar)
                    return func(args, result_type);
                if (!IsAggregateType(result_type.type_desc))
                    return std::nullopt;

                const auto component_count = ComponentCount(result_type.type_desc);
                if (component_count == 0) {
                    return std::nullopt;
                }

                Aggregate result{
                    .type_desc  = result_type.type_desc,
                    .components = {}
                };

                result.components.reserve(component_count);

                std::vector<Value> component_args;
                component_args.reserve(args.size());

                for (auto i = 0uz; i != component_count; ++i) {
                    component_args.clear();

                    for (const auto& argv : args) {
                        if (const auto* scalar = std::get_if<Scalar>(&argv)) {
                            component_args.emplace_back(*scalar);
                            continue;
                        }

                        const auto* aggregate = std::get_if<Aggregate>(&argv);
                        if (aggregate == nullptr || aggregate->components.size() != component_count) {
                            return std::nullopt;
                        }

                        component_args.emplace_back(Scalar{
                            aggregate->components[i]
                        });
                    }

                    const auto component_result = func(component_args, result_type);
                    if (!component_result.has_value()) {
                        return std::nullopt;
                    }

                    const auto* scalar_result = std::get_if<Scalar>(&*component_result);
                    if (scalar_result == nullptr) {
                        return std::nullopt;
                    }

                    result.components.push_back(*scalar_result);
                }

                return Value{ std::move(result) };
            };
        }
    }

#define REGISTER_OVERLOADS(name, func) \
    Register(name, MakeOverloader(WrapSingleSignature(func<std::int64_t>), WrapSingleSignature(func<std::uint64_t>), WrapSingleSignature(func<double>)))

#define REGISTER_OVERLOADS_INTEGER_ONLY(name, func) \
    Register(name, MakeOverloader(WrapSingleSignature(func<std::int64_t>), WrapSingleSignature(func<std::uint64_t>)))

#define REGISTER_COMPONENT_WISE_OVERLOADS(name, func) \
    Register(name, MakeComponentWise(MakeOverloader(WrapSingleSignature(func<std::int64_t>), WrapSingleSignature(func<std::uint64_t>), WrapSingleSignature(func<double>))))

    void ConstantEvaluator::RegisterBuiltins() {
        Register("radians", WrapSingleSignature(+[](double degrees) -> double {
            return MathMeta::Radians(degrees);
        }));

        Register("degrees", WrapSingleSignature(+[](double radians) -> double {
            return MathMeta::Degrees(radians);
        }));

        Register("sin", WrapSingleSignature(+[](double angle) -> double {
            return MathMeta::Sin(angle);
        }));

        Register("cos", WrapSingleSignature(+[](double angle) -> double {
            return MathMeta::Cos(angle);
        }));

        Register("tan", WrapSingleSignature(+[](double angle) -> double {
            return MathMeta::Tan(angle);
        }));

        Register("asin", WrapSingleSignature(+[](double value) -> std::optional<double> {
            return MathMeta::Asin(value);
        }));

        Register("acos", WrapSingleSignature(+[](double value) -> std::optional<double> {
            return MathMeta::Acos(value);
        }));

        Register("atan", MakeOverloader(
            WrapSingleSignature(+[](double numerator_y, double denominator_x) -> double {
                return MathMeta::Atan2(numerator_y, denominator_x);
            }),
            WrapSingleSignature(+[](double slope_y_over_x) -> double {
                return MathMeta::Atan(slope_y_over_x);
            })
        ));

        Register("sinh", WrapSingleSignature(+[](double value) -> double {
            return MathMeta::Sinh(value);
        }));

        Register("cosh", WrapSingleSignature(+[](double value) -> double {
            return MathMeta::Cosh(value);
        }));

        Register("tanh", WrapSingleSignature(+[](double value) -> double {
            return MathMeta::Tanh(value);
        }));

        Register("asinh", WrapSingleSignature(+[](double value) -> double {
            return MathMeta::Asinh(value);
        }));

        Register("acosh", WrapSingleSignature(+[](double value) -> std::optional<double> {
            return MathMeta::Acosh(value);
        }));

        Register("atanh", WrapSingleSignature(+[](double value) -> std::optional<double> {
            return MathMeta::Atanh(value);
        }));

        Register("pow", WrapSingleSignature(+[](double base, double exponent) -> std::optional<double> {
            return MathMeta::Pow(base, exponent);
        }));

        Register("exp", WrapSingleSignature(+[](double value) -> double {
            return MathMeta::Exp(value);
        }));

        Register("exp2", WrapSingleSignature(+[](double value) -> double {
            return MathMeta::Exp2(value);
        }));

        Register("log", WrapSingleSignature(+[](double value) -> std::optional<double> {
            return MathMeta::Log(value);
        }));

        Register("log2", WrapSingleSignature(+[](double value) -> std::optional<double> {
            return MathMeta::Log2(value);
        }));

        Register("sqrt", WrapSingleSignature(+[](double value) -> std::optional<double> {
            return MathMeta::Sqrt(value);
        }));

        Register("inversesqrt", WrapSingleSignature(+[](double value) -> std::optional<double> {
            return MathMeta::InverseSqrt(value);
        }));

        REGISTER_OVERLOADS("abs", MathMeta::Abs);
        REGISTER_OVERLOADS("sign", MathMeta::Sign);
        REGISTER_OVERLOADS("min", MathMeta::Min);
        REGISTER_OVERLOADS("max", MathMeta::Max);
        REGISTER_OVERLOADS("clamp", MathMeta::Clamp);
        REGISTER_OVERLOADS("mod", MathMeta::Mod);
        REGISTER_OVERLOADS("step", MathMeta::Step);
        REGISTER_OVERLOADS("fma", MathMeta::Fma);

        Register("floor", WrapSingleSignature(+[](double value) -> double { return MathMeta::Floor(value); }));
        Register("trunc", WrapSingleSignature(+[](double value) -> double { return MathMeta::Trunc(value); }));
        Register("round", WrapSingleSignature(+[](double value) -> double { return MathMeta::Round(value); }));
        Register("roundEven", WrapSingleSignature(+[](double value) -> double { return MathMeta::RoundEven(value); }));
        Register("ceil", WrapSingleSignature(+[](double value) -> double { return MathMeta::Ceil(value); }));
        Register("fract", WrapSingleSignature(+[](double value) -> double { return MathMeta::Fract(value); }));

        Register("smoothstep", WrapSingleSignature(+[](double edge_start, double edge_end, double value) -> double {
            return MathMeta::SmoothStep(edge_start, edge_end, value);
        }));

        Register("mix", MakeOverloader(
            WrapSingleSignature(+[](double lhs, double rhs, double factor) -> double {
                return MathMeta::Mix(lhs, rhs, factor);
            }),
            WrapSingleSignature(+[](double lhs, double rhs, bool condition) -> double {
                return MathMeta::MixBool(lhs, rhs, condition);
            }),
            WrapSingleSignature(+[](std::int64_t lhs, std::int64_t rhs, bool condition) -> std::int64_t {
                return MathMeta::MixBool(lhs, rhs, condition);
            }),
            WrapSingleSignature(+[](std::uint64_t lhs, std::uint64_t rhs, bool condition) -> std::uint64_t {
                return MathMeta::MixBool(lhs, rhs, condition);
            })
        ));

        Register("isnan", WrapSingleSignature(+[](double value) -> bool { return MathMeta::IsNan(value); }));
        Register("isinf", WrapSingleSignature(+[](double value) -> bool { return MathMeta::IsInf(value); }));

        Register("ldexp", WrapSingleSignature(+[](double value, std::int64_t exponent) -> double {
            return MathMeta::Ldexp(value, exponent);
        }));

        Register("floatBitsToInt", WrapSingleSignature(+[](double value) -> std::int64_t {
            return MathMeta::FloatBitsToInt(value);
        }));
        Register("floatBitsToUint", WrapSingleSignature(+[](double value) -> std::uint64_t {
            return MathMeta::FloatBitsToUint(value);
        }));
        Register("intBitsToFloat", WrapSingleSignature(+[](std::int64_t value) -> double {
            return MathMeta::IntBitsToFloat(value);
        }));
        Register("uintBitsToFloat", WrapSingleSignature(+[](std::uint64_t value) -> double {
            return MathMeta::UintBitsToFloat(value);
        }));

        Register("packUnorm2x16", WrapSingleSignature(+[](double x, double y) -> std::uint64_t {
            return MathMeta::PackUnorm2x16(x, y);
        }));
        Register("packSnorm2x16", WrapSingleSignature(+[](double x, double y) -> std::uint64_t {
            return MathMeta::PackSnorm2x16(x, y);
        }));
        Register("packUnorm4x8", WrapSingleSignature(+[](double x, double y, double z, double w) -> std::uint64_t {
            return MathMeta::PackUnorm4x8(x, y, z, w);
        }));
        Register("packSnorm4x8", WrapSingleSignature(+[](double x, double y, double z, double w) -> std::uint64_t {
            return MathMeta::PackSnorm4x8(x, y, z, w);
        }));
        Register("packHalf2x16", WrapSingleSignature(+[](double x, double y) -> std::uint64_t {
            return MathMeta::PackHalf2x16(x, y);
        }));
        Register("packDouble2x32", WrapSingleSignature(+[](std::uint64_t x, std::uint64_t y) -> double {
            return MathMeta::PackDouble2x32(x, y);
        }));

        Register("length", WrapSingleSignature(+[](double value) -> double {
            return MathMeta::Length(value);
        }));

        Register("distance", WrapSingleSignature(+[](double x, double y) -> double {
            return MathMeta::Distance(x, y);
        }));

        REGISTER_OVERLOADS("dot", MathMeta::Dot);

        Register("normalize", WrapSingleSignature(+[](double value) -> std::optional<double> {
            return MathMeta::Normalize(value);
        }));

        Register("faceforward", WrapSingleSignature(+[](double normal, double incident, double reference_normal) -> double {
            return MathMeta::FaceForward(normal, incident, reference_normal);
        }));

        Register("reflect", WrapSingleSignature(+[](double incident, double normal) -> double {
            return MathMeta::Reflect(incident, normal);
        }));

        Register("refract", WrapSingleSignature(+[](double incident, double normal, double eta) -> double {
            return MathMeta::Refract(incident, normal, eta);
        }));

        REGISTER_OVERLOADS("lessThan", MathMeta::LessThan);
        REGISTER_OVERLOADS("lessThanEqual", MathMeta::LessThanEqual);
        REGISTER_OVERLOADS("greaterThan", MathMeta::GreaterThan);
        REGISTER_OVERLOADS("greaterThanEqual", MathMeta::GreaterThanEqual);
        REGISTER_OVERLOADS("equal", MathMeta::Equal);
        REGISTER_OVERLOADS("notEqual", MathMeta::NotEqual);

        Register("not", WrapSingleSignature(+[](bool condition) -> bool {
            return MathMeta::LogicalNot(condition);
        }));

        Register("any", WrapSingleSignature(+[](bool condition) -> bool {
            return MathMeta::Any(condition);
        }));

        Register("all", WrapSingleSignature(+[](bool condition) -> bool {
            return MathMeta::All(condition);
        }));

        REGISTER_OVERLOADS_INTEGER_ONLY("bitCount", MathMeta::BitCount);
        REGISTER_OVERLOADS_INTEGER_ONLY("findLSB", MathMeta::FindLsb);
        REGISTER_OVERLOADS_INTEGER_ONLY("findMSB", MathMeta::FindMsb);
        REGISTER_OVERLOADS_INTEGER_ONLY("bitfieldReverse", MathMeta::BitfieldReverse);
        REGISTER_OVERLOADS_INTEGER_ONLY("bitfieldExtract", MathMeta::BitfieldExtract);
        REGISTER_OVERLOADS_INTEGER_ONLY("bitfieldInsert", MathMeta::BitfieldInsert);

        Register("umulExtended", WrapSingleSignature(+[](std::uint64_t multiplier, std::uint64_t multiplicand) -> std::uint64_t {
            return MathMeta::UmulExtended(multiplier, multiplicand);
        }));

        Register("imulExtended", WrapSingleSignature(+[](std::int64_t multiplier, std::int64_t multiplicand) -> std::int64_t {
            return MathMeta::ImulExtended(multiplier, multiplicand);
        }));

        Register("uaddCarry", WrapSingleSignature(+[](std::uint64_t lhs, std::uint64_t rhs) -> std::uint64_t {
            return MathMeta::UaddCarry(lhs, rhs);
        }));

        Register("usubBorrow", WrapSingleSignature(+[](std::uint64_t lhs, std::uint64_t rhs) -> std::uint64_t {
            return MathMeta::UsubBorrow(lhs, rhs);
        }));
    }

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

    namespace {
        std::optional<Scalar> ConvertScalarValue(const Scalar& scalar, BaseFamily target_family, bool explicit_conversion) {
            switch (target_family) {
            case BaseFamily::kBool:
                if (const auto* source = std::get_if<bool>(&scalar))
                    return *source;
                if (!explicit_conversion)
                    return std::nullopt;
                if (const auto* source = std::get_if<std::int64_t>(&scalar))
                    return *source != 0;
                if (const auto* source = std::get_if<std::uint64_t>(&scalar))
                    return *source != 0;
                if (const auto* source = std::get_if<double>(&scalar))
                    return *source != 0.0;
                break;
            case BaseFamily::kInt:
                if (const auto* source = std::get_if<std::int64_t>(&scalar)) {
                    return *source;
                }

                if (const auto* source = std::get_if<std::uint64_t>(&scalar)) {
                    if (*source <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
                        return static_cast<std::int64_t>(*source);
                    }
                }

                if (!explicit_conversion) {
                    return std::nullopt;
                }

                if (const auto* source = std::get_if<double>(&scalar)) {
                    constexpr double kMin          = -9223372036854775808.0; // -2^63
                    constexpr double kMaxExclusive =  9223372036854775808.0; //  2^63

                    if (std::isfinite(*source) &&
                        *source >= kMin &&
                        *source < kMaxExclusive)
                    {
                        return static_cast<std::int64_t>(*source);
                    }
                }

                if (const auto* source = std::get_if<bool>(&scalar)) {
                    return static_cast<std::int64_t>(*source);
                }

                break;
            case BaseFamily::kUint:
                if (const auto* source = std::get_if<std::uint64_t>(&scalar)) {
                    return *source;
                }

                if (const auto* source = std::get_if<std::int64_t>(&scalar)) {
                    if (source != nullptr && *source >= 0) {
                        return static_cast<std::uint64_t>(*source);
                    }
                }

                if (!explicit_conversion) {
                    return std::nullopt;
                }

                if (const auto* source = std::get_if<double>(&scalar)) {
                    constexpr double kMaxExclusive = 18446744073709551616.0;

                    if (std::isfinite(*source) &&
                        *source >= 0.0 &&
                        *source < kMaxExclusive)
                    {
                        return static_cast<std::uint64_t>(*source);
                    }
                }

                if (const auto* source = std::get_if<bool>(&scalar)) {
                    return static_cast<std::uint64_t>(*source);
                }

                break;
            case BaseFamily::kFloat:
                if (const auto* source = std::get_if<double>(&scalar))
                    return *source;
                if (const auto* source = std::get_if<std::int64_t>(&scalar))
                    return static_cast<double>(*source);
                if (const auto* source = std::get_if<std::uint64_t>(&scalar))
                    return static_cast<double>(*source);

                if (explicit_conversion) {
                    if (const auto* source = std::get_if<bool>(&scalar)) {
                        return *source ? 1.0 : 0.0;
                    }
                }
                break;
            default:
                break;
            }

            return std::nullopt;
        }
    }

    std::optional<ConstantEvaluator::ValueType> ConstantEvaluator::ConvertValueToType(
        const ValueType& value,
        const TypeInfo& target_type,
        ConversionMode mode) const
    {
        if (!target_type.is_valid() ||
            target_type.is_array()  ||
            target_type.block_symbol != nullptr)
        {
            return std::nullopt;
        }

        bool explicit_conversion = mode == ConversionMode::kExplicit;

        const auto& target_desc = target_type.type_desc;
        if (!IsAggregateType(target_desc)) {
            const auto* scalar = GetScalar(value);
            if (scalar == nullptr) {
                return std::nullopt;
            }

            return ConvertScalarValue(*scalar, target_desc.family, explicit_conversion);
        }

        const auto* aggregate = std::get_if<Aggregate>(&value);
        if (aggregate == nullptr ||
            aggregate->type_desc.vector_count  != target_desc.vector_count ||
            aggregate->type_desc.vector_length != target_desc.vector_length)
        {
            return std::nullopt;
        }

        Aggregate result{
            .type_desc  = target_desc,
            .components = {}
        };

        result.components.reserve(aggregate->components.size());

        for (const auto& component : aggregate->components) {
            const auto converted = ConvertScalarValue(component, target_desc.family, explicit_conversion);
            if (!converted.has_value()) {
                return std::nullopt;
            }

            result.components.push_back(*converted);
        }

        return result;
    }

    std::optional<ConstantEvaluator::ValueType> ConstantEvaluator::EvaluateBuiltinFunction(
        std::string_view name,
        std::span<const ValueType> args,
        const TypeInfo& result_type)
    {
        auto it = registry_.find(name);
        if (it != registry_.end()) {
            return it->second(args, result_type);
        }

        return std::nullopt;
    }

    std::optional<ConstantEvaluator::ValueType> ConstantEvaluator::EvaluateConstructor(
        CallExpressionNode* node,
        const TypeInfo& target_type)
    {
        if (node == nullptr ||
            !target_type.is_valid() ||
            target_type.is_array() ||
            target_type.block_symbol != nullptr)
        {
            return std::nullopt;
        }

        std::vector<ValueType> args;
        args.reserve(node->args.size());

        for (auto* arg_node : node->args) {
            if (arg_node == nullptr) {
                return std::nullopt;
            }

            auto argv = Evaluate(arg_node);
            if (!argv.has_value()) {
                return std::nullopt;
            }

            args.push_back(std::move(*argv));
        }

        const auto& target_desc = target_type.type_desc;
        // int(...), float(...)
        if (!IsAggregateType(target_desc)) {
            if (args.size() != 1) {
                return std::nullopt;
            }

            return ConvertValueToType(args.front(), target_type, ConversionMode::kExplicit);
        }

        const bool is_vector = target_desc.vector_count == 1 && target_desc.vector_length > 1;
        const bool is_matrix = target_desc.vector_count  > 1 && target_desc.vector_length > 1;
        if (!is_vector && !is_matrix) {
            return std::nullopt;
        }

        const auto target_count = ComponentCount(target_desc);
        if (target_count == 0) {
            return std::nullopt;
        }

        // vec4(1.0)
        if (is_vector && args.size() == 1) {
            if (const auto* scalar = GetScalar(args.front())) {
                const auto converted = ConvertScalarValue(*scalar, target_desc.family, true);
                if (!converted.has_value()) {
                    return std::nullopt;
                }

                return Aggregate{
                    .type_desc  = target_desc,
                    .components = std::vector<Scalar>(target_count, *converted)
                };
            }
        }

        // mat4(1.0)
        if (is_matrix && args.size() == 1) {
            if (const auto* scalar = GetScalar(args.front())) {
                const auto converted = ConvertScalarValue(*scalar, target_desc.family, true);
                if (!converted.has_value()) {
                    return std::nullopt;
                }

                Aggregate result{
                    .type_desc  = target_desc,
                    .components = std::vector<Scalar>(target_count, Scalar{ 0.0 })
                };

                const auto diagonal_size = std::min(target_desc.vector_count, target_desc.vector_length);
                for (int i = 0; i != diagonal_size; ++i) {
                    const auto index = static_cast<std::size_t>(i * target_desc.vector_length + i);
                    result.components[index] = *converted;
                }

                return result;
            }
        }

        // mat3(mat2(...)), mat2(mat3(...))
        if (is_matrix && args.size() == 1) {
            const auto* source = std::get_if<Aggregate>(&args.front());
            if (source != nullptr &&
                source->type_desc.vector_count  > 1 &&
                source->type_desc.vector_length > 1)
            {
                Aggregate result{
                    .type_desc  = target_desc,
                    .components = std::vector<Scalar>(target_count, Scalar{ 0.0 })
                };

                // 扩展矩阵时，新增的对角线分量为 1
                const auto diagonal_size = std::min(target_desc.vector_count, target_desc.vector_length);
                for (int i = 0; i != diagonal_size; ++i) {
                    const auto index = static_cast<std::size_t>(i * target_desc.vector_length + i);
                    result.components[index] = Scalar{ 1.0 };
                }

                const auto copy_columns = std::min(target_desc.vector_count,  source->type_desc.vector_count);
                const auto copy_rows    = std::min(target_desc.vector_length, source->type_desc.vector_length);

                for (int column = 0; column != copy_columns; ++column) {
                    for (int row = 0; row != copy_rows; ++row) {
                        const auto source_index = static_cast<std::size_t>(column * source->type_desc.vector_length + row);
                        const auto target_index = static_cast<std::size_t>(column * target_desc.vector_length + row);
                        const auto converted    = ConvertScalarValue(source->components[source_index], target_desc.family, true);

                        if (!converted.has_value()) {
                            return std::nullopt;
                        }

                        result.components[target_index] = *converted;
                    }
                }

                return result;
            }
        }

        // 收集 vec4(1, vec2(2, 3), 4) 或 mat2(vec2(...), vec2(...)) 的分量
        std::vector<Scalar> flattened;
        flattened.reserve(target_count);

        for (auto arg_index = 0uz; arg_index != args.size(); ++arg_index) {
            const auto& argv = args[arg_index];
            if (const auto* scalar = GetScalar(argv)) {
                flattened.push_back(*scalar);
                continue;
            }

            const auto* aggregate = std::get_if<Aggregate>(&argv);

            // 普通分量列表中只允许向量，不展开矩阵
            if (aggregate == nullptr ||
                aggregate->type_desc.vector_count  != 1 ||
                aggregate->type_desc.vector_length <= 1)
            {
                return std::nullopt;
            }

            flattened.append_range(aggregate->components);
        }

        if (flattened.size() < target_count) {
            return std::nullopt;
        }

        if (flattened.size() > target_count) {
            // 仅允许最后一个向量提供多余分量，如 vec3(vec4(...))
            const auto* last = std::get_if<Aggregate>(&args.back());

            if (last == nullptr || last->type_desc.vector_count != 1) {
                return std::nullopt;
            }

            const auto count_before_last = flattened.size() - last->components.size();
            if (count_before_last >= target_count) {
                return std::nullopt;
            }

            flattened.resize(target_count);
        }

        Aggregate result{
            .type_desc  = target_desc,
            .components = {}
        };

        result.components.reserve(target_count);

        for (const auto& component : flattened) {
            const auto converted = ConvertScalarValue(component, target_desc.family, true);
            if (!converted.has_value()) {
                return std::nullopt;
            }

            result.components.push_back(*converted);
        }

        return result;
    }

    namespace {
        std::string FormatScalar(const Scalar& value) {
            return std::visit([](const auto& current) -> std::string {
                using Ty = std::decay_t<decltype(current)>;

                if constexpr (std::same_as<Ty, std::int64_t> || std::same_as<Ty, std::uint64_t>) {
                    return std::to_string(current);
                } else if constexpr (std::same_as<Ty, double>) {
                    auto text = std::format("{}", current);
                    if (std::isfinite(current) && text.find_first_of(".eE") == std::string::npos) {
                        text += ".0";
                    }

                    return text;
                } else {
                    return current ? "true" : "false";
                }
            }, value);
        }

        std::string AggregateTypename(const TypeDescriptor& type) {
            std::string prefix;

            switch (type.family) {
            case BaseFamily::kBool:
                prefix = "b";
                break;
            case BaseFamily::kInt:
                prefix = type.bits == 32 ? "i" : std::format("i{}", type.bits);
                break;
            case BaseFamily::kUint:
                prefix = type.bits == 32 ? "u" : std::format("u{}", type.bits);
                break;
            case BaseFamily::kFloat:
                if (type.bits == 64) {
                    prefix = "d";
                } else if (type.bits == 16) {
                    prefix = "h";
                } else if (type.bits != 32) {
                    prefix = std::format("f{}", type.bits);
                }
                break;
            default:
                return {};
            }

            if (type.vector_count == 1 && type.vector_length > 1)
                return std::format("{}vec{}", prefix, type.vector_length);
            if (type.vector_count > 1 && type.vector_length > 1)
                return std::format("{}mat{}x{}", prefix, type.vector_count, type.vector_length);
            return {};
        }
    }

    std::optional<std::string> ConstantEvaluator::FormatValue(const ValueType& value) const {
        if (const auto* scalar = std::get_if<ScalarValue>(&value)) {
            return FormatScalar(*scalar);
        }

        const auto* aggregate = std::get_if<AggregateValue>(&value);
        if (aggregate == nullptr || aggregate->components.size() != ComponentCount(aggregate->type_desc)) {
            return std::nullopt;
        }

        auto type_name = AggregateTypename(aggregate->type_desc);
        if (type_name.empty()) {
            return std::nullopt;
        }

        auto result = std::move(type_name) + "(";

        for (auto i = 0uz; i != aggregate->components.size(); ++i) {
            if (i != 0) {
                result += ", ";
            }

            result += FormatScalar(aggregate->components[i]);
        }

        result += ")";
        return result;
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

        if (symbol == nullptr || symbol->kind != SymbolKind::kVariable ||
            visited_symbols_.contains(symbol) ||
            symbol->node->kind() != AstNodeKind::kVariableDeclaration)
        {
            is_valid_ = false;
            return;
        }

        auto* var_decl = static_cast<const VariableDeclarationNode*>(symbol->node);
        if (!var_decl->type_spec.has_keyword("const") || var_decl->init == nullptr) {
            is_valid_ = false;
            return;
        }

        visited_symbols_.emplace(symbol);
        const auto result = Evaluate(var_decl->init);
        if (result.has_value()) {
            current_value_ = *result;
        } else {
            is_valid_ = false;
        }

        visited_symbols_.erase(symbol);
    }

    void ConstantEvaluator::VisitCastExpression(CastExpressionNode* node) {
        if (node == nullptr || node->operand == nullptr ||
            !node->evaluated_type.is_valid())
        {
            is_valid_ = false;
            return;
        }

        const auto operand = Evaluate(node->operand);
        if (!operand.has_value()) {
            is_valid_ = false;
            return;
        }

        const auto converted = ConvertValueToType(*operand, node->evaluated_type, ConversionMode::kExplicit);
        if (!converted.has_value()) {
            is_valid_ = false;
            return;
        }

        current_value_ = *converted;
    }

    void ConstantEvaluator::VisitBinaryExpression(BinaryExpressionNode* node) {
        if (node->left == nullptr || node->right == nullptr) {
            is_valid_ = false;
            return;
        }

        const auto left_value  = Evaluate(node->left);
        const auto right_value = Evaluate(node->right);

        if (!left_value.has_value() || !right_value.has_value()) {
            is_valid_ = false;
            return;
        }

        const auto* left_result  = GetScalar(*left_value);
        const auto* right_result = GetScalar(*right_value);

        if (left_result == nullptr || right_result == nullptr) {
            is_valid_ = false;
            return;
        }

        auto PromoteArithmetic = [](const auto& lhs, const auto& rhs)
            -> std::optional<std::pair<ScalarValue, ScalarValue>>
        {
            if (std::holds_alternative<bool>(lhs) || std::holds_alternative<bool>(rhs)) {
                return std::nullopt;
            }

            if (lhs.index() == rhs.index()) {
                return std::make_pair(lhs, rhs);
            }

            auto ToDouble = [](const auto& scalar) -> double {
                return std::visit([](auto&& argv) -> double {
                    return static_cast<double>(argv);
                }, scalar);
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
            const auto promoted = PromoteArithmetic(*left_result, *right_result);
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

        const auto value = Evaluate(node->operand);
        if (!value.has_value()) {
            is_valid_ = false;
            return;
        }

        const auto* result = GetScalar(*value);
        if (!result) {
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

    void ConstantEvaluator::VisitCallExpression(CallExpressionNode* node) {
        if (node == nullptr || node->callee == nullptr ||
            node->callee->kind() != AstNodeKind::kVariableExpression)
        {
            is_valid_ = false;
            return;
        }


        auto* callee = static_cast<VariableExpressionNode*>(node->callee);

        if (callee->original_token.type == TokenType::kPrimitive ||
            callee->original_token.type == TokenType::kBuiltInType)
        {
            const auto converted = EvaluateConstructor(node, node->evaluated_type);
            if (!converted.has_value()) {
                is_valid_ = false;
                return;
            }

            current_value_ = *converted;
            return;
        }

        auto* symbol_slot = std::get_if<const SymbolInfo*>(&callee->linked_symbols);

        if (symbol_slot == nullptr || *symbol_slot == nullptr) {
            is_valid_ = false;
            return;
        }

        const auto* symbol = *symbol_slot;
        if (symbol->kind != SymbolKind::kFunctionDecl &&
            symbol->kind != SymbolKind::kFunctionImpl)
        {
            is_valid_ = false;
            return;
        }

        const auto* source_file = symbol->location.source_file();
        if (source_file == nullptr || source_file->kind() != SourceKind::kMetadata) {
            is_valid_ = false;
            return;
        }

        std::vector<ValueType> arguments;
        arguments.reserve(node->args.size());

        for (auto i = 0uz; i != node->args.size(); ++i) {
            if (node->args[i] == nullptr) {
                is_valid_ = false;
                return;
            }

            const auto evaluated = Evaluate(node->args[i]);
            if (!evaluated.has_value()) {
                is_valid_ = false;
                return;
            }

            const auto converted = ConvertValueToType(*evaluated, symbol->param_typeinfos[i], ConversionMode::kImplicit);
            if (!converted.has_value()) {
                is_valid_ = false;
                return;
            }

            arguments.push_back(*converted);
        }

        const auto result = EvaluateBuiltinFunction(callee->name, arguments, symbol->type_info);
        if (!result.has_value()) {
            is_valid_ = false;
            return;
        }

        const auto converted_result = ConvertValueToType(*result, symbol->type_info, ConversionMode::kImplicit);
        if (!converted_result.has_value()) {
            is_valid_ = false;
            return;
        }

        current_value_ = *converted_result;
    }

    void ConstantEvaluator::VisitRawExpression(RawExpressionNode* node) {
        if (node->tokens.size() != 1 || node->tokens.front().type != TokenType::kNumberLiteral) {
            is_valid_ = false;
            return;
        }

        const auto& text = node->tokens.front().text;
        if (text.find_first_of(".eEpPfF") != std::string::npos) {
            double value = 0.0;
            const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
            if (ec == std::errc{}) {
                current_value_ = value;
                return;
            }
        } else if (text.find_first_of("uU") != std::string::npos) {
            std::uint64_t value = 0;
            const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
            if (ec == std::errc{}) {
                current_value_ = value;
                return;
            }
        } else {
            std::int64_t value = 0;
            const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
            if (ec == std::errc{}) {
                current_value_ = value;
                return;
            }
        }

        is_valid_ = false;
    }

    namespace {
        std::optional<std::int64_t> GetStaticLength(const TypeInfo& type) {
            if (type.is_array()) {
                if (type.array_sizes.empty() || !type.array_sizes.front().has_value()) {
                    return std::nullopt;
                }

                const auto size = *type.array_sizes.front();
                if (size > static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max())) {
                    return std::nullopt;
                }

                return static_cast<std::int64_t>(size);
            }

            if (!type.is_valid() || type.block_symbol != nullptr ||
                type.type_desc.family == BaseFamily::kUnknown ||
                type.type_desc.family == BaseFamily::kVoid ||
                type.type_desc.family == BaseFamily::kOpaque)
            {
                return std::nullopt;
            }

            using enum TypeDescriptor::ArithmeticStructure;

            switch (type.type_desc.arithmetic_structure()) {
            case kVector:
                if (type.type_desc.vector_length > 1) {
                    return type.type_desc.vector_length;
                }
                break;
            case kMatrix:
                if (type.type_desc.vector_count > 1) {
                    return type.type_desc.vector_count;
                }
                break;
            case kScalar:
                break;
            }

            return std::nullopt;
        }
    }

    void ConstantEvaluator::VisitMemberAccessExpression(MemberAccessExpressionNode* node) {
        if (FindLengthCall(node) == nullptr || node->object == nullptr) {
            is_valid_ = false;
            return;
        }

        const auto length = GetStaticLength(node->object->evaluated_type);
        if (!length.has_value()) {
            is_valid_ = false;
            return;
        }

        current_value_ = *length;
    }
}
