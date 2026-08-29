#pragma once

#include <cstdint>
#include <bit>
#include <type_traits>

#include <glm/glm.hpp>

namespace glsld::MathMeta {

#pragma region Section 8.1 Angle and Trigonometry Functions

    template <typename Ty>
    Ty Radians(Ty degrees) {
        return glm::radians(degrees);
    }

    template <typename Ty>
    Ty Degrees(Ty radians) {
        return glm::degrees(radians);
    }

    template <typename Ty>
    Ty Sin(Ty angle) {
        return glm::sin(angle);
    }

    template <typename Ty>
    Ty Cos(Ty angle) {
        return glm::cos(angle);
    }

    template <typename Ty>
    Ty Tan(Ty angle) {
        return glm::tan(angle);
    }

    template <typename Ty>
    Ty Asin(Ty value) {
        return glm::asin(value);
    }

    template <typename Ty>
    Ty Acos(Ty value) {
        return glm::acos(value);
    }

    template <typename Ty>
    Ty Atan(Ty y, Ty x) {
        return glm::atan(y, x);
    }

    template <typename Ty>
    Ty Atan2(Ty slope_y_over_x) {
        return glm::atan(slope_y_over_x);
    }

    template <typename Ty>
    Ty Sinh(Ty value) {
        return glm::sinh(value);
    }

    template <typename Ty>
    Ty Cosh(Ty value) {
        return glm::cosh(value);
    }

    template <typename Ty>
    Ty Tanh(Ty value) {
        return glm::tanh(value);
    }

    template <typename Ty>
    Ty Asinh(Ty value) {
        return glm::asinh(value);
    }

    template <typename Ty>
    Ty Acosh(Ty value) {
        return glm::acosh(value);
    }

    template <typename Ty>
    Ty Atanh(Ty value) {
        return glm::atanh(value);
    }

#pragma endregion

#pragma region Section 8.2 Exponential Functions

    template <typename Ty>
    Ty Pow(Ty base, Ty exponent) {
        return glm::pow(base, exponent);
    }

    template <typename Ty>
    Ty Exp(Ty value) {
        return glm::exp(value);
    }

    template <typename Ty>
    Ty Exp2(Ty value) {
        return glm::exp2(value);
    }

    template <typename Ty>
    Ty Log(Ty value) {
        return glm::log(value);
    }

    template <typename Ty>
    Ty Log2(Ty value) {
        return glm::log2(value);
    }

    template <typename Ty>
    Ty Sqrt(Ty value) {
        return glm::sqrt(value);
    }

    template <typename Ty>
    Ty InverseSqrt(Ty value) {
        return glm::inversesqrt(value);
    }

#pragma endregion

#pragma region Section 8.3 Common Functions

    namespace Details {
        template <typename Ty>
        struct ComponentType {
            using Type = Ty;
        };

        template <typename Ty>
        requires requires { typename Ty::value_type; }
        struct ComponentType<Ty> {
            using Type = typename Ty::value_type;
        };

        template <typename T>
        using ComponentTypeT = typename ComponentType<std::remove_cvref_t<T>>::Type;
    }

    template <typename Ty>
    Ty Abs(Ty value) {
        using ElementType = Details::ComponentTypeT<Ty>;
        if constexpr (std::is_unsigned_v<ElementType>) {
            return value;
        } else {
            return glm::abs(value);
        }
    }

    template <typename Ty>
    Ty Sign(Ty value) {
        using ElementType = Details::ComponentTypeT<Ty>;
        if constexpr (std::is_unsigned_v<ElementType>) {
            if constexpr (std::same_as<Ty, ElementType>) {
                return (value > static_cast<Ty>(0)) ? static_cast<Ty>(1) : static_cast<Ty>(0);
            } else {
                return glm::mix(Ty(0), Ty(1), glm::greaterThan(value, Ty(0)));
            }
        } else {
            return glm::sign(value);
        }
    }

    template <typename Ty>
    Ty Floor(Ty value) {
        return glm::floor(value);
    }

    template <typename Ty>
    Ty Trunc(Ty value) {
        return glm::trunc(value);
    }

    template <typename Ty>
    Ty Round(Ty value) {
        return glm::round(value);
    }

