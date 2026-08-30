#include "pch.hpp"
#include "ConstantEvaluator.hpp"

#include <cmath>
#include <cstddef>
#include <algorithm>
#include <charconv>
#include <concepts>
#include <format>
#include <iterator>
#include <limits>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <utility>

#include <glm/glm.hpp>

#include "Analyzer/Syntax/Token.hpp"
#include "Base/FileSystem/Source.hpp"
#include "Base/MathMeta.hpp"
#include "Utils/Utils.hpp"

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
        using Scalar    = ConstantEvaluator::Scalar;
        using Aggregate = ConstantEvaluator::Aggregate;
        using Value     = ConstantEvaluator::Value;

        const Scalar* GetScalar(const Value& value) {
            return std::get_if<Scalar>(&value);
        }

        const Aggregate* GetAggregate(const Value& value) {
            return std::get_if<Aggregate>(&value);
        }

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
                if (const auto* source = std::get_if<std::int64_t>(&scalar))
                    return *source;
                if (!explicit_conversion)
                    return std::nullopt;
                if (const auto* source = std::get_if<std::uint64_t>(&scalar))
                    return static_cast<std::int64_t>(*source);

                if (const auto* source = std::get_if<double>(&scalar)) {
                    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
                    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();

                    if (!std::isfinite(*source) ||
                        *source < static_cast<double>(kMin) ||
                        *source > static_cast<double>(kMax))
                    {
                        return std::nullopt;
                    }

                    return static_cast<std::int64_t>(*source);
                }

                if (const auto* source = std::get_if<bool>(&scalar)) {
                    return static_cast<std::int64_t>(*source);
                }

                break;

            case BaseFamily::kUint:
                if (const auto* source = std::get_if<std::uint64_t>(&scalar))
                    return *source;
                if (const auto* source = std::get_if<std::int64_t>(&scalar))
                    return static_cast<std::uint64_t>(*source);
                if (!explicit_conversion)
                    return std::nullopt;

                if (const auto* source = std::get_if<double>(&scalar)) {
                    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
                    if (!std::isfinite(*source) ||
                        *source < 0.0 ||
                        *source > static_cast<double>(kMax))
                    {
                        return std::nullopt;
                    }

                    return static_cast<std::uint64_t>(*source);
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
                if (!explicit_conversion)
                    return std::nullopt;
                if (const auto* source = std::get_if<bool>(&scalar))
                    return *source ? 1.0 : 0.0;
                break;

            default:
                break;
            }

            return std::nullopt;
        }

#define ENUMERATE_GLM_VECTOR(prefix) \
    glm::prefix##vec2, glm::prefix##vec3, glm::prefix##vec4

