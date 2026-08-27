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
#include <vector>

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
        using Type = ConstantEvaluator::ValueType;

        template <typename Tuple, std::size_t... Is>
        std::optional<Tuple> ExtractArgs(std::span<const Type> args, std::index_sequence<Is...>) {
            if (args.size() < sizeof...(Is)) {
                return std::nullopt;
            }

            Tuple result{};
            bool ok = (... && ([&]() -> bool {
                using TargetType = std::tuple_element_t<Is, Tuple>;

                if (const auto* value = std::get_if<std::decay_t<TargetType>>(&args[Is])) {
                    std::get<Is>(result) = *value;
                    return true;
                }

                return false;
            }()));

            if (ok) {
                return result;
            }

            return std::nullopt;
        }

        template <typename Return, typename... Args>
        auto WrapSingleSignature(Return(*func)(Args...)) {
            return [func](std::span<const Type> args) -> std::optional<Type> {
                if (args.size() != sizeof...(Args)) {
                    return std::nullopt;
                }

                const auto extracted =
                    ExtractArgs<std::tuple<std::decay_t<Args>...>>(args, std::index_sequence_for<Args...>{});
                if (!extracted.has_value()) {
                    return std::nullopt;
                }

                if constexpr (std::same_as<Return, void>) {
                    std::apply(func, *extracted);
                    return std::nullopt;
                } else {
                    return std::apply(func, *extracted);
                }
            };
        }

        template <typename... Funcs>
        auto MakeOverloader(Funcs... funcs) {
            return [wrappers = std::make_tuple(WrapSingleSignature(funcs)...)](
                std::span<const Type> args
            ) -> std::optional<Type> {
                std::optional<Type> result = std::nullopt;
                std::apply([&](const auto&... wrapper) -> void {
                    (... || (result = wrapper(args), result.has_value()));
                }, wrappers);

                return result;
            };
        }
    }