    template <typename Ty>
    Ty RoundEven(Ty value) {
        return glm::roundEven(value);
    }

    template <typename Ty>
    Ty Ceil(Ty value) {
        return glm::ceil(value);
    }

    template <typename Ty>
    Ty Fract(Ty value) {
        return glm::fract(value);
    }

    template <typename Ty>
    Ty Mod(Ty lhs, Ty rhs) {
        return glm::mod(lhs, rhs);
    }

    template <typename Ty>
    Ty Min(Ty lhs, Ty rhs) {
        return glm::min(lhs, rhs);
    }

    template <typename Ty>
    Ty Max(Ty lhs, Ty rhs) {
        return glm::max(lhs, rhs);
    }

    template <typename Ty>
    Ty Clamp(Ty value, Ty low, Ty high) {
        return glm::clamp(value, low, high);
    }

    template <typename Ty>
    Ty Mix(Ty lhs, Ty rhs, Ty factor) {
        return glm::mix(lhs, rhs, factor);
    }

    template <typename Ty>
    Ty MixScalarFactor(Ty lhs, Ty rhs, double factor) {
        return glm::mix(lhs, rhs, factor);
    }

    template <typename Ty, typename BoolTy>
    Ty MixBool(Ty lhs, Ty rhs, BoolTy condition) {
        if constexpr (std::same_as<BoolTy, bool>) {
            return condition ? rhs : lhs;
        } else {
            return glm::mix(lhs, rhs, condition);
        }
    }

    template <typename Ty>
    Ty Step(Ty edge, Ty value) {
        return glm::step(edge, value);
    }

    template <typename Ty>
    Ty SmoothStep(Ty edge_start, Ty edge_end, Ty value) {
        return glm::smoothstep(edge_start, edge_end, value);
    }

    template <typename Ty>
    auto IsNan(Ty value) {
        return glm::isnan(value);
    }

    template <typename Ty>
    auto IsInf(Ty value) {
        return glm::isinf(value);
    }

    template <typename Ty>
    Ty Fma(Ty x, Ty y, Ty z) {
        return glm::fma(x, y, z);
    }

    template <typename Ty1, typename Ty2>
    auto Ldexp(Ty1 value, Ty2 exponent) {
        return glm::ldexp(value, exponent);
    }

    namespace Details {
        // IEEE 754 float32 -> float16
        inline std::uint16_t FloatToHalf(float value) {
            const std::uint32_t float_bits = std::bit_cast<std::uint32_t>(value);
            const std::uint32_t sign       = (float_bits >> 16) & 0x8000u;
            std::int32_t  exponent = static_cast<std::int32_t>((float_bits >> 23) & 0xFFu) - 127 + 15;
            std::uint32_t mantissa = float_bits & 0x007FFFFFu;

            if (exponent <= 0) {
                if (exponent < -10) {
                    return static_cast<std::uint16_t>(sign);
                }
                mantissa = (mantissa | 0x00800000u) >> (1 - exponent);
                return static_cast<std::uint16_t>(sign | (mantissa >> 13));
            } else if (exponent >= 31) {
                return static_cast<std::uint16_t>(sign | 0x7C00u);
            }

            return static_cast<std::uint16_t>(sign | (static_cast<std::uint32_t>(exponent) << 10) | (mantissa >> 13));
        }

        // IEEE 754 float16 -> float32
        inline float HalfToFloat(std::uint16_t value) {
            const std::uint32_t sign     = static_cast<std::uint32_t>(value & 0x8000u) << 16;
            const std::uint32_t exponent = (value >> 10) & 0x1Fu;
            const std::uint32_t mantissa = value & 0x03FFu;

            if (exponent == 0) {
                if (mantissa == 0) {
                    return std::bit_cast<float>(sign);
                }
                float result = std::bit_cast<float>(sign | 0x38800000u);
                result += static_cast<float>(mantissa) * (1.0f / (1 << 24));
                return result;
            } else if (exponent == 31) {
                return std::bit_cast<float>(sign | 0x7F800000u | (mantissa << 13));
            }

            const std::uint32_t float_exponent = (exponent - 15 + 127) << 23;
            return std::bit_cast<float>(sign | float_exponent | (mantissa << 13));
        }
    } // namespace Details