#define ENUMERATE_GLM_MATRIX(prefix) \
    glm::prefix##mat2x2, glm::prefix##mat2x3, glm::prefix##mat2x4, \
    glm::prefix##mat3x2, glm::prefix##mat3x3, glm::prefix##mat3x4, \
    glm::prefix##mat4x2, glm::prefix##mat4x3, glm::prefix##mat4x4

        using GlmAggregate = std::variant<
            ENUMERATE_GLM_VECTOR(i64),
            ENUMERATE_GLM_VECTOR(u64),
            ENUMERATE_GLM_VECTOR(d),
            ENUMERATE_GLM_VECTOR(b),
            ENUMERATE_GLM_MATRIX(i64),
            ENUMERATE_GLM_MATRIX(u64),
            ENUMERATE_GLM_MATRIX(d)
        >;

        template <typename Ty>
        struct GlmShape {
            static constexpr bool is_vector        = false;
            static constexpr bool is_matrix        = false;
            static constexpr glm::length_t columns = 0;
            static constexpr glm::length_t rows    = 0;
        };

        template <glm::length_t Length, typename Ty, glm::qualifier Qualifier>
        struct GlmShape<glm::vec<Length, Ty, Qualifier>> {
            static constexpr bool is_vector        = true;
            static constexpr bool is_matrix        = false;
            static constexpr glm::length_t columns = 1;
            static constexpr glm::length_t rows    = Length;
        };

        template <glm::length_t Columns, glm::length_t Rows, typename Ty, glm::qualifier Qualifier>
        struct GlmShape<glm::mat<Columns, Rows, Ty, Qualifier>> {
            static constexpr bool is_vector        = false;
            static constexpr bool is_matrix        = true;
            static constexpr glm::length_t columns = Columns;
            static constexpr glm::length_t rows    = Rows;
        };

        template <typename Ty>
        concept IsGlmVector = GlmShape<std::remove_cvref_t<Ty>>::is_vector;

        template <typename Ty>
        concept IsGlmMatrix = GlmShape<std::remove_cvref_t<Ty>>::is_matrix;

        template <typename Ty>
        consteval BaseFamily GetGlmBaseFamily() {
            using ComponentType = typename Ty::value_type;
            if constexpr (std::same_as<ComponentType, std::int64_t>) {
                return BaseFamily::kInt;
            } else if constexpr (std::same_as<ComponentType, std::uint64_t>) {
                return BaseFamily::kUint;
            } else if constexpr (std::same_as<ComponentType, double>) {
                return BaseFamily::kFloat;
            } else if constexpr (std::same_as<ComponentType, bool>) {
                return BaseFamily::kBool;
            } else {
                return BaseFamily::kUnknown;
            }
        }

        template <IsGlmVector Ty>
        std::optional<Ty> ToGlmVector(std::span<const Scalar> components) {
            constexpr auto kLength = GlmShape<std::remove_cvref_t<Ty>>::rows;
            constexpr auto kFamily = GetGlmBaseFamily<Ty>();

            if (kFamily == BaseFamily::kUnknown || components.size() != static_cast<std::size_t>(kLength)) {
                return std::nullopt;
            }

            Ty result{};

            for (glm::length_t i = 0; i != kLength; ++i) {
                const auto index     = static_cast<std::size_t>(i);
                const auto converted = ConvertScalarValue(components[index], kFamily, false);

                if (!converted.has_value()) {
                    return std::nullopt;
                }

                result[i] = std::get<typename Ty::value_type>(*converted);
            }

            return result;
        }

        template <IsGlmVector Ty>
        std::optional<GlmAggregate> ToGlmVector(const Aggregate& source) {
            constexpr auto kLength = GlmShape<std::remove_cvref_t<Ty>>::rows;
            if (source.type_desc.vector_count != 1 || source.type_desc.vector_length != kLength) {
                return std::nullopt;
            }

            auto result = ToGlmVector<Ty>(source.components);
            if (!result.has_value()) {
                return std::nullopt;
            }

            return GlmAggregate{ std::move(*result) };
        }

        template <IsGlmMatrix Ty>
        std::optional<GlmAggregate> ToGlmMatrix(const Aggregate& source) {
            constexpr auto kColumns = GlmShape<std::remove_cvref_t<Ty>>::columns;
            constexpr auto kRows    = GlmShape<std::remove_cvref_t<Ty>>::rows;
            if (source.type_desc.vector_count != kColumns || source.type_desc.vector_length != kRows) {
                return std::nullopt;
            }

            Ty result{};

            for (glm::length_t column = 0; column != kColumns; ++column) {
                const auto offset = static_cast<std::size_t>(column * kRows);
                const auto size   = static_cast<std::size_t>(kRows);

                auto component_span = std::span(source.components);
                auto column_value   = ToGlmVector<typename Ty::col_type>(component_span.subspan(offset, size));

                if (!column_value.has_value()) {
                    return std::nullopt;
                }

                result[column] = std::move(*column_value);
            }

            return result;
        }

        template <typename Ty>
        std::optional<GlmAggregate> EnumerateGlmVectorTransform(glm::length_t columns, glm::length_t rows, const Aggregate& source) {
            if (columns != 1) {
                return std::nullopt;
            }

            switch (rows) {
            case 2:
                return ToGlmVector<glm::vec<2, Ty, glm::defaultp>>(source);
            case 3:
                return ToGlmVector<glm::vec<3, Ty, glm::defaultp>>(source);
            case 4:
                return ToGlmVector<glm::vec<4, Ty, glm::defaultp>>(source);
            default:
                return std::nullopt;
            }
        }

        template <typename Ty>
        std::optional<GlmAggregate> EnumerateGlmMatrixTransform(glm::length_t columns, glm::length_t rows, const Aggregate& source) {
            switch (columns * 10 + rows) {
            case 22:
                return ToGlmMatrix<glm::mat<2, 2, Ty, glm::defaultp>>(source);
            case 23:
                return ToGlmMatrix<glm::mat<2, 3, Ty, glm::defaultp>>(source);
            case 24:
                return ToGlmMatrix<glm::mat<2, 4, Ty, glm::defaultp>>(source);
            case 32:
                return ToGlmMatrix<glm::mat<3, 2, Ty, glm::defaultp>>(source);
            case 33:
                return ToGlmMatrix<glm::mat<3, 3, Ty, glm::defaultp>>(source);
            case 34:
                return ToGlmMatrix<glm::mat<3, 4, Ty, glm::defaultp>>(source);
            case 42:
                return ToGlmMatrix<glm::mat<4, 2, Ty, glm::defaultp>>(source);
            case 43:
                return ToGlmMatrix<glm::mat<4, 3, Ty, glm::defaultp>>(source);
            case 44:
                return ToGlmMatrix<glm::mat<4, 4, Ty, glm::defaultp>>(source);
            default:
                return std::nullopt;
            }
        }

        template <typename Ty>
        std::optional<GlmAggregate> EnumerateGlmTransform(glm::length_t columns, glm::length_t rows, const Aggregate& source) {
            if (columns == 1) {
                return EnumerateGlmVectorTransform<Ty>(columns, rows, source);
            } else {
                return EnumerateGlmMatrixTransform<Ty>(columns, rows, source);
            }
        }

        std::optional<GlmAggregate> ToGlmAggregate(const Aggregate& source, BaseFamily target_family) {
            if (target_family == BaseFamily::kUnknown) {
                return std::nullopt;
            }

            const auto columns = source.type_desc.vector_count;
            const auto rows    = source.type_desc.vector_length;

            if (target_family == BaseFamily::kInt) {
                return EnumerateGlmTransform<std::int64_t>(columns, rows, source);
            } else if (target_family == BaseFamily::kUint) {
                return EnumerateGlmTransform<std::uint64_t>(columns, rows, source);
            } else if (target_family == BaseFamily::kFloat) {
                return EnumerateGlmTransform<double>(columns, rows, source);
            } else if (target_family == BaseFamily::kBool) {
                return EnumerateGlmVectorTransform<bool>(columns, rows, source);
            }

            return std::nullopt;
        }

        template <typename Ty>
        std::optional<Scalar> FromGlmComponent(Ty value, BaseFamily family) {
            switch (family) {
            case BaseFamily::kInt:
                return Scalar{ static_cast<std::int64_t>(value) };
            case BaseFamily::kUint:
                return Scalar{ static_cast<std::uint64_t>(value) };
            case BaseFamily::kFloat:
                return Scalar{ static_cast<double>(value) };
            case BaseFamily::kBool:
                return Scalar{ static_cast<bool>(value) };
            default:
                return std::nullopt;
            }
        }

        std::optional<Aggregate> FromGlmAggregate(const GlmAggregate& source, const TypeDescriptor& result_type) {
            return std::visit([&](const auto& glm_value) -> std::optional<Aggregate> {
                using GlmTy = std::remove_cvref_t<decltype(glm_value)>;
                using Shape = GlmShape<GlmTy>;

                if (GetGlmBaseFamily<GlmTy>() != result_type.family) {
                    return std::nullopt;
                }

                if constexpr (Shape::is_vector) {
                    if (result_type.vector_count != 1 || result_type.vector_length != Shape::rows) {
                        return std::nullopt;
                    }

                    Aggregate result{
                        .type_desc  = result_type,
                        .components = {}
                    };

                    result.components.reserve(static_cast<std::size_t>(Shape::rows));

                    for (glm::length_t i = 0; i != Shape::rows; ++i) {
                        const auto component = FromGlmComponent(glm_value[i], result_type.family);
                        if (!component.has_value()) {
                            return std::nullopt;
                        }

                        result.components.emplace_back(*component);
                    }

                    return result;
                } else {
                    if (result_type.vector_count != Shape::columns || result_type.vector_length != Shape::rows) {
                        return std::nullopt;
                    }

                    Aggregate result{
                        .type_desc  = result_type,
                        .components = {}
                    };

                    result.components.reserve(static_cast<std::size_t>(Shape::columns * Shape::rows));

                    for (glm::length_t column = 0; column != Shape::columns; ++column) {
                        for (glm::length_t row = 0; row != Shape::rows; ++row) {
                            const auto component = FromGlmComponent(glm_value[column][row], result_type.family);
                            if (!component.has_value()) {
                                return std::nullopt;
                            }

                            result.components.emplace_back(*component);
                        }
                    }

                    return result;
                }
            }, source);
        }

        template <typename Ty>
        std::optional<GlmAggregate> WrapGlmResult(Ty&& value) {
            using TargetType = std::remove_cvref_t<Ty>;
            if constexpr (std::constructible_from<GlmAggregate, TargetType>) {
                return GlmAggregate{ std::forward<Ty>(value) };
            } else {
                return std::nullopt;
            }
        }

        template <typename Ty>
        concept IsOptional = requires {
            [] <typename Uy> (std::optional<Uy>*) -> void {}(static_cast<std::remove_cvref_t<Ty>*>(nullptr));
        };

        template <typename Ty>
        std::optional<Value> WrapReturnValue(Ty&& value, const TypeInfo& result_type) {
            using ResultType = std::remove_cvref_t<Ty>;

            if constexpr (IsOptional<ResultType>) {
                if (!value.has_value()) {
                    return std::nullopt;
                }

                return WrapReturnValue(*std::forward<Ty>(value), result_type);
            } else if constexpr (IsGlmVector<ResultType> || IsGlmMatrix<ResultType>) {
                const GlmAggregate glm_result{ std::forward<Ty>(value) };
                auto aggregate = FromGlmAggregate(glm_result, result_type.type_desc);
                if (!aggregate.has_value()) {
                    return std::nullopt;
                }

                return Value{ std::move(*aggregate) };
            } else {
                return Value{ std::forward<Ty>(value) };
            }
        }

        template <typename Ty>
        std::optional<Ty> ExtractArgument(const Value& value) {
            using TargetType = std::remove_cvref_t<Ty>;
            if constexpr (IsGlmVector<TargetType> || IsGlmMatrix<TargetType>) {
                const auto* aggregate = GetAggregate(value);
                if (aggregate == nullptr) {
                    return std::nullopt;
                }

                const auto converted = ToGlmAggregate(*aggregate, GetGlmBaseFamily<TargetType>());
                if (!converted.has_value()) {
                    return std::nullopt;
                }

                const auto* result = std::get_if<TargetType>(&*converted);
                if (result == nullptr) {
                    return std::nullopt;
                }

                return *result;
            } else {
                const auto scalar = GetScalar(value);
                if (scalar == nullptr) {
                    return std::nullopt;
                }

                const auto* result = std::get_if<TargetType>(scalar);
                if (result == nullptr) {
                    return std::nullopt;
                }

                return *result;
            }
        }

        template <typename Tuple, std::size_t... Is>
        std::optional<Tuple> ExtractArgs(std::span<const Value> args, std::index_sequence<Is...>) {
            if (args.size() < sizeof...(Is)) {
                return std::nullopt;
            }

            Tuple result{};
            bool ok = (... && ([&]() -> bool {
                using TargetType = std::remove_cvref_t<std::tuple_element_t<Is, Tuple>>;

                const auto argument = ExtractArgument<TargetType>(args[Is]);
                if (!argument.has_value()) {
                    return false;
                }

                std::get<Is>(result) = *argument;
                return true;
            }()));

            if (ok) {
                return result;
            }

            return std::nullopt;
        }

        template <typename Return, typename... Args>
        auto WrapSingleSignature(Return(*func)(Args...)) {
            return [func](std::span<const Value> args, const TypeInfo& result_type) -> std::optional<Value> {
                if (args.size() != sizeof...(Args)) {
                    return std::nullopt;
                }

                using ArgumentTuple  = std::tuple<std::decay_t<Args>...>;
                const auto extracted = ExtractArgs<ArgumentTuple>(args, std::index_sequence_for<Args...>{});
                if (!extracted.has_value()) {
                    return std::nullopt;
                }

                if constexpr (std::same_as<Return, void>) {
                    std::apply(func, *extracted);
                    return std::nullopt;
                } else {
                    auto result = std::apply(func, *extracted);
                    return WrapReturnValue(std::move(result), result_type);
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
                        if (const auto* scalar = GetScalar(argv)) {
                            component_args.emplace_back(*scalar);
                            continue;
                        }

                        const auto* aggregate = GetAggregate(argv);
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

                    const auto* scalar_result = GetScalar(*component_result);
                    if (scalar_result == nullptr) {
                        return std::nullopt;
                    }

                    result.components.push_back(*scalar_result);
                }

                return Value{ std::move(result) };
            };
        }
    }

#define WRAP_TYPED_OVERLOAD(func, type) WrapSingleSignature(func<type>)

#define WRAP_GLM_VECTOR_OVERLOADS(func, prefix)            \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##vec2),          \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##vec3),          \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##vec4)

#define WRAP_GLM_MATRIX_OVERLOADS(func, prefix)            \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat2x2),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat2x3),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat2x4),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat3x2),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat3x3),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat3x4),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat4x2),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat4x3),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat4x4)

#define WRAP_GLM_SQUARE_MATRIX_OVERLOADS(func, prefix)     \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat2x2),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat3x3),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat4x4)

#define REGISTER_FLOAT_OVERLOADS(name, func)               \
    Register(name, MakeOverloader(                         \
        WRAP_TYPED_OVERLOAD(func, double),                 \
        WRAP_GLM_VECTOR_OVERLOADS(func, d)                 \
    ))

#define REGISTER_NUMERIC_OVERLOADS(name, func)             \
    Register(name, MakeOverloader(                         \
        WRAP_TYPED_OVERLOAD(func, std::int64_t),           \
        WRAP_GLM_VECTOR_OVERLOADS(func, i64),              \
        WRAP_TYPED_OVERLOAD(func, std::uint64_t),          \
        WRAP_GLM_VECTOR_OVERLOADS(func, u64),              \
        WRAP_TYPED_OVERLOAD(func, double),                 \
        WRAP_GLM_VECTOR_OVERLOADS(func, d)                 \
    ))

#define REGISTER_INTEGER_OVERLOADS(name, func)             \
    Register(name, MakeOverloader(                         \
        WRAP_TYPED_OVERLOAD(func, std::int64_t),           \
        WRAP_GLM_VECTOR_OVERLOADS(func, i64),              \
        WRAP_TYPED_OVERLOAD(func, std::uint64_t),          \
        WRAP_GLM_VECTOR_OVERLOADS(func, u64)               \
    ))

#define REGISTER_FLOAT_VECTOR_OVERLOADS(name, func)        \
    Register(name, MakeOverloader(                         \
        WRAP_GLM_VECTOR_OVERLOADS(func, d)                 \
    ))