#define REGISTER_OVERLOADS(name, func) Register(name, MakeOverloader(func<std::int64_t>, func<std::uint64_t>, func<double>))
#define REGISTER_OVERLOADS_INTEGER_ONLY(name, func) Register(name, MakeOverloader(func<std::int64_t>, func<std::uint64_t>))

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

        // atan 包含两个标准重载: atan(y, x) 和 atan(y_over_x)
        Register("atan", MakeOverloader(
            +[](double numerator_y, double denominator_x) -> double {
                return MathMeta::Atan2(numerator_y, denominator_x);
            },
            +[](double slope_y_over_x) -> double {
                return MathMeta::Atan(slope_y_over_x);
            }
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
            +[](double lhs, double rhs, double factor) -> double {
                return MathMeta::Mix(lhs, rhs, factor);
            },
            +[](double lhs, double rhs, bool condition) -> double {
                return MathMeta::MixBool(lhs, rhs, condition);
            },
            +[](std::int64_t lhs, std::int64_t rhs, bool condition) -> std::int64_t {
                return MathMeta::MixBool(lhs, rhs, condition);
            },
            +[](std::uint64_t lhs, std::uint64_t rhs, bool condition) -> std::uint64_t {
                return MathMeta::MixBool(lhs, rhs, condition);
            }
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

    std::optional<ConstantEvaluator::ValueType> ConstantEvaluator::ConvertValueToType(
        const ValueType& value,
        const TypeInfo& target_type) const
    {
        if (!target_type.is_valid()                 ||
            target_type.is_array()                  ||
            target_type.block_symbol != nullptr     ||
            target_type.type_desc.vector_count != 1 ||
            target_type.type_desc.vector_length != 1)
        {
            return std::nullopt;
        }

        switch (target_type.type_desc.family) {
        case BaseFamily::kBool:
            if (const auto* result = std::get_if<bool>(&value)) {
                return *result;
            }
            break;
        case BaseFamily::kInt:
            if (const auto* result = std::get_if<std::int64_t>(&value)) {
                return *result;
            }
            break;
        case BaseFamily::kUint:
            if (const auto* result = std::get_if<std::uint64_t>(&value)) {
                return *result;
            }

            if (const auto* result = std::get_if<std::int64_t>(&value);
                result != nullptr && *result >= 0)
            {
                return static_cast<std::uint64_t>(*result);
            }

            break;
        case BaseFamily::kFloat:
            if (const auto* result = std::get_if<double>(&value))
                return *result;
            if (const auto* result = std::get_if<std::int64_t>(&value))
                return static_cast<double>(*result);
            if (const auto* result = std::get_if<std::uint64_t>(&value))
                return static_cast<double>(*result);
            break;
        default:
            break;
        }

        return std::nullopt;
    }

    std::optional<ConstantEvaluator::ValueType> ConstantEvaluator::ConvertConstructorToType(
        const ValueType& value,
        const TypeInfo& target_type) const
    {
        if (!target_type.is_valid()                 ||
            target_type.is_array()                  ||
            target_type.block_symbol != nullptr     ||
            target_type.type_desc.vector_count != 1 ||
            target_type.type_desc.vector_length != 1)
        {
            return std::nullopt;
        }

        if (auto converted = ConvertValueToType(value, target_type)) {
            return converted;
        }

        switch (target_type.type_desc.family) {
        case BaseFamily::kBool:
            if (const auto* source = std::get_if<std::int64_t>(&value))
                return *source != 0;
            if (const auto* source = std::get_if<std::uint64_t>(&value))
                return *source != 0;
            if (const auto* source = std::get_if<double>(&value))
                return *source != 0.0;
            break;
        case BaseFamily::kInt:
            if (const auto* source = std::get_if<std::uint64_t>(&value)) {
                if (*source <= static_cast<std::uint64_t>(
                    std::numeric_limits<std::int64_t>::max())) {
                    return static_cast<std::int64_t>(*source);
                }
            }

            if (const auto* source = std::get_if<double>(&value)) {
                // 使用半开区间，避免 INT64_MAX 转成 double 后向上舍入。
                constexpr double kMin          = -9223372036854775808.0; // -2^63
                constexpr double kMaxExclusive =  9223372036854775808.0; //  2^63

                if (std::isfinite(*source) &&
                    *source >= kMin &&
                    *source < kMaxExclusive)
                {
                    return static_cast<std::int64_t>(*source);
                }
            }

            if (const auto* source = std::get_if<bool>(&value)) {
                return static_cast<std::int64_t>(*source);
            }

            break;
        case BaseFamily::kUint:
            if (const auto* source = std::get_if<double>(&value)) {
                constexpr double kMaxExclusive = 18446744073709551616.0;

                if (std::isfinite(*source) &&
                    *source >= 0.0 &&
                    *source < kMaxExclusive)
                {
                    return static_cast<std::uint64_t>(*source);
                }
            }

            if (const auto* source = std::get_if<bool>(&value)) {
                return static_cast<std::uint64_t>(*source);
            }

            break;
        case BaseFamily::kFloat:
            if (const auto* source = std::get_if<bool>(&value)) {
                return *source ? 1.0 : 0.0;
            }
            break;
        default:
            break;
        }

        return std::nullopt;
    }

    std::optional<ConstantEvaluator::ValueType> ConstantEvaluator::EvaluateBuiltinFunction(
        std::string_view name,
        std::span<const ValueType> args)
    {
        auto it = registry_.find(name);
        if (it != registry_.end()) {
            return it->second(args);
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

        const auto converted = ConvertConstructorToType(*operand, node->evaluated_type);

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

        const auto left_result  = Evaluate(node->left);
        const auto right_result = Evaluate(node->right);

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

        const auto result = Evaluate(node->operand);
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
            if (node->args.size() != 1 || node->args.front() == nullptr) {
                is_valid_ = false;
                return;
            }

            const auto argument = Evaluate(node->args.front());
            if (!argument.has_value()) {
                is_valid_ = false;
                return;
            }

            const auto converted = ConvertConstructorToType(*argument, node->evaluated_type);

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

            const auto converted = ConvertValueToType(*evaluated, symbol->param_typeinfos[i]);
            if (!converted.has_value()) {
                is_valid_ = false;
                return;
            }

            arguments.push_back(*converted);
        }

        const auto result = EvaluateBuiltinFunction(callee->name, arguments);
        if (!result.has_value()) {
            is_valid_ = false;
            return;
        }

        const auto converted_result = ConvertValueToType(*result, symbol->type_info);
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