    inline std::int64_t Float16BitsToInt16(double value) {
        const auto half_bits = static_cast<std::int16_t>(Details::FloatToHalf(static_cast<float>(value)));
        return static_cast<std::int64_t>(half_bits);
    }

    inline std::uint64_t Float16BitsToUint16(double value) {
        const auto half_bits = Details::FloatToHalf(static_cast<float>(value));
        return static_cast<std::uint64_t>(half_bits);
    }

    inline double Int16BitsToFloat16(std::int64_t value) {
        const auto half_bits = static_cast<std::uint16_t>(static_cast<std::int16_t>(value));
        return static_cast<double>(Details::HalfToFloat(half_bits));
    }

    inline double Uint16BitsToFloat16(std::uint64_t value) {
        const auto half_bits = static_cast<std::uint16_t>(value);
        return static_cast<double>(Details::HalfToFloat(half_bits));
    }

    inline std::int64_t FloatBitsToInt(double value) {
        const float float_value = static_cast<float>(value);
        return static_cast<std::int64_t>(std::bit_cast<std::int32_t>(float_value));
    }

    inline std::uint64_t FloatBitsToUint(double value) {
        const float float_value = static_cast<float>(value);
        return static_cast<std::uint64_t>(std::bit_cast<std::uint32_t>(float_value));
    }

    inline double IntBitsToFloat(std::int64_t value) {
        const auto int32_value = static_cast<std::int32_t>(value);
        return static_cast<double>(std::bit_cast<float>(int32_value));
    }

    inline double UintBitsToFloat(std::uint64_t value) {
        const auto uint32_value = static_cast<std::uint32_t>(value);
        return static_cast<double>(std::bit_cast<float>(uint32_value));
    }

    inline std::int64_t DoubleBitsToInt64(double value) {
        return std::bit_cast<std::int64_t>(value);
    }

    inline std::uint64_t DoubleBitsToUint64(double value) {
        return std::bit_cast<std::uint64_t>(value);
    }

    inline double Int64BitsToDouble(std::int64_t value) {
        return std::bit_cast<double>(value);
    }

    inline double Uint64BitsToDouble(std::uint64_t value) {
        return std::bit_cast<double>(value);
    }

#pragma endregion

#pragma region Section 8.4 Floating-Point Pack and Unpack Functions

    inline std::uint64_t PackUnorm2x16(glm::dvec2 value) {
        return static_cast<std::uint64_t>(glm::packUnorm2x16(glm::vec2(value)));
    }

    inline std::uint64_t PackSnorm2x16(glm::dvec2 value) {
        return static_cast<std::uint64_t>(glm::packSnorm2x16(glm::vec2(value)));
    }

    inline std::uint64_t PackUnorm4x8(glm::dvec4 value) {
        return static_cast<std::uint64_t>(glm::packUnorm4x8(glm::vec4(value)));
    }

    inline std::uint64_t PackSnorm4x8(glm::dvec4 value) {
        return static_cast<std::uint64_t>(glm::packSnorm4x8(glm::vec4(value)));
    }

    inline glm::dvec2 UnpackUnorm2x16(std::uint64_t value) {
        return glm::dvec2(glm::unpackUnorm2x16(static_cast<glm::uint32>(value)));
    }

    inline glm::dvec2 UnpackSnorm2x16(std::uint64_t value) {
        return glm::dvec2(glm::unpackSnorm2x16(static_cast<glm::uint32>(value)));
    }

    inline glm::dvec4 UnpackUnorm4x8(std::uint64_t value) {
        return glm::dvec4(glm::unpackUnorm4x8(static_cast<glm::uint32>(value)));
    }

    inline glm::dvec4 UnpackSnorm4x8(std::uint64_t value) {
        return glm::dvec4(glm::unpackSnorm4x8(static_cast<glm::uint32>(value)));
    }

    inline std::uint64_t PackHalf2x16(glm::dvec2 value) {
        return static_cast<std::uint64_t>(glm::packHalf2x16(glm::vec2(value)));
    }