#define REGISTER_NUMERIC_VECTOR_OVERLOADS(name, func)      \
    Register(name, MakeOverloader(                         \
        WRAP_GLM_VECTOR_OVERLOADS(func, i64),              \
        WRAP_GLM_VECTOR_OVERLOADS(func, u64),              \
        WRAP_GLM_VECTOR_OVERLOADS(func, d)                 \
    ))

#define REGISTER_BOOLEAN_OVERLOADS(name, func)             \
    Register(name, MakeOverloader(                         \
        WRAP_TYPED_OVERLOAD(func, bool),                   \
        WRAP_GLM_VECTOR_OVERLOADS(func, b)                 \
    ))

#define REGISTER_BOOLEAN_VECTOR_OVERLOADS(name, func)      \
    Register(name, MakeOverloader(                         \
        WRAP_GLM_VECTOR_OVERLOADS(func, b)                 \
    ))

#define REGISTER_FLOAT_MATRIX_OVERLOADS(name, func)        \
    Register(name, MakeOverloader(                         \
        WRAP_GLM_MATRIX_OVERLOADS(func, d)                 \
    ))

#define REGISTER_FLOAT_SQUARE_MATRIX_OVERLOADS(name, func) \
    Register(name, MakeOverloader(                         \
        WRAP_GLM_SQUARE_MATRIX_OVERLOADS(func, d)          \
    ))

    void ConstantEvaluator::RegisterBuiltins() {
        // Section 8.1 三角函数与角度
        REGISTER_FLOAT_OVERLOADS("radians", MathMeta::Radians);
        REGISTER_FLOAT_OVERLOADS("degrees", MathMeta::Degrees);
        REGISTER_FLOAT_OVERLOADS("sin", MathMeta::Sin);
        REGISTER_FLOAT_OVERLOADS("cos", MathMeta::Cos);
        REGISTER_FLOAT_OVERLOADS("tan", MathMeta::Tan);
        REGISTER_FLOAT_OVERLOADS("asin", MathMeta::Asin);
        REGISTER_FLOAT_OVERLOADS("acos", MathMeta::Acos);
        REGISTER_FLOAT_OVERLOADS("sinh", MathMeta::Sinh);
        REGISTER_FLOAT_OVERLOADS("cosh", MathMeta::Cosh);
        REGISTER_FLOAT_OVERLOADS("tanh", MathMeta::Tanh);
        REGISTER_FLOAT_OVERLOADS("asinh", MathMeta::Asinh);
        REGISTER_FLOAT_OVERLOADS("acosh", MathMeta::Acosh);
        REGISTER_FLOAT_OVERLOADS("atanh", MathMeta::Atanh);

        Register("atan", MakeOverloader(
            WRAP_TYPED_OVERLOAD(MathMeta::Atan, double),
            WRAP_GLM_VECTOR_OVERLOADS(MathMeta::Atan, d),
            WRAP_TYPED_OVERLOAD(MathMeta::Atan2, double),
            WRAP_GLM_VECTOR_OVERLOADS(MathMeta::Atan2, d)
        ));

        // Section 8.2 指数
        REGISTER_FLOAT_OVERLOADS("pow", MathMeta::Pow);
        REGISTER_FLOAT_OVERLOADS("exp", MathMeta::Exp);
        REGISTER_FLOAT_OVERLOADS("exp2", MathMeta::Exp2);
        REGISTER_FLOAT_OVERLOADS("log", MathMeta::Log);
        REGISTER_FLOAT_OVERLOADS("log2", MathMeta::Log2);
        REGISTER_FLOAT_OVERLOADS("sqrt", MathMeta::Sqrt);
        REGISTER_FLOAT_OVERLOADS("inversesqrt", MathMeta::InverseSqrt);

        // Section 8.3 通用函数
        REGISTER_NUMERIC_OVERLOADS("abs", MathMeta::Abs);
        REGISTER_NUMERIC_OVERLOADS("sign", MathMeta::Sign);
        REGISTER_FLOAT_OVERLOADS("floor", MathMeta::Floor);
        REGISTER_FLOAT_OVERLOADS("trunc", MathMeta::Trunc);
        REGISTER_FLOAT_OVERLOADS("round", MathMeta::Round);
        REGISTER_FLOAT_OVERLOADS("roundEven", MathMeta::RoundEven);
        REGISTER_FLOAT_OVERLOADS("ceil", MathMeta::Ceil);
        REGISTER_FLOAT_OVERLOADS("fract", MathMeta::Fract);
        REGISTER_FLOAT_OVERLOADS("mod", MathMeta::Mod);
        REGISTER_NUMERIC_OVERLOADS("min", MathMeta::Min);
        REGISTER_NUMERIC_OVERLOADS("max", MathMeta::Max);
        REGISTER_NUMERIC_OVERLOADS("clamp", MathMeta::Clamp);
        REGISTER_FLOAT_OVERLOADS("step", MathMeta::Step);
        REGISTER_FLOAT_OVERLOADS("smoothstep", MathMeta::SmoothStep);
        REGISTER_FLOAT_OVERLOADS("isnan", MathMeta::IsNan);
        REGISTER_FLOAT_OVERLOADS("isinf", MathMeta::IsInf);
        REGISTER_FLOAT_OVERLOADS("fma", MathMeta::Fma);

        Register("ldexp", MakeOverloader(
            WrapSingleSignature(+[](double value, std::int64_t exponent) -> double { return glm::ldexp(value, static_cast<int>(exponent)); }),
            WrapSingleSignature(+[](glm::dvec2 value, glm::i64vec2 exponent) -> glm::dvec2 { return glm::ldexp(value, glm::ivec2(exponent)); }),
            WrapSingleSignature(+[](glm::dvec3 value, glm::i64vec3 exponent) -> glm::dvec3 { return glm::ldexp(value, glm::ivec3(exponent)); }),
            WrapSingleSignature(+[](glm::dvec4 value, glm::i64vec4 exponent) -> glm::dvec4 { return glm::ldexp(value, glm::ivec4(exponent)); })
        ));

        Register("mix", MakeOverloader(
            WRAP_TYPED_OVERLOAD(MathMeta::Mix, double),
            WRAP_GLM_VECTOR_OVERLOADS(MathMeta::Mix, d),

            WrapSingleSignature(+[](glm::dvec2 lhs, glm::dvec2 rhs, double factor) -> glm::dvec2 { return MathMeta::MixScalarFactor(lhs, rhs, factor); }),
            WrapSingleSignature(+[](glm::dvec3 lhs, glm::dvec3 rhs, double factor) -> glm::dvec3 { return MathMeta::MixScalarFactor(lhs, rhs, factor); }),
            WrapSingleSignature(+[](glm::dvec4 lhs, glm::dvec4 rhs, double factor) -> glm::dvec4 { return MathMeta::MixScalarFactor(lhs, rhs, factor); }),

            WrapSingleSignature(+[](double lhs, double rhs, bool condition) -> double { return MathMeta::MixBool(lhs, rhs, condition); }),
            WrapSingleSignature(+[](glm::dvec2 lhs, glm::dvec2 rhs, glm::bvec2 condition) -> glm::dvec2 { return MathMeta::MixBool(lhs, rhs, condition); }),
            WrapSingleSignature(+[](glm::dvec3 lhs, glm::dvec3 rhs, glm::bvec3 condition) -> glm::dvec3 { return MathMeta::MixBool(lhs, rhs, condition); }),
            WrapSingleSignature(+[](glm::dvec4 lhs, glm::dvec4 rhs, glm::bvec4 condition) -> glm::dvec4 { return MathMeta::MixBool(lhs, rhs, condition); }),

            WrapSingleSignature(+[](std::int64_t lhs, std::int64_t rhs, bool condition) -> std::int64_t { return MathMeta::MixBool(lhs, rhs, condition); }),
            WrapSingleSignature(+[](glm::i64vec2 lhs, glm::i64vec2 rhs, glm::bvec2 condition) -> glm::i64vec2 { return MathMeta::MixBool(lhs, rhs, condition); }),
            WrapSingleSignature(+[](glm::i64vec3 lhs, glm::i64vec3 rhs, glm::bvec3 condition) -> glm::i64vec3 { return MathMeta::MixBool(lhs, rhs, condition); }),
            WrapSingleSignature(+[](glm::i64vec4 lhs, glm::i64vec4 rhs, glm::bvec4 condition) -> glm::i64vec4 { return MathMeta::MixBool(lhs, rhs, condition); }),

            WrapSingleSignature(+[](std::uint64_t lhs, std::uint64_t rhs, bool condition) -> std::uint64_t { return MathMeta::MixBool(lhs, rhs, condition); }),
            WrapSingleSignature(+[](glm::u64vec2 lhs, glm::u64vec2 rhs, glm::bvec2 condition) -> glm::u64vec2 { return MathMeta::MixBool(lhs, rhs, condition); }),
            WrapSingleSignature(+[](glm::u64vec3 lhs, glm::u64vec3 rhs, glm::bvec3 condition) -> glm::u64vec3 { return MathMeta::MixBool(lhs, rhs, condition); }),
            WrapSingleSignature(+[](glm::u64vec4 lhs, glm::u64vec4 rhs, glm::bvec4 condition) -> glm::u64vec4 { return MathMeta::MixBool(lhs, rhs, condition); }),

            WrapSingleSignature(+[](bool lhs, bool rhs, bool condition) -> bool { return MathMeta::MixBool(lhs, rhs, condition); }),
            WrapSingleSignature(+[](glm::bvec2 lhs, glm::bvec2 rhs, glm::bvec2 condition) -> glm::bvec2 { return MathMeta::MixBool(lhs, rhs, condition); }),
            WrapSingleSignature(+[](glm::bvec3 lhs, glm::bvec3 rhs, glm::bvec3 condition) -> glm::bvec3 { return MathMeta::MixBool(lhs, rhs, condition); }),
            WrapSingleSignature(+[](glm::bvec4 lhs, glm::bvec4 rhs, glm::bvec4 condition) -> glm::bvec4 { return MathMeta::MixBool(lhs, rhs, condition); })
        ));

        Register("floatBitsToInt", WrapSingleSignature(+[](double value) -> std::int64_t { return MathMeta::FloatBitsToInt(value); }));
        Register("floatBitsToUint", WrapSingleSignature(+[](double value) -> std::uint64_t { return MathMeta::FloatBitsToUint(value); }));
        Register("intBitsToFloat", WrapSingleSignature(+[](std::int64_t value) -> double { return MathMeta::IntBitsToFloat(value); }));
        Register("uintBitsToFloat", WrapSingleSignature(+[](std::uint64_t value) -> double { return MathMeta::UintBitsToFloat(value); }));

        // Section 8.4 打包解包（无扩展）
        Register("packUnorm2x16", WrapSingleSignature(MathMeta::PackUnorm2x16));
        Register("packSnorm2x16", WrapSingleSignature(MathMeta::PackSnorm2x16));
        Register("packUnorm4x8", WrapSingleSignature(MathMeta::PackUnorm4x8));
        Register("packSnorm4x8", WrapSingleSignature(MathMeta::PackSnorm4x8));
        Register("unpackUnorm2x16", WrapSingleSignature(MathMeta::UnpackUnorm2x16));
        Register("unpackSnorm2x16", WrapSingleSignature(MathMeta::UnpackSnorm2x16));
        Register("unpackUnorm4x8", WrapSingleSignature(MathMeta::UnpackUnorm4x8));
        Register("unpackSnorm4x8", WrapSingleSignature(MathMeta::UnpackSnorm4x8));
        Register("packHalf2x16", WrapSingleSignature(MathMeta::PackHalf2x16));
        Register("unpackHalf2x16", WrapSingleSignature(MathMeta::UnpackHalf2x16));
        Register("packDouble2x32", WrapSingleSignature(MathMeta::PackDouble2x32));
        Register("unpackDouble2x32", WrapSingleSignature(MathMeta::UnpackDouble2x32));

        // Section 8.5 几何函数
        REGISTER_FLOAT_OVERLOADS("length", MathMeta::Length);
        REGISTER_FLOAT_OVERLOADS("distance", MathMeta::Distance);
        REGISTER_FLOAT_OVERLOADS("dot", MathMeta::Dot);
        REGISTER_FLOAT_OVERLOADS("normalize", MathMeta::Normalize);
        REGISTER_FLOAT_OVERLOADS("faceforward", MathMeta::FaceForward);
        REGISTER_FLOAT_OVERLOADS("reflect", MathMeta::Reflect);

        Register("refract", MakeOverloader(
            WrapSingleSignature(+[](double incident, double normal, double eta) -> double { return MathMeta::Refract(incident, normal, eta); }),
            WrapSingleSignature(+[](glm::dvec2 incident, glm::dvec2 normal, double eta) -> glm::dvec2 { return MathMeta::Refract(incident, normal, eta); }),
            WrapSingleSignature(+[](glm::dvec3 incident, glm::dvec3 normal, double eta) -> glm::dvec3 { return MathMeta::Refract(incident, normal, eta); }),
            WrapSingleSignature(+[](glm::dvec4 incident, glm::dvec4 normal, double eta) -> glm::dvec4 { return MathMeta::Refract(incident, normal, eta); })
        ));

        Register("cross", MakeOverloader(
            WrapSingleSignature(+[](glm::dvec3 lhs, glm::dvec3 rhs) -> glm::dvec3 { return MathMeta::Cross(lhs, rhs); })
        ));

        // Section 8.6 矩阵函数
        REGISTER_FLOAT_MATRIX_OVERLOADS("matrixCompMult", MathMeta::MatrixCompMult);
        REGISTER_FLOAT_MATRIX_OVERLOADS("transpose", MathMeta::Transpose);
        REGISTER_FLOAT_SQUARE_MATRIX_OVERLOADS("determinant", MathMeta::Determinant);
        REGISTER_FLOAT_SQUARE_MATRIX_OVERLOADS("inverse", MathMeta::Inverse);

        Register("outerProduct", MakeOverloader(
            WrapSingleSignature(+[](glm::dvec2 column, glm::dvec2 row) -> glm::dmat2x2 { return MathMeta::OuterProduct(column, row); }),
            WrapSingleSignature(+[](glm::dvec3 column, glm::dvec2 row) -> glm::dmat2x3 { return MathMeta::OuterProduct(column, row); }),
            WrapSingleSignature(+[](glm::dvec4 column, glm::dvec2 row) -> glm::dmat2x4 { return MathMeta::OuterProduct(column, row); }),
            WrapSingleSignature(+[](glm::dvec2 column, glm::dvec3 row) -> glm::dmat3x2 { return MathMeta::OuterProduct(column, row); }),
            WrapSingleSignature(+[](glm::dvec3 column, glm::dvec3 row) -> glm::dmat3x3 { return MathMeta::OuterProduct(column, row); }),
            WrapSingleSignature(+[](glm::dvec4 column, glm::dvec3 row) -> glm::dmat3x4 { return MathMeta::OuterProduct(column, row); }),
            WrapSingleSignature(+[](glm::dvec2 column, glm::dvec4 row) -> glm::dmat4x2 { return MathMeta::OuterProduct(column, row); }),
            WrapSingleSignature(+[](glm::dvec3 column, glm::dvec4 row) -> glm::dmat4x3 { return MathMeta::OuterProduct(column, row); }),
            WrapSingleSignature(+[](glm::dvec4 column, glm::dvec4 row) -> glm::dmat4x4 { return MathMeta::OuterProduct(column, row); })
        ));

        // Section 8.7 向量关系
        REGISTER_NUMERIC_VECTOR_OVERLOADS("lessThan", MathMeta::LessThan);
        REGISTER_NUMERIC_VECTOR_OVERLOADS("lessThanEqual", MathMeta::LessThanEqual);
        REGISTER_NUMERIC_VECTOR_OVERLOADS("greaterThan", MathMeta::GreaterThan);
        REGISTER_NUMERIC_VECTOR_OVERLOADS("greaterThanEqual", MathMeta::GreaterThanEqual);
        REGISTER_NUMERIC_VECTOR_OVERLOADS("equal", MathMeta::Equal);
        REGISTER_NUMERIC_VECTOR_OVERLOADS("notEqual", MathMeta::NotEqual);

        REGISTER_BOOLEAN_OVERLOADS("any", MathMeta::Any);
        REGISTER_BOOLEAN_OVERLOADS("all", MathMeta::All);
        REGISTER_BOOLEAN_OVERLOADS("not", MathMeta::Not);

        // Section 8.8 整数函数
        REGISTER_INTEGER_OVERLOADS("bitCount", MathMeta::BitCount);
        REGISTER_INTEGER_OVERLOADS("findLSB", MathMeta::FindLsb);
        REGISTER_INTEGER_OVERLOADS("findMSB", MathMeta::FindMsb);
        REGISTER_INTEGER_OVERLOADS("bitfieldReverse", MathMeta::BitfieldReverse);
        REGISTER_INTEGER_OVERLOADS("bitfieldExtract", MathMeta::BitfieldExtract);
        REGISTER_INTEGER_OVERLOADS("bitfieldInsert", MathMeta::BitfieldInsert);

        // =========================================================================
        // GL_EXT_shader_explicit_arithmetic_types
        // =========================================================================
        Register("halfBitsToInt16", WrapSingleSignature(MathMeta::Float16BitsToInt16));
        Register("float16BitsToInt16", WrapSingleSignature(MathMeta::Float16BitsToInt16));
        Register("halfBitsToUint16", WrapSingleSignature(MathMeta::Float16BitsToUint16));
        Register("float16BitsToUint16", WrapSingleSignature(MathMeta::Float16BitsToUint16));
        Register("int16BitsToHalf", WrapSingleSignature(MathMeta::Int16BitsToFloat16));
        Register("int16BitsToFloat16", WrapSingleSignature(MathMeta::Int16BitsToFloat16));
        Register("uint16BitsToHalf", WrapSingleSignature(MathMeta::Uint16BitsToFloat16));
        Register("uint16BitsToFloat16", WrapSingleSignature(MathMeta::Uint16BitsToFloat16));

        Register("doubleBitsToInt64", WrapSingleSignature(MathMeta::DoubleBitsToInt64));
        Register("doubleBitsToUint64", WrapSingleSignature(MathMeta::DoubleBitsToUint64));
        Register("int64BitsToDouble", WrapSingleSignature(MathMeta::Int64BitsToDouble));
        Register("uint64BitsToDouble", WrapSingleSignature(MathMeta::Uint64BitsToDouble));

        // =========================================================================
        // GL_EXT_shader_explicit_arithmetic_types 打包解包
        // =========================================================================
        Register("packFloat2x16", WrapSingleSignature(MathMeta::PackFloat2x16));
        Register("unpackFloat2x16", WrapSingleSignature(MathMeta::UnpackFloat2x16));

        Register("packInt2x16", WrapSingleSignature(MathMeta::PackInt2x16));
        Register("unpackInt2x16", WrapSingleSignature(MathMeta::UnpackInt2x16));
        Register("packUint2x16", WrapSingleSignature(MathMeta::PackUint2x16));
        Register("unpackUint2x16", WrapSingleSignature(MathMeta::UnpackUint2x16));

        Register("packInt4x16", WrapSingleSignature(MathMeta::PackInt4x16));
        Register("unpackInt4x16", WrapSingleSignature(MathMeta::UnpackInt4x16));
        Register("packUint4x16", WrapSingleSignature(MathMeta::PackUint4x16));
        Register("unpackUint4x16", WrapSingleSignature(MathMeta::UnpackUint4x16));

        Register("packInt2x32", WrapSingleSignature(MathMeta::PackInt2x32));
        Register("unpackInt2x32", WrapSingleSignature(MathMeta::UnpackInt2x32));
        Register("packUint2x32", WrapSingleSignature(MathMeta::PackUint2x32));
        Register("unpackUint2x32", WrapSingleSignature(MathMeta::UnpackUint2x32));
    }

    std::optional<ConstantEvaluator::Value> ConstantEvaluator::Evaluate(ExpressionNode* node) {
        if (node == nullptr) {
            return std::nullopt;
        }

        Traverse(node);

        if (is_valid_) {
            return current_value_;
        }

        return std::nullopt;
    }

    std::optional<ConstantEvaluator::Value> ConstantEvaluator::ConvertValueToType(
        const Value& value,
        const TypeInfo& target_type,
        ConversionMode mode) const
    {
        if (!target_type.is_valid())
            return std::nullopt;
        if (target_type.is_array())
            return ForwardArrayChecked(value, target_type);
        if (target_type.block_symbol != nullptr)
            return ForwardStructChecked(value, target_type);

        bool explicit_conversion = mode == ConversionMode::kExplicit;
        const auto& target_desc = target_type.type_desc;

        // scalar
        if (!IsAggregateType(target_desc)) {
            const auto* scalar = GetScalar(value);
            if (scalar == nullptr) {
                return std::nullopt;
            }

            return ConvertScalarValue(*scalar, target_desc.family, explicit_conversion);
        }

        // vector and matrix
        const auto* aggregate = GetAggregate(value);
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

    namespace {
        using Array     = ConstantEvaluator::Array;
        using ArrayPtr  = ConstantEvaluator::ArrayPtr;
        using Struct    = ConstantEvaluator::Struct;
        using StructPtr = ConstantEvaluator::StructPtr;

        const Array* GetArray(const Value& value) {
            const auto* array_ptr = std::get_if<ArrayPtr>(&value);
            return array_ptr != nullptr ? array_ptr->get() : nullptr;
        }

        const Struct* GetStruct(const Value& value) {
            const auto* struct_ptr = std::get_if<StructPtr>(&value);
            return struct_ptr != nullptr ? struct_ptr->get() : nullptr;
        }

        TypeInfo StripOuterArrayDimension(const TypeInfo& array_type) {
            auto result = array_type;
            if (!result.array_sizes.empty()) {
                result.array_sizes = result.array_sizes.subspan(1);
            }

            return result;
        }
    }

    std::optional<Value> ConstantEvaluator::ForwardArrayChecked(
        const Value& value,
        const TypeInfo& target_type) const
    {
        const auto* source = GetArray(value);
        if (source == nullptr ||
            target_type.array_sizes.empty() ||
            !target_type.array_sizes.front().has_value())
        {
            return std::nullopt;
        }

        if (!source->type_info.CompareWithoutQualifiers(target_type)) {
            return std::nullopt;
        }

        return value;
    }

    std::optional<Value> ConstantEvaluator::ForwardStructChecked(
        const Value& value,
        const TypeInfo& target_type) const
    {
        if (target_type.block_symbol->kind != SymbolKind::kStruct) {
            return std::nullopt;
        }

        const auto* source = GetStruct(value);
        if (source == nullptr ||
            source->type_info.block_symbol != target_type.block_symbol)
        {
            return std::nullopt;
        }

        return value;
    }

    std::optional<ConstantEvaluator::Value> ConstantEvaluator::EvaluateBuiltinFunction(
        std::string_view name,
        std::span<const Value> args,
        const TypeInfo& result_type)
    {
        auto it = registry_.find(name);
        if (it != registry_.end()) {
            return it->second(args, result_type);
        }

        return std::nullopt;
    }

    std::optional<Value> ConstantEvaluator::EvaluateAggregateElements(
        std::span<ExpressionNode* const> elements,
        const TypeInfo& target_type)
    {
        const auto& target_desc = target_type.type_desc;
        const bool  is_matrix   = target_desc.vector_count > 1;

        const auto expected_size = static_cast<std::size_t>(
            is_matrix ? target_desc.vector_count : target_desc.vector_length);

        if (elements.size() != expected_size) {
            return std::nullopt;
        }

        auto element_type = target_type;
        element_type.type_desc.vector_count = 1;
        if (!is_matrix) {
            element_type.type_desc.vector_length = 1;
        }

        Aggregate result{
            .type_desc  = target_desc,
            .components = {}
        };

        result.components.reserve(ComponentCount(target_desc));

        for (auto* element : elements) {
            const auto evaluated = Evaluate(element);
            if (!evaluated) {
                return std::nullopt;
            }

            auto converted = ConvertValueToType(*evaluated, element_type, ConversionMode::kImplicit);
            if (!converted.has_value()) {
                return std::nullopt;
            }

            if (is_matrix) {
                auto* column = GetAggregate(*converted);
                if (column == nullptr) {
                    return std::nullopt;
                }

                result.components.append_range(column->components | std::views::as_rvalue);
            } else {
                const auto* component = GetScalar(*converted);
                if (component == nullptr) {
                    return std::nullopt;
                }

                result.components.push_back(*component);
            }
        }

        return result;
    }

    std::optional<Value> ConstantEvaluator::EvaluateArrayElements(
        std::span<ExpressionNode* const> elements,
        const TypeInfo& target_type)
    {
        if (!target_type.is_valid() ||
            !target_type.is_array() ||
            target_type.array_sizes.empty() ||
            !target_type.array_sizes.front().has_value())
        {
            return std::nullopt;
        }

        const auto expected_size = *target_type.array_sizes.front();
        if (elements.size() != expected_size) {
            return std::nullopt;
        }

        const auto element_type = StripOuterArrayDimension(target_type);

        auto result = std::make_shared<Array>();
        result->type_info = target_type;
        result->elements.reserve(elements.size());

        for (auto* element : elements) {
            if (element == nullptr) {
                return std::nullopt;
            }

            const auto evaluated = Evaluate(element);
            if (!evaluated.has_value()) {
                return std::nullopt;
            }

            auto converted = ConvertValueToType(*evaluated, element_type, ConversionMode::kImplicit);
            if (!converted.has_value()) {
                return std::nullopt;
            }

            result->elements.push_back(std::move(*converted));
        }

        return ArrayPtr{ std::move(result) };
    }

    std::optional<Value> ConstantEvaluator::EvaluateStructElements(
        std::span<ExpressionNode* const> elements,
        const TypeInfo& target_type)
    {
        const auto fields = Utils::CollectStructFieldsOrdered(target_type.block_symbol);
        if (!fields.has_value() || elements.size() != fields->size()) {
            return std::nullopt;
        }

        auto result = std::make_shared<Struct>();
        result->type_info = target_type;
        result->fields.reserve(fields->size());

        for (auto i = 0uz; i != fields->size(); ++i) {
            // recursively evaluate each field
            const auto evaluated = Evaluate(elements[i]);
            if (!evaluated.has_value()) {
                return std::nullopt;
            }

            const auto* field = (*fields)[i];

            auto converted = ConvertValueToType(*evaluated, field->type_info, ConversionMode::kImplicit);
            if (!converted.has_value()) {
                return std::nullopt;
            }

            result->fields.emplace_back(field, std::move(*converted));
        }

        return StructPtr{ std::move(result) };
    }

    std::optional<ConstantEvaluator::Value> ConstantEvaluator::EvaluateConstructor(
        CallExpressionNode* node,
        const TypeInfo& target_type)
    {
        if (node == nullptr ||
            !target_type.is_valid() ||
            target_type.is_array())
        {
            return std::nullopt;
        }

        // struct constructor
        if (target_type.block_symbol != nullptr) {
            return EvaluateStructElements(node->args, target_type);
        }

        // builtin constructor
        std::vector<Value> args;
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
        // int(...), float(...), etc.
        if (!IsAggregateType(target_desc)) {
            if (args.size() != 1) {
                return std::nullopt;
            }

            return ConvertValueToType(args.front(), target_type, ConversionMode::kExplicit);
        }

        // vector and matrix
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

        // mat4(1.0), 初始化对角线元素
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
            const auto* source = GetAggregate(args.front());
            if (source != nullptr &&
                source->type_desc.vector_count  > 1 &&
                source->type_desc.vector_length > 1)
            {
                Aggregate result{
                    .type_desc  = target_desc,
                    .components = std::vector<Scalar>(target_count, Scalar{ 0.0 })
                };

                // 扩展矩阵时，新增的对角线分量为 1.0。原矩阵放在左上角
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

        // 收集 vec4(1, vec2(2, 3), 4) 或 mat4(vec4(...), vec4(...)) 的分量
        std::vector<Scalar> flattened;
        flattened.reserve(target_count);

        for (auto i = 0uz; i != args.size(); ++i) {
            const auto& argv = args[i];
            if (const auto* scalar = GetScalar(argv)) {
                flattened.push_back(*scalar);
                continue;
            }

            const auto* aggregate = GetAggregate(argv);
            // 矩阵构造函数不允许嵌套多个矩阵构造
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
            // 仅允许最后一个向量提供多余分量，如 vec3(float, vec4(...))，不允许 vec3(vec4(...), float)
            const auto* last = GetAggregate(args.back());
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
                using Ty = std::remove_cvref_t<decltype(current)>;
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

    std::optional<std::string> ConstantEvaluator::FormatValue(const Value& value) const {
        if (const auto* scalar = GetScalar(value)) {
            return FormatScalar(*scalar);
        }

        if (const auto* array = GetArray(value)) {
            if (!array->type_info.is_array() || array->type_info.array_sizes.empty()) {
                return std::nullopt;
            }

            auto type_name = std::string(array->type_info.typename_token.text);

            for (const auto& size : array->type_info.array_sizes) {
                if (size.has_value()) {
                    std::format_to(std::back_inserter(type_name), "[{}]", *size);
                } else {
                    type_name += "[]";
                }
            }

            auto result = std::move(type_name) + "(";

            for (auto i = 0uz; i != array->elements.size(); ++i) {
                if (i != 0) {
                    result += ", ";
                }

                const auto formatted = FormatValue(array->elements[i]);
                if (!formatted.has_value()) {
                    return std::nullopt;
                }

                result += *formatted;
            }

            result += ")";
            return result;
        }

        if (const auto* object = GetStruct(value)) {
            const auto* struct_symbol = object->type_info.block_symbol;
            if (struct_symbol == nullptr || struct_symbol->kind != SymbolKind::kStruct) {
                return std::nullopt;
            }

            const auto declared_fields = Utils::CollectStructFieldsOrdered(struct_symbol);
            if (!declared_fields.has_value() || declared_fields->size() != object->fields.size()) {
                return std::nullopt;
            }

            std::string result(struct_symbol->name);
            result += "(";

            for (auto i = 0uz; i != object->fields.size(); ++i) {
                if (object->fields[i].symbol != (*declared_fields)[i]) {
                    return std::nullopt;
                }

                if (i != 0) {
                    result += ", ";
                }

                const auto formatted = FormatValue(object->fields[i].value);
                if (!formatted.has_value()) {
                    return std::nullopt;
                }

                result += *formatted;
            }

            result += ")";
            return result;
        }

        const auto* aggregate = GetAggregate(value);
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

        const auto evaluated = Evaluate(var_decl->init);
        if (!evaluated.has_value()) {
            visited_symbols_.erase(symbol);
            is_valid_ = false;
            return;
        }

        auto converted = ConvertValueToType(*evaluated, symbol->type_info, ConversionMode::kImplicit);
        if (!converted.has_value()) {
            visited_symbols_.erase(symbol);
            is_valid_ = false;
            return;
        }

        current_value_ = std::move(*converted);
        visited_symbols_.erase(symbol);
    }

    void ConstantEvaluator::VisitInitializerListExpression(InitializerListExpressionNode* node) {
        if (node == nullptr) {
            is_valid_ = false;
            return;
        }

        std::optional<Value> result;

        if (node->evaluated_type.is_array()) {
            result = EvaluateArrayElements(node->elements, node->evaluated_type);
        } else if (node->evaluated_type.block_symbol != nullptr) {
            result = EvaluateStructElements(node->elements, node->evaluated_type);
        } else if (IsAggregateType(node->evaluated_type.type_desc)) {
            result = EvaluateAggregateElements(node->elements, node->evaluated_type);
        }

        if (!result.has_value()) {
            is_valid_ = false;
            return;
        }

        current_value_ = std::move(*result);
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

        auto converted = ConvertValueToType(*operand, node->evaluated_type, ConversionMode::kExplicit);
        if (!converted.has_value()) {
            is_valid_ = false;
            return;
        }

        current_value_ = std::move(*converted);
    }

    namespace {
        template <typename Ty>
        struct GlmComponentType {
            using Type = std::remove_cvref_t<Ty>;
        };

        template <glm::length_t Length, typename Ty, glm::qualifier Qualifier>
        struct GlmComponentType<glm::vec<Length, Ty, Qualifier>> {
            using Type = Ty;
        };

        template <glm::length_t Columns, glm::length_t Rows, typename Ty, glm::qualifier Qualifier>
        struct GlmComponentType<glm::mat<Columns, Rows, Ty, Qualifier>> {
            using Type = Ty;
        };

        template <typename Ty>
        using GlmComponentTypeT = typename GlmComponentType<std::remove_cvref_t<Ty>>::Type;

        template <typename Ty>
        concept IsGlmAggregate = IsGlmVector<std::remove_cvref_t<Ty>> || IsGlmMatrix<std::remove_cvref_t<Ty>>;

        template <typename Lhs, typename Rhs>
        consteval bool IsValidGlmComponentWiseOperation() {
            using LeftType  = std::remove_cvref_t<Lhs>;
            using RightType = std::remove_cvref_t<Rhs>;

            using LeftComponent  = GlmComponentTypeT<LeftType>;
            using RightComponent = GlmComponentTypeT<RightType>;

            if constexpr (!std::same_as<LeftComponent, RightComponent> || std::same_as<LeftComponent, bool>) {
                return false;
            } else if constexpr (std::is_arithmetic_v<LeftType> && IsGlmAggregate<RightType>) {
                return true;
            } else if constexpr (IsGlmAggregate<LeftType> && std::is_arithmetic_v<RightType>) {
                return true;
            } else {
                return std::same_as<LeftType, RightType>;
            }
        }

        template <typename Lhs, typename Rhs>
        consteval bool IsValidGlmMultiplication() {
            using LeftType  = std::remove_cvref_t<Lhs>;
            using RightType = std::remove_cvref_t<Rhs>;

            using LeftComponent  = GlmComponentTypeT<LeftType>;
            using RightComponent = GlmComponentTypeT<RightType>;

            if constexpr (!std::same_as<LeftComponent, RightComponent> || std::same_as<LeftComponent, bool>) {
                return false;
            } else if constexpr (std::is_arithmetic_v<LeftType> && IsGlmAggregate<RightType>) {
                return true;
            } else if constexpr (IsGlmAggregate<LeftType> && std::is_arithmetic_v<RightType>) {
                return true;
            } else if constexpr (IsGlmVector<LeftType> && IsGlmVector<RightType>) {
                // 向量乘法是按分量运算
                return std::same_as<LeftType, RightType>;
            } else if constexpr (IsGlmMatrix<LeftType> && IsGlmMatrix<RightType>) {
                // GLM 的矩阵乘法最终会调用 dot，而 GLM 限制 dot 为浮点
                return std::floating_point<LeftComponent> && GlmShape<LeftType>::columns == GlmShape<RightType>::rows;
            } else if constexpr (IsGlmMatrix<LeftType> && IsGlmVector<RightType>) {
                return std::floating_point<LeftComponent> && GlmShape<LeftType>::columns == GlmShape<RightType>::rows;
            } else if constexpr (IsGlmVector<LeftType> && IsGlmMatrix<RightType>) {
                return std::floating_point<LeftComponent> && GlmShape<LeftType>::rows == GlmShape<RightType>::rows;
            } else {
                return false;
            }
        }

        template <typename Lhs, typename Rhs>
        std::optional<GlmAggregate> EvaluateGlmBinaryImpl(const Lhs& lhs, const Rhs& rhs, TokenType op) {
            using LeftType  = std::remove_cvref_t<Lhs>;
            using RightType = std::remove_cvref_t<Rhs>;

            using LeftComponent  = GlmComponentTypeT<LeftType>;
            using RightComponent = GlmComponentTypeT<RightType>;

            if constexpr (!std::same_as<LeftComponent, RightComponent> || std::same_as<LeftComponent, bool>) {
                return std::nullopt;
            } else {
                switch (op) {
                case TokenType::kPlus:
                    if constexpr (
                        IsValidGlmComponentWiseOperation<LeftType, RightType>()) {
                        if constexpr (requires { lhs + rhs; }) {
                            return WrapGlmResult(lhs + rhs);
                        }
                    }

                    break;

                case TokenType::kMinus:
                    if constexpr (IsValidGlmComponentWiseOperation<LeftType, RightType>()) {
                        if constexpr (requires { lhs - rhs; }) {
                            return WrapGlmResult(lhs - rhs);
                        }
                    }

                    break;

                case TokenType::kStar:
                    if constexpr (
                        IsValidGlmMultiplication<LeftType, RightType>()) {
                        // 整数 matrix/vector 组合已经被排除，不会进入 GLM 的浮点 dot
                        return WrapGlmResult(lhs * rhs);
                    }

                    break;

                case TokenType::kSlash: // GLSL 矩阵除法是逐分量计算，而 GLM 是 A * glm::inverse(B)
                    if constexpr (IsValidGlmComponentWiseOperation<LeftType, RightType>()) {
                        if constexpr (IsGlmMatrix<LeftType> && IsGlmMatrix<RightType> && std::same_as<LeftType, RightType>) {
                            LeftType result{};
                            for (glm::length_t column = 0; column != GlmShape<LeftType>::columns; ++column) {
                                result[column] = lhs[column] / rhs[column];
                            }

                            return WrapGlmResult(std::move(result));
                        } else if constexpr (std::is_arithmetic_v<LeftType> && IsGlmMatrix<RightType>) {
                            RightType result{};
                            for (glm::length_t column = 0; column != GlmShape<RightType>::columns; ++column) {
                                result[column] = lhs / rhs[column];
                            }

                            return WrapGlmResult(std::move(result));
                        } else if constexpr (IsGlmMatrix<LeftType> && std::is_arithmetic_v<RightType>) {
                            LeftType result{};
                            for (glm::length_t column = 0; column != GlmShape<LeftType>::columns; ++column) {
                                result[column] = lhs[column] / rhs;
                            }

                            return WrapGlmResult(std::move(result));
                        } else if constexpr (requires { lhs / rhs; }) {
                            return WrapGlmResult(lhs / rhs);
                        }
                    }

                    break;

                default:
                    break;
                }

                return std::nullopt;
            }
        }

        std::optional<GlmAggregate> EvaluateGlmBinary(const Value& lhs, const Value& rhs, TokenType op, BaseFamily result_family) {
            const auto* lhs_aggregate = GetAggregate(lhs);
            const auto* rhs_aggregate = GetAggregate(rhs);

            // 向量/矩阵混合二元运算
            if (lhs_aggregate != nullptr && rhs_aggregate != nullptr) {
                const auto glm_lhs = ToGlmAggregate(*lhs_aggregate, result_family);
                const auto glm_rhs = ToGlmAggregate(*rhs_aggregate, result_family);

                if (!glm_lhs.has_value() || !glm_rhs.has_value()) {
                    return std::nullopt;
                }

                return std::visit([op](const auto& left, const auto& right) -> std::optional<GlmAggregate> {
                    return EvaluateGlmBinaryImpl(left, right, op);
                }, *glm_lhs, *glm_rhs);
            }

            const auto* lhs_scalar = GetScalar(lhs);
            const auto* rhs_scalar = GetScalar(rhs);

            // 向量/矩阵与标量混合二元运算
            if (lhs_aggregate != nullptr && rhs_scalar != nullptr) {
                const auto glm_lhs = ToGlmAggregate(*lhs_aggregate, result_family);
                if (!glm_lhs.has_value()) {
                    return std::nullopt;
                }

                return std::visit([&](const auto& left) -> std::optional<GlmAggregate> {
                    using AggregateType     = std::remove_cvref_t<decltype(left)>;
                    using ComponentType     = typename AggregateType::value_type;
                    constexpr auto kFamily  = GetGlmBaseFamily<AggregateType>();

                    const auto converted = ConvertScalarValue(*rhs_scalar, kFamily, false);
                    if (!converted.has_value()) {
                        return std::nullopt;
                    }

                    const auto* component = std::get_if<ComponentType>(&*converted);
                    if (component == nullptr) {
                        return std::nullopt;
                    }

                    return EvaluateGlmBinaryImpl(left, *component, op);
                }, *glm_lhs);
            }

            // 标量与向量/矩阵混合二元运算
            if (lhs_scalar != nullptr && rhs_aggregate != nullptr) {
                const auto glm_rhs = ToGlmAggregate(*rhs_aggregate, result_family);
                if (!glm_rhs.has_value()) {
                    return std::nullopt;
                }

                return std::visit([&](const auto& right) -> std::optional<GlmAggregate> {
                    using AggregateType    = std::remove_cvref_t<decltype(right)>;
                    using ComponentType    = typename AggregateType::value_type;
                    constexpr auto kFamily = GetGlmBaseFamily<AggregateType>();

                    const auto converted = ConvertScalarValue(*lhs_scalar, kFamily, false);
                    if (!converted.has_value()) {
                        return std::nullopt;
                    }

                    const auto* component = std::get_if<ComponentType>(&*converted);
                    if (component == nullptr) {
                        return std::nullopt;
                    }

                    return EvaluateGlmBinaryImpl(*component, right, op);
                }, *glm_rhs);
            }

            return std::nullopt;
        }
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

        const auto* left_scalar  = GetScalar(*left_value);
        const auto* right_scalar = GetScalar(*right_value);

        // 双聚合类型或者有任意一个不是标量
        if (left_scalar == nullptr || right_scalar == nullptr) {
            const auto glm_result =
                EvaluateGlmBinary(*left_value, *right_value, node->op, node->evaluated_type.type_desc.family);

            if (!glm_result.has_value()) {
                is_valid_ = false;
                return;
            }

            auto aggregate_result = FromGlmAggregate(*glm_result, node->evaluated_type.type_desc);
            if (!aggregate_result.has_value()) {
                is_valid_ = false;
                return;
            }

            current_value_ = std::move(*aggregate_result);
            return;
        }

        auto PromoteArithmetic = [](const auto& lhs, const auto& rhs)
            -> std::optional<std::pair<Scalar, Scalar>>
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
            const auto promoted = PromoteArithmetic(*left_scalar, *right_scalar);
            if (!promoted.has_value()) {
                is_valid_ = false;
                return;
            }

            const auto& [promoted_left, promoted_right] = *promoted;
            std::visit([this, op = node->op](auto&& lhs, auto&& rhs) -> void {
                using LeftType  = std::remove_cvref_t<decltype(lhs)>;
                using RightType = std::remove_cvref_t<decltype(rhs)>;
                if constexpr (!std::same_as<LeftType, bool> && !std::same_as<RightType, bool>) {
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
            if (left_scalar->index() != right_scalar->index() ||
                std::holds_alternative<bool>(*left_scalar) ||
                std::holds_alternative<double>(*left_scalar))
            {
                is_valid_ = false;
                return;
            }

            std::visit([this](auto&& lhs, auto&& rhs) -> void {
                using LeftType  = std::remove_cvref_t<decltype(lhs)>;
                using RightType = std::remove_cvref_t<decltype(rhs)>;
                if constexpr (std::is_integral_v<LeftType> && !std::same_as<LeftType, bool> && std::same_as<LeftType, RightType>) {
                    if (rhs == 0) {
                        is_valid_ = false;
                    } else {
                        current_value_ = lhs % rhs;
                    }
                } else {
                    is_valid_ = false;
                }
            }, *left_scalar, *right_scalar);

            break;

        case TokenType::kAmpersand:
        case TokenType::kVerticalBar:
        case TokenType::kCaret:
        case TokenType::kLeftShift:
        case TokenType::kRightShift: {
            if (left_scalar->index() != right_scalar->index() ||
                std::holds_alternative<bool>(*left_scalar) ||
                std::holds_alternative<double>(*left_scalar))
            {
                is_valid_ = false;
                return;
            }

            std::visit([this, op = node->op](auto&& lhs, auto&& rhs) -> void {
                using LeftType  = std::remove_cvref_t<decltype(lhs)>;
                using RightType = std::remove_cvref_t<decltype(rhs)>;
                if constexpr (std::is_integral_v<LeftType> && !std::same_as<LeftType, bool> && std::same_as<LeftType, RightType>) {
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
            }, * left_scalar, * right_scalar);

            break;
        }

        default:
            is_valid_ = false;
            break;
        }
    }

    namespace {
        template <typename Ty>
        std::optional<GlmAggregate> EvaluateGlmUnaryImpl(const Ty& value, TokenType op) {
            using ValueType     = std::remove_cvref_t<Ty>;
            using ComponentType = GlmComponentTypeT<ValueType>;

            if constexpr (std::same_as<ComponentType, bool>) {
                return std::nullopt;
            } else {
                switch (op) {
                case TokenType::kPlus:
                    return WrapGlmResult(value);

                case TokenType::kMinus:
                    if constexpr ((std::is_integral_v<ComponentType> && std::is_signed_v<ComponentType>) || std::is_floating_point_v<ComponentType>) {
                        return WrapGlmResult(-value);
                    } else {
                        // u64vec/u64mat 不允许进入 GLM 的一元负号
                        return std::nullopt;
                    }

                default:
                    return std::nullopt;
                }
            }
        }

        std::optional<GlmAggregate> EvaluateGlmUnary(const Aggregate& value, TokenType op) {
            const auto glm_value = ToGlmAggregate(value, value.type_desc.family);
            if (!glm_value.has_value()) {
                return std::nullopt;
            }

            return std::visit([op](const auto& current) -> std::optional<GlmAggregate> {
                return EvaluateGlmUnaryImpl(current, op);
            }, *glm_value);
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

        const auto* scalar = GetScalar(*value);
        if (scalar == nullptr) { // 聚合类型
            const auto* aggregate = GetAggregate(*value);
            if (aggregate == nullptr) {
                is_valid_ = false;
                return;
            }

            const auto glm_result = EvaluateGlmUnary(*aggregate, node->op);
            if (!glm_result.has_value()) {
                is_valid_ = false;
                return;
            }

            auto converted = FromGlmAggregate(*glm_result, node->evaluated_type.type_desc);
            if (!converted.has_value()) {
                is_valid_ = false;
                return;
            }

            current_value_ = std::move(*converted);
            return;
        }

        switch (node->op) {
        case TokenType::kMinus:
            if (std::holds_alternative<bool>(*scalar)) {
                is_valid_ = false;
            } else {
                std::visit([this](auto&& value) -> void {
                    using Ty = std::remove_cvref_t<decltype(value)>;
                    if constexpr ((std::is_integral_v<Ty> && std::is_signed_v<Ty>) || std::is_floating_point_v<Ty>) {
                        current_value_ = -value;
                    } else {
                        is_valid_ = false;
                    }
                }, *scalar);
            }

            break;

        case TokenType::kPlus:
            current_value_ = *scalar;
            break;

        case TokenType::kExclamation:
            if (!std::holds_alternative<bool>(*scalar)) {
                is_valid_ = false;
            } else {
                current_value_ = !std::get<bool>(*scalar);
            }

            break;

        case TokenType::kTilde:
            if (std::holds_alternative<bool>(*scalar) || std::holds_alternative<double>(*scalar)) {
                is_valid_ = false;
            } else {
                std::visit([this](auto&& value) -> void {
                    using Ty = std::remove_cvref_t<decltype(value)>;
                    if constexpr (std::is_integral_v<Ty> && !std::same_as<Ty, bool>) {
                        current_value_ = ~value;
                    } else {
                        is_valid_ = false;
                    }
                }, *scalar);
            }

            break;

        default:
            is_valid_ = false;
            break;
        }
    }

    void ConstantEvaluator::VisitCallExpression(CallExpressionNode* node) {
        if (node == nullptr || node->callee == nullptr) {
            is_valid_ = false;
            return;
        }

        // array constructor like float array = float[3](1.0, 2.0, 3.0)
        if (node->evaluated_type.is_array() &&
            node->callee->kind() == AstNodeKind::kIndexExpression)
        {
            auto result = EvaluateArrayElements(node->args, node->evaluated_type);
            if (!result.has_value()) {
                is_valid_ = false;
                return;
            }

            current_value_ = std::move(*result);
            return;
        }

        if (node->callee->kind() != AstNodeKind::kVariableExpression) {
            is_valid_ = false;
            return;
        }

        auto* callee = static_cast<VariableExpressionNode*>(node->callee);

        const bool builtin_construct =
            callee->original_token.type == TokenType::kPrimitive ||
            callee->original_token.type == TokenType::kBuiltInType;

        auto* symbol_slot = std::get_if<const SymbolInfo*>(&callee->linked_symbols);

        const bool struct_construct =
            symbol_slot != nullptr && *symbol_slot != nullptr && (*symbol_slot)->kind == SymbolKind::kStruct;

        if (builtin_construct || struct_construct) {
            auto converted = EvaluateConstructor(node, node->evaluated_type);
            if (!converted.has_value()) {
                is_valid_ = false;
                return;
            }

            current_value_ = std::move(*converted);
            return;
        }

        if (symbol_slot == nullptr || *symbol_slot == nullptr) {
            is_valid_ = false;
            return;
        }

        const auto* symbol = *symbol_slot;
        if (symbol->kind != SymbolKind::kFunctionDecl && symbol->kind != SymbolKind::kFunctionImpl) {
            is_valid_ = false;
            return;
        }

        const auto* source_file = symbol->location.source_file();
        if (source_file == nullptr || source_file->kind() != SourceKind::kMetadata) {
            is_valid_ = false;
            return;
        }

        // collect arguments and evaluate them
        std::vector<Value> args;
        args.reserve(node->args.size());

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

            args.push_back(*converted);
        }

        const auto result = EvaluateBuiltinFunction(callee->name, args, symbol->type_info);
        if (!result.has_value()) {
            is_valid_ = false;
            return;
        }

        auto converted_result = ConvertValueToType(*result, symbol->type_info, ConversionMode::kImplicit);
        if (!converted_result.has_value()) {
            is_valid_ = false;
            return;
        }

        current_value_ = std::move(*converted_result);
    }

    namespace {
        std::optional<std::size_t> GetIndex(const Scalar* index_scalar) {
            std::optional<std::size_t> index;
            if (const auto* signed_index = std::get_if<std::int64_t>(index_scalar)) {
                if (*signed_index >= 0) {
                    index = static_cast<std::size_t>(*signed_index);
                }
            } else if (const auto* unsigned_index = std::get_if<std::uint64_t>(index_scalar)) {
                if (*unsigned_index <= static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
                    index = static_cast<std::size_t>(*unsigned_index);
                }
            }

            return index;
        }
    }

    void ConstantEvaluator::VisitIndexExpression(IndexExpressionNode* node) {
        if (node == nullptr || node->base == nullptr || node->index == nullptr) {
            is_valid_ = false;
            return;
        }

        const auto base_value = Evaluate(node->base);
        if (!base_value.has_value()) {
            is_valid_ = false;
            return;
        }

        const auto index_value = Evaluate(node->index);
        if (!index_value.has_value()) {
            is_valid_ = false;
            return;
        }

        const auto* index_scalar = GetScalar(*index_value);
        if (index_scalar == nullptr) {
            is_valid_ = false;
            return;
        }

        const auto index = GetIndex(index_scalar);
        if (!index.has_value()) {
            is_valid_ = false;
            return;
        }

        if (const auto* array = GetArray(*base_value)) {
            if (*index >= array->elements.size()) {
                is_valid_ = false;
                return;
            }

            current_value_ = array->elements[*index];
            return;
        }

        const auto* aggregate = GetAggregate(*base_value);
        if (aggregate == nullptr || aggregate->components.size() != ComponentCount(aggregate->type_desc)) {
            is_valid_ = false;
            return;
        }

        const auto& source_desc = aggregate->type_desc;

        // vector[index] -> scalar
        if (source_desc.vector_count == 1 && source_desc.vector_length > 1) {
            const auto vector_length = static_cast<std::size_t>(source_desc.vector_length);
            if (*index >= vector_length) {
                is_valid_ = false;
                return;
            }

            current_value_ = aggregate->components[*index];
            return;
        }

        // matrix[column] -> column vector
        if (source_desc.vector_count > 1 && source_desc.vector_length > 1) {
            const auto column_count = static_cast<std::size_t>(source_desc.vector_count);
            const auto row_count    = static_cast<std::size_t>(source_desc.vector_length);

            if (*index >= column_count) {
                is_valid_ = false;
                return;
            }

            const auto component_begin = *index * row_count;

            Aggregate result{
                .type_desc  = node->evaluated_type.type_desc,
                .components = {}
            };

            result.components.reserve(row_count);

            for (auto row = 0uz; row != row_count; ++row) {
                result.components.push_back(aggregate->components[component_begin + row]);
            }

            current_value_ = std::move(result);
            return;
        }

        is_valid_ = false;
    }

    void ConstantEvaluator::VisitRawExpression(RawExpressionNode* node) {
        if (node == nullptr || node->tokens.size() != 1 || node->tokens.front().type != TokenType::kNumberLiteral) {
            is_valid_ = false;
            return;
        }

        const auto literal = Utils::AnalyzeNumberLiteral(node->tokens.front().text);
        if (!literal) {
            is_valid_ = false;
            return;
        }

        auto TransformLiteral = [this, &literal] <typename Ty> (std::type_identity<Ty>) -> void {
            Ty value{};
            std::from_chars_result result;
            if constexpr (std::is_floating_point_v<Ty>) {
                result = std::from_chars(literal.core.data(), literal.core.data() + literal.core.size(), value);
            } else {
                result = std::from_chars(literal.core.data(), literal.core.data() + literal.core.size(), value, literal.base);
            }

            if (result.ec == std::errc{}) {
                current_value_ = value;
                return;
            }

            is_valid_ = false;
        };

        switch (literal.kind) {
            using enum Utils::NumberLiteralKind;
        case kSignedInteger:
            TransformLiteral(std::type_identity<std::int64_t>{});
            break;
        case kUnsignedInteger:
            TransformLiteral(std::type_identity<std::uint64_t>{});
            break;
        case kFloatingPoint:
            TransformLiteral(std::type_identity<double>{});
            break;
        case kInvalid:
        default:
            is_valid_ = false;
            break;
        }
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

            switch (type.type_desc.arithmetic_structure()) {
                using enum TypeDescriptor::ArithmeticStructure;
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
        if (node == nullptr || node->object == nullptr) {
            is_valid_ = false;
            return;
        }

        // array.length(), vector.length(), matrix.length()
        if (FindLengthCall(node) != nullptr) {
            const auto length = GetStaticLength(node->object->evaluated_type);
            if (!length.has_value()) {
                is_valid_ = false;
                return;
            }

            current_value_ = *length;
            return;
        }

        // common object.member
        if (node->member == nullptr || node->member->kind() != AstNodeKind::kVariableExpression) {
            is_valid_ = false;
            return;
        }

        auto* member = static_cast<const VariableExpressionNode*>(node->member);
        const auto& object_type = node->object->evaluated_type;

        if (object_type.block_symbol != nullptr) {
            const auto object_value = Evaluate(node->object);
            if (!object_value.has_value()) {
                is_valid_ = false;
                return;
            }

            const auto* object = GetStruct(*object_value);
            if (object == nullptr || object->type_info.block_symbol != object_type.block_symbol) {
                is_valid_ = false;
                return;
            }

            const auto* member_slot = std::get_if<const SymbolInfo*>(&member->linked_symbols);
            if (member_slot == nullptr || *member_slot == nullptr) {
                is_valid_ = false;
                return;
            }

            const auto* member_symbol = *member_slot;
            auto it = std::ranges::find(object->fields, member_symbol, &StructField::symbol);
            if (it == object->fields.end()) {
                is_valid_ = false;
                return;
            }

            current_value_ = it->value;
            return;
        }

        // vector swizzle
        if (object_type.type_desc.arithmetic_structure() != TypeDescriptor::ArithmeticStructure::kVector) {
            is_valid_ = false;
            return;
        }

        const auto swizzle = Utils::ParseVectorSwizzle(
            member->name, static_cast<std::size_t>(object_type.type_desc.vector_length));

        if (!swizzle.has_value()) {
            is_valid_ = false;
            return;
        }

        const auto object_value = Evaluate(node->object);
        if (!object_value.has_value()) {
            is_valid_ = false;
            return;
        }

        const auto* source = GetAggregate(*object_value);

        if (source == nullptr ||
            source->type_desc.vector_count != 1 ||
            source->components.size() != static_cast<std::size_t>(source->type_desc.vector_length))
        {
            is_valid_ = false;
            return;
        }

        if (swizzle->count == 1) {
            current_value_ = source->components[swizzle->indices[0]];
            return;
        }

        Aggregate result{
            .type_desc  = node->evaluated_type.type_desc,
            .components = {}
        };

        result.components.reserve(swizzle->count);

        for (auto i = 0uz; i != swizzle->count; ++i) {
            result.components.push_back(source->components[swizzle->indices[i]]);
        }

        current_value_ = std::move(result);
    }
}