    inline glm::dvec2 UnpackHalf2x16(std::uint64_t value) {
        return glm::dvec2(glm::unpackHalf2x16(static_cast<glm::uint32>(value)));
    }

    inline double PackDouble2x32(glm::u64vec2 value) {
        return glm::packDouble2x32(glm::uvec2(value));
    }

    inline glm::u64vec2 UnpackDouble2x32(double value) {
        return glm::u64vec2(glm::unpackDouble2x32(value));
    }

    inline std::uint64_t PackFloat2x16(glm::dvec2 value) {
        return static_cast<std::uint64_t>(glm::packHalf2x16(glm::vec2(value)));
    }

    inline glm::dvec2 UnpackFloat2x16(std::uint64_t value) {
        return glm::dvec2(glm::unpackHalf2x16(static_cast<glm::uint32>(value)));
    }

    inline std::int64_t PackInt2x16(glm::i64vec2 value) {
        const auto x = static_cast<std::uint16_t>(static_cast<std::int16_t>(value.x));
        const auto y = static_cast<std::uint16_t>(static_cast<std::int16_t>(value.y));
        return static_cast<std::int64_t>(static_cast<std::int32_t>((static_cast<std::uint32_t>(y) << 16) | x));
    }

    inline std::uint64_t PackUint2x16(glm::u64vec2 value) {
        const auto x = static_cast<std::uint16_t>(value.x);
        const auto y = static_cast<std::uint16_t>(value.y);
        return static_cast<std::uint64_t>((static_cast<std::uint32_t>(y) << 16) | x);
    }

    inline std::int64_t PackInt4x16(glm::i64vec4 value) {
        const auto x = static_cast<std::uint64_t>(static_cast<std::uint16_t>(static_cast<std::int16_t>(value.x)));
        const auto y = static_cast<std::uint64_t>(static_cast<std::uint16_t>(static_cast<std::int16_t>(value.y)));
        const auto z = static_cast<std::uint64_t>(static_cast<std::uint16_t>(static_cast<std::int16_t>(value.z)));
        const auto w = static_cast<std::uint64_t>(static_cast<std::uint16_t>(static_cast<std::int16_t>(value.w)));
        return static_cast<std::int64_t>((w << 48) | (z << 32) | (y << 16) | x);
    }

    inline std::uint64_t PackUint4x16(glm::u64vec4 value) {
        const auto x = static_cast<std::uint64_t>(static_cast<std::uint16_t>(value.x));
        const auto y = static_cast<std::uint64_t>(static_cast<std::uint16_t>(value.y));
        const auto z = static_cast<std::uint64_t>(static_cast<std::uint16_t>(value.z));
        const auto w = static_cast<std::uint64_t>(static_cast<std::uint16_t>(value.w));
        return (w << 48) | (z << 32) | (y << 16) | x;
    }

    inline glm::i64vec2 UnpackInt2x16(std::int64_t value) {
        const auto u32 = static_cast<std::uint32_t>(value);
        return glm::i64vec2(
            static_cast<std::int64_t>(static_cast<std::int16_t>(u32 & 0xFFFFu)),
            static_cast<std::int64_t>(static_cast<std::int16_t>((u32 >> 16) & 0xFFFFu))
        );
    }

    inline glm::u64vec2 UnpackUint2x16(std::uint64_t value) {
        const auto u32 = static_cast<std::uint32_t>(value);
        return glm::u64vec2(
            static_cast<std::uint64_t>(u32 & 0xFFFFu),
            static_cast<std::uint64_t>((u32 >> 16) & 0xFFFFu)
        );
    }

    inline glm::i64vec4 UnpackInt4x16(std::int64_t value) {
        const auto u64 = static_cast<std::uint64_t>(value);
        return glm::i64vec4(
            static_cast<std::int64_t>(static_cast<std::int16_t>(u64 & 0xFFFFu)),
            static_cast<std::int64_t>(static_cast<std::int16_t>((u64 >> 16) & 0xFFFFu)),
            static_cast<std::int64_t>(static_cast<std::int16_t>((u64 >> 32) & 0xFFFFu)),
            static_cast<std::int64_t>(static_cast<std::int16_t>((u64 >> 48) & 0xFFFFu))
        );
    }

    inline glm::u64vec4 UnpackUint4x16(std::uint64_t value) {
        return glm::u64vec4(
            (value) & 0xFFFFu,
            (value >> 16) & 0xFFFFu,
            (value >> 32) & 0xFFFFu,
            (value >> 48) & 0xFFFFu
        );
    }

    inline std::int64_t PackInt2x32(glm::i64vec2 value) {
        const auto low = static_cast<std::uint64_t>(static_cast<std::uint32_t>(static_cast<std::int32_t>(value.x)));
        const auto high = static_cast<std::uint64_t>(static_cast<std::uint32_t>(static_cast<std::int32_t>(value.y)));
        return static_cast<std::int64_t>((high << 32) | low);
    }

    inline std::uint64_t PackUint2x32(glm::u64vec2 value) {
        const auto low = static_cast<std::uint64_t>(static_cast<std::uint32_t>(value.x));
        const auto high = static_cast<std::uint64_t>(static_cast<std::uint32_t>(value.y));
        return (high << 32) | low;
    }

    inline glm::i64vec2 UnpackInt2x32(std::int64_t value) {
        const auto u64 = static_cast<std::uint64_t>(value);
        return glm::i64vec2(
            static_cast<std::int64_t>(static_cast<std::int32_t>(u64 & 0xFFFFFFFFu)),
            static_cast<std::int64_t>(static_cast<std::int32_t>((u64 >> 32) & 0xFFFFFFFFu))
        );
    }

    inline glm::u64vec2 UnpackUint2x32(std::uint64_t value) {
        return glm::u64vec2(
            value & 0xFFFFFFFFu,
            (value >> 32) & 0xFFFFFFFFu
        );
    }

#pragma endregion

#pragma region Section 8.5 Geometric Functions

    template <typename Ty>
    auto Length(Ty value) {
        return glm::length(value);
    }

    template <typename Ty>
    auto Distance(Ty lhs, Ty rhs) {
        return glm::distance(lhs, rhs);
    }

    template <typename Ty>
    auto Dot(Ty lhs, Ty rhs) {
        return glm::dot(lhs, rhs);
    }

    template <typename Ty>
    Ty Cross(Ty lhs, Ty rhs) {
        return glm::cross(lhs, rhs);
    }

    template <typename Ty>
    Ty Normalize(Ty value) {
        if constexpr (std::is_floating_point_v<Ty>) {
            return value / glm::abs(value);
        } else {
            return glm::normalize(value);
        }
    }

    template <typename Ty>
    Ty FaceForward(Ty normal, Ty incident, Ty reference_normal) {
        return glm::faceforward(normal, incident, reference_normal);
    }

    template <typename Ty>
    Ty Reflect(Ty incident, Ty normal) {
        return glm::reflect(incident, normal);
    }

    template <typename Ty>
    Ty Refract(Ty incident, Ty normal, double eta) {
        return glm::refract(incident, normal, eta);
    }

#pragma endregion

#pragma region Section 8.6 Matrix Functions

    template <typename Ty>
    Ty MatrixCompMult(Ty lhs, Ty rhs) {
        return glm::matrixCompMult(lhs, rhs);
    }

    template <typename ColVector, typename RowVector>
    auto OuterProduct(ColVector column_vector, RowVector row_vector) {
        return glm::outerProduct(column_vector, row_vector);
    }

    template <typename Ty>
    auto Transpose(Ty matrix) {
        return glm::transpose(matrix);
    }

    template <typename Ty>
    auto Determinant(Ty matrix) {
        return glm::determinant(matrix);
    }

    template <typename Ty>
    Ty Inverse(Ty matrix) {
        return glm::inverse(matrix);
    }

#pragma endregion

#pragma region Section 8.7 Vector Relational Functions

    template <typename Ty>
    auto LessThan(Ty lhs, Ty rhs) {
        return glm::lessThan(lhs, rhs);
    }

    template <typename Ty>
    auto LessThanEqual(Ty lhs, Ty rhs) {
        return glm::lessThanEqual(lhs, rhs);
    }

    template <typename Ty>
    auto GreaterThan(Ty lhs, Ty rhs) {
        return glm::greaterThan(lhs, rhs);
    }

    template <typename Ty>
    auto GreaterThanEqual(Ty lhs, Ty rhs) {
        return glm::greaterThanEqual(lhs, rhs);
    }

    template <typename Ty>
    auto Equal(Ty lhs, Ty rhs) {
        return glm::equal(lhs, rhs);
    }

    template <typename Ty>
    auto NotEqual(Ty lhs, Ty rhs) {
        return glm::notEqual(lhs, rhs);
    }

    template <typename Ty>
    bool Any(Ty condition) {
        if constexpr (std::same_as<Ty, bool>) {
            return condition;
        } else {
            return glm::any(condition);
        }
    }

    template <typename Ty>
    bool All(Ty condition) {
        if constexpr (std::same_as<Ty, bool>) {
            return condition;
        } else {
            return glm::all(condition);
        }
    }

    template <typename Ty>
    Ty Not(Ty condition) {
        if constexpr (std::same_as<Ty, bool>) {
            return !condition;
        } else {
            return glm::not_(condition);
        }
    }

#pragma endregion

#pragma region Section 8.8 Integer Functions (基于 32-bit 截断保证绝对正确)

    template <typename Ty>
    struct IntegerQueryResult {
        using Type = std::int64_t;
    };

    template <glm::length_t Length, typename Ty, glm::qualifier Qualifier>
    struct IntegerQueryResult<glm::vec<Length, Ty, Qualifier>> {
        using Type = glm::vec<Length, std::int64_t, Qualifier>;
    };

    template <typename Ty>
    using IntegerQueryResultT = typename IntegerQueryResult<std::remove_cv_t<Ty>>::Type;

    template <typename Ty>
    IntegerQueryResultT<Ty> BitCount(Ty value) {
        using ResultType = IntegerQueryResultT<Ty>;
        if constexpr (std::same_as<Ty, std::int64_t> || std::same_as<Ty, std::uint64_t>) {
            return static_cast<std::int64_t>(std::popcount(static_cast<std::uint32_t>(value)));
        } else {
            using Uvec32 = glm::vec<Ty::length(), glm::uint32, glm::defaultp>;
            return ResultType(glm::bitCount(Uvec32(value)));
        }
    }

    template <typename Ty>
    IntegerQueryResultT<Ty> FindLsb(Ty value) {
        using ResultType = IntegerQueryResultT<Ty>;
        if constexpr (std::same_as<Ty, std::int64_t> || std::same_as<Ty, std::uint64_t>) {
            const auto u32 = static_cast<std::uint32_t>(value);
            return (u32 == 0) ? static_cast<std::int64_t>(-1) : static_cast<std::int64_t>(std::countr_zero(u32));
        } else {
            using Uvec32 = glm::vec<Ty::length(), glm::uint32, glm::defaultp>;
            return ResultType(glm::findLSB(Uvec32(value)));
        }
    }

    template <typename Ty>
    IntegerQueryResultT<Ty> FindMsb(Ty value) {
        using ResultType = IntegerQueryResultT<Ty>;
        if constexpr (std::same_as<Ty, std::int64_t>) {
            const auto i32 = static_cast<std::int32_t>(value);
            if (i32 == 0 || i32 == -1) return static_cast<std::int64_t>(-1);
            if (i32 < 0) return static_cast<std::int64_t>(31 - std::countl_zero(static_cast<std::uint32_t>(~i32)));
            return static_cast<std::int64_t>(31 - std::countl_zero(static_cast<std::uint32_t>(i32)));
        } else if constexpr (std::same_as<Ty, std::uint64_t>) {
            const auto u32 = static_cast<std::uint32_t>(value);
            if (u32 == 0) return static_cast<std::int64_t>(-1);
            return static_cast<std::int64_t>(31 - std::countl_zero(u32));
        } else {
            if constexpr (std::is_signed_v<typename Ty::value_type>) {
                using Ivec32 = glm::vec<Ty::length(), glm::int32, glm::defaultp>;
                return ResultType(glm::findMSB(Ivec32(value)));
            } else {
                using Uvec32 = glm::vec<Ty::length(), glm::uint32, glm::defaultp>;
                return ResultType(glm::findMSB(Uvec32(value)));
            }
        }
    }

    template <typename Ty>
    Ty BitfieldReverse(Ty value) {
        if constexpr (std::same_as<Ty, std::int64_t> || std::same_as<Ty, std::uint64_t>) {
            auto bits32 = static_cast<std::uint32_t>(value);
            bits32 = ((bits32 >> 1) & 0x55555555u) | ((bits32 & 0x55555555u) << 1);
            bits32 = ((bits32 >> 2) & 0x33333333u) | ((bits32 & 0x33333333u) << 2);
            bits32 = ((bits32 >> 4) & 0x0F0F0F0Fu) | ((bits32 & 0x0F0F0F0Fu) << 4);
            bits32 = ((bits32 >> 8) & 0x00FF00FFu) | ((bits32 & 0x00FF00FFu) << 8);
            bits32 = (bits32 >> 16) | (bits32 << 16);

            if constexpr (std::same_as<Ty, std::int64_t>) {
                return static_cast<std::int64_t>(static_cast<std::int32_t>(bits32));
            } else {
                return static_cast<std::uint64_t>(bits32);
            }
        } else {
            if constexpr (std::is_signed_v<typename Ty::value_type>) {
                using Ivec32 = glm::vec<Ty::length(), glm::int32, glm::defaultp>;
                return Ty(glm::bitfieldReverse(Ivec32(value)));
            } else {
                using Uvec32 = glm::vec<Ty::length(), glm::uint32, glm::defaultp>;
                return Ty(glm::bitfieldReverse(Uvec32(value)));
            }
        }
    }

    template <typename Ty>
    Ty BitfieldExtract(Ty value, std::int64_t offset, std::int64_t bits) {
        if constexpr (std::same_as<Ty, std::int64_t>) {
            return static_cast<std::int64_t>(glm::bitfieldExtract(static_cast<std::int32_t>(value), static_cast<int>(offset), static_cast<int>(bits)));
        } else if constexpr (std::same_as<Ty, std::uint64_t>) {
            return static_cast<std::uint64_t>(glm::bitfieldExtract(static_cast<std::uint32_t>(value), static_cast<int>(offset), static_cast<int>(bits)));
        } else {
            if constexpr (std::is_signed_v<typename Ty::value_type>) {
                using Ivec32 = glm::vec<Ty::length(), glm::int32, glm::defaultp>;
                return Ty(glm::bitfieldExtract(Ivec32(value), static_cast<int>(offset), static_cast<int>(bits)));
            } else {
                using Uvec32 = glm::vec<Ty::length(), glm::uint32, glm::defaultp>;
                return Ty(glm::bitfieldExtract(Uvec32(value), static_cast<int>(offset), static_cast<int>(bits)));
            }
        }
    }

    template <typename Ty>
    Ty BitfieldInsert(Ty base, Ty insert, std::int64_t offset, std::int64_t bits) {
        if constexpr (std::same_as<Ty, std::int64_t>) {
            return static_cast<std::int64_t>(glm::bitfieldInsert(static_cast<std::int32_t>(base), static_cast<std::int32_t>(insert), static_cast<int>(offset), static_cast<int>(bits)));
        } else if constexpr (std::same_as<Ty, std::uint64_t>) {
            return static_cast<std::uint64_t>(glm::bitfieldInsert(static_cast<std::uint32_t>(base), static_cast<std::uint32_t>(insert), static_cast<int>(offset), static_cast<int>(bits)));
        } else {
            if constexpr (std::is_signed_v<typename Ty::value_type>) {
                using Ivec32 = glm::vec<Ty::length(), glm::int32, glm::defaultp>;
                return Ty(glm::bitfieldInsert(Ivec32(base), Ivec32(insert), static_cast<int>(offset), static_cast<int>(bits)));
            } else {
                using Uvec32 = glm::vec<Ty::length(), glm::uint32, glm::defaultp>;
                return Ty(glm::bitfieldInsert(Uvec32(base), Uvec32(insert), static_cast<int>(offset), static_cast<int>(bits)));
            }
        }
    }

#pragma endregion

} // namespace glsld::MathMeta
