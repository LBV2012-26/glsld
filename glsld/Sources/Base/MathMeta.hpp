#pragma once

#include <cmath>
#include <cstdint>
#include <algorithm>
#include <bit>
#include <numeric>
#include <type_traits>

namespace glsld::MathMeta {
#pragma region Section 8.1
    template <typename Ty>
    Ty Radians(Ty degrees) {
        return degrees * (static_cast<Ty>(std::numbers::pi) / static_cast<Ty>(180.0));
    }

    template <typename Ty>
    Ty Degrees(Ty radians) {
        return radians * (static_cast<Ty>(180.0) / static_cast<Ty>(std::numbers::pi));
    }

    template <typename Ty>
    Ty Sin(Ty angle) {
        return std::sin(angle);
    }

    template <typename Ty>
    Ty Cos(Ty angle) {
        return std::cos(angle);
    }

    template <typename Ty>
    Ty Tan(Ty angle) {
        return std::tan(angle);
    }

    template <typename Ty>
    Ty Asin(Ty x) {
        return std::asin(x);
    }

    template <typename Ty>
    Ty Acos(Ty x) {
        return std::acos(x);
    }

    template <typename Ty>
    Ty Atan2(Ty y, Ty x) {
        return std::atan2(y, x);
    }

    template <typename Ty>
    Ty Atan(Ty y_over_x) {
        return std::atan(y_over_x);
    }

    template <typename Ty>
    Ty Sinh(Ty x) {
        return std::sinh(x);
    }

    template <typename Ty>
    Ty Cosh(Ty x) {
        return std::cosh(x);
    }

    template <typename Ty>
    Ty Tanh(Ty x) {
        return std::tanh(x);
    }

    template <typename Ty>
    Ty Asinh(Ty x) {
        return std::asinh(x);
    }

    template <typename Ty>
    Ty Acosh(Ty x) {
        return std::acosh(x);
    }

    template <typename Ty>
    Ty Atanh(Ty x) {
        return std::atanh(x);
    }
#pragma endregion

#pragma region Section 8.2
    template <typename Ty>
    Ty Pow(Ty x, Ty y) {
        return std::pow(x, y);
    }

    template <typename Ty>
    Ty Exp(Ty x) {
        return std::exp(x);
    }

    template <typename Ty>
    Ty Exp2(Ty x) {
        return std::exp2(x);
    }

    template <typename Ty>
    Ty Log(Ty x) {
        return std::log(x);
    }

    template <typename Ty>
    Ty Log2(Ty x) {
        return std::log2(x);
    }

    template <typename Ty>
    Ty Sqrt(Ty x) {
        return std::sqrt(x);
    }

    template <typename Ty>
    Ty InverseSqrt(Ty x) {
        return static_cast<Ty>(1.0) / std::sqrt(x);
    }
#pragma endregion

#pragma region Section 8.3
    template <typename Ty>
    Ty Abs(Ty value) {
        if constexpr (std::is_unsigned_v<Ty>) {
            return value;
        } else {
            return std::abs(value);
        }
    }

    template <typename Ty>
    Ty Sign(Ty value) {
        if constexpr (std::is_unsigned_v<Ty>) {
            return (value > static_cast<Ty>(0)) ? static_cast<Ty>(1) : static_cast<Ty>(0);
        } else {
            if (value > static_cast<Ty>(0)) return static_cast<Ty>(1);
            if (value < static_cast<Ty>(0)) return static_cast<Ty>(-1);
            return static_cast<Ty>(0);
        }
    }

    template <typename Ty>
    Ty Floor(Ty value) {
        if constexpr (std::is_floating_point_v<Ty>) {
            return std::floor(value);
        } else {
            return value;
        }
    }

    template <typename Ty>
    Ty Trunc(Ty value) {
        if constexpr (std::is_floating_point_v<Ty>) {
            return std::trunc(value);
        } else {
            return value;
        }
    }

    template <typename Ty>
    Ty Round(Ty value) {
        if constexpr (std::is_floating_point_v<Ty>) {
            return std::round(value);
        } else {
            return value;
        }
    }

    template <typename Ty>
    Ty RoundEven(Ty value) {
        if constexpr (std::is_floating_point_v<Ty>) {
            return std::nearbyint(value);
        } else {
            return value;
        }
    }

    template <typename Ty>
    Ty Ceil(Ty value) {
        if constexpr (std::is_floating_point_v<Ty>) {
            return std::ceil(value);
        } else {
            return value;
        }
    }

    template <typename Ty>
    Ty Fract(Ty value) {
        return value - Floor(value);
    }

    template <typename Ty>
    Ty Mod(Ty lhs, Ty rhs) {
        if (rhs == static_cast<Ty>(0)) {
            return static_cast<Ty>(0);
        }

        if constexpr (std::is_floating_point_v<Ty>) {
            return lhs - rhs * std::floor(lhs / rhs);
        } else {
            Ty result = lhs % rhs;
            if ((result > 0 && rhs < 0) || (result < 0 && rhs > 0)) {
                result += rhs;
            }
            return result;
        }
    }

    template <typename Ty>
    Ty Min(Ty lhs, Ty rhs) {
        return std::min(lhs, rhs);
    }

    template <typename Ty>
    Ty Max(Ty lhs, Ty rhs) {
        return std::max(lhs, rhs);
    }

    template <typename Ty>
    Ty Clamp(Ty value, Ty low, Ty high) {
        return std::clamp(value, low, high);
    }

    template <typename Ty>
    Ty Mix(Ty lhs, Ty rhs, Ty factor) {
        return lhs * (static_cast<Ty>(1) - factor) + rhs * factor;
    }

    template <typename Ty>
    Ty MixBool(Ty lhs, Ty rhs, bool condition) {
        return condition ? rhs : lhs;
    }

    template <typename Ty>
    Ty Step(Ty edge, Ty value) {
        return (value < edge) ? static_cast<Ty>(0) : static_cast<Ty>(1);
    }

    template <typename Ty>
    Ty SmoothStep(Ty edge_start, Ty edge_end, Ty value) {
        if (edge_start >= edge_end) {
            return static_cast<Ty>(0);
        }
        Ty clamped_range = std::clamp((value - edge_start) / (edge_end - edge_start), static_cast<Ty>(0), static_cast<Ty>(1));
        return clamped_range * clamped_range * (static_cast<Ty>(3) - static_cast<Ty>(2) * clamped_range);
    }

    template <typename Ty>
    bool IsNan(Ty value) {
        if constexpr (std::is_floating_point_v<Ty>) {
            return std::isnan(value);
        } else {
            return false;
        }
    }

    template <typename Ty>
    bool IsInf(Ty value) {
        if constexpr (std::is_floating_point_v<Ty>) {
            return std::isinf(value);
        } else {
            return false;
        }
    }

    template <typename Ty>
    Ty Fma(Ty lhs, Ty rhs, Ty addend) {
        if constexpr (std::is_floating_point_v<Ty>) {
            return std::fma(lhs, rhs, addend);
        } else {
            return lhs * rhs + addend;
        }
    }

    template <typename Ty>
    Ty Ldexp(Ty value, std::int64_t exponent) {
        return std::ldexp(value, static_cast<int>(exponent));
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
#pragma endregion

#pragma region Section 8.4
    namespace Detail {
        inline std::uint32_t PackUnorm(double value, double scale) {
            const double clamped_value = std::clamp(value, 0.0, 1.0);
            return static_cast<std::uint32_t>(std::round(clamped_value * scale));
        }

        inline double UnpackUnorm(std::uint32_t value, double scale) {
            return static_cast<double>(value) / scale;
        }

        inline std::int32_t PackSnorm(double value, double scale) {
            const double clamped_value = std::clamp(value, -1.0, 1.0);
            return static_cast<std::int32_t>(std::round(clamped_value * scale));
        }

        inline double UnpackSnorm(std::int32_t value, double scale) {
            const double unpacked_value = static_cast<double>(value) / scale;
            return std::clamp(unpacked_value, -1.0, 1.0);
        }

        // IEEE 754 float32 <-> float16
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
    } // namespace Detail

    // 1. Unorm 2x16 (vec2 <-> uint32)
    inline std::uint64_t PackUnorm2x16(double x, double y) {
        const std::uint32_t packed_x = Detail::PackUnorm(x, 65535.0);
        const std::uint32_t packed_y = Detail::PackUnorm(y, 65535.0);
        return static_cast<std::uint64_t>((packed_y << 16) | packed_x);
    }

    // 2. Snorm 2x16 (vec2 <-> uint32)
    inline std::uint64_t PackSnorm2x16(double x, double y) {
        const auto packed_x = static_cast<std::uint16_t>(Detail::PackSnorm(x, 32767.0));
        const auto packed_y = static_cast<std::uint16_t>(Detail::PackSnorm(y, 32767.0));
        return static_cast<std::uint64_t>((static_cast<std::uint32_t>(packed_y) << 16) | static_cast<std::uint32_t>(packed_x));
    }

    // 3. Unorm 4x8 (vec4 <-> uint32)
    inline std::uint64_t PackUnorm4x8(double x, double y, double z, double w) {
        const std::uint32_t packed_x = Detail::PackUnorm(x, 255.0);
        const std::uint32_t packed_y = Detail::PackUnorm(y, 255.0);
        const std::uint32_t packed_z = Detail::PackUnorm(z, 255.0);
        const std::uint32_t packed_w = Detail::PackUnorm(w, 255.0);
        return static_cast<std::uint64_t>((packed_w << 24) | (packed_z << 16) | (packed_y << 8) | packed_x);
    }

    // 4. Snorm 4x8 (vec4 <-> uint32)
    inline std::uint64_t PackSnorm4x8(double x, double y, double z, double w) {
        const auto packed_x = static_cast<std::uint8_t>(Detail::PackSnorm(x, 127.0));
        const auto packed_y = static_cast<std::uint8_t>(Detail::PackSnorm(y, 127.0));
        const auto packed_z = static_cast<std::uint8_t>(Detail::PackSnorm(z, 127.0));
        const auto packed_w = static_cast<std::uint8_t>(Detail::PackSnorm(w, 127.0));

        return static_cast<std::uint64_t>(
            (static_cast<std::uint32_t>(packed_w) << 24) |
            (static_cast<std::uint32_t>(packed_z) << 16) |
            (static_cast<std::uint32_t>(packed_y) << 8)  |
             static_cast<std::uint32_t>(packed_x)
        );
    }

    // 5. Half 2x16 (vec2 <-> uint32, 半精度浮点打包)
    inline std::uint64_t PackHalf2x16(double x, double y) {
        const std::uint16_t packed_x = Detail::FloatToHalf(static_cast<float>(x));
        const std::uint16_t packed_y = Detail::FloatToHalf(static_cast<float>(y));
        return static_cast<std::uint64_t>((static_cast<std::uint32_t>(packed_y) << 16) | static_cast<std::uint32_t>(packed_x));
    }

    // 6. Double 2x32 (uvec2 <-> double, 将两个 uint32 拼接为一个 64 位 double)
    inline double PackDouble2x32(std::uint64_t x, std::uint64_t y) {
        const auto low_bits  = static_cast<std::uint32_t>(x);
        const auto high_bits = static_cast<std::uint32_t>(y);
        const std::uint64_t combined_bits = (static_cast<std::uint64_t>(high_bits) << 32) | static_cast<std::uint64_t>(low_bits);
        return std::bit_cast<double>(combined_bits);
    }
#pragma endregion

#pragma region Section 8.5
    template <typename Ty>
    Ty Length(Ty value) {
        return std::abs(value);
    }

    template <typename Ty>
    Ty Distance(Ty lhs, Ty rhs) {
        return std::abs(lhs - rhs);
    }

    template <typename Ty>
    Ty Dot(Ty lhs, Ty rhs) {
        return lhs * rhs;
    }

    template <typename Ty>
    Ty Normalize(Ty value) {
        return (value > static_cast<Ty>(0)) ? static_cast<Ty>(1) : static_cast<Ty>(-1);
    }

    template <typename Ty>
    Ty FaceForward(Ty normal, Ty incident, Ty reference_normal) {
        return (reference_normal * incident < static_cast<Ty>(0)) ? normal : -normal;
    }

    template <typename Ty>
    Ty Reflect(Ty incident, Ty normal) {
        return incident - static_cast<Ty>(2.0) * (normal * incident) * normal;
    }

    template <typename Ty>
    Ty Refract(Ty incident, Ty normal, Ty eta) {
        const Ty dot_product  = normal * incident;
        const Ty discriminant = static_cast<Ty>(1.0) - std::pow(eta, static_cast<Ty>(2)) * (static_cast<Ty>(1.0) - std::pow(dot_product, static_cast<Ty>(2)));
        if (discriminant < static_cast<Ty>(0.0)) {
            return static_cast<Ty>(0.0);
        }
        return eta * incident - (eta * dot_product + std::sqrt(discriminant)) * normal;
    }
#pragma endregion

#pragma region Section 8.7
    template <typename Ty> bool LessThan(Ty lhs, Ty rhs) {
        return lhs < rhs;
    }

    template <typename Ty> bool LessThanEqual(Ty lhs, Ty rhs) {
        return lhs <= rhs;
    }

    template <typename Ty> bool GreaterThan(Ty lhs, Ty rhs) {
        return lhs > rhs;
    }

    template <typename Ty> bool GreaterThanEqual(Ty lhs, Ty rhs) {
        return lhs >= rhs;
    }

    template <typename Ty> bool Equal(Ty lhs, Ty rhs) {
        return lhs == rhs;
    }

    template <typename Ty> bool NotEqual(Ty lhs, Ty rhs) {
        return lhs != rhs;
    }

    inline bool LogicalNot(bool condition) {
        return !condition;
    }

    inline bool Any(bool condition) {
        return condition;
    }

    inline bool All(bool condition) {
        return condition;
    }
#pragma endregion

#pragma region Section 8.8
    template <typename Ty>
    std::int64_t BitCount(Ty value) {
        if constexpr (std::is_signed_v<Ty>) {
            return static_cast<std::int64_t>(std::popcount(static_cast<std::uint32_t>(value)));
        } else {
            return static_cast<std::int64_t>(std::popcount(static_cast<std::uint32_t>(value)));
        }
    }

    template <typename Ty>
    std::int64_t FindLsb(Ty value) {
        const auto unsigned_value = static_cast<std::uint32_t>(value);
        if (unsigned_value == 0) {
            return -1;
        }
        return static_cast<std::int64_t>(std::countr_zero(unsigned_value));
    }

    template <typename Ty>
    std::int64_t FindMsb(Ty value) {
        if constexpr (std::is_signed_v<Ty>) {
            const auto signed_value = static_cast<std::int32_t>(value);
            if (signed_value == 0 || signed_value == -1)
                return -1;
            if (signed_value < 0)
                return static_cast<std::int64_t>(31 - std::countl_zero(static_cast<std::uint32_t>(~signed_value)));
            return static_cast<std::int64_t>(31 - std::countl_zero(static_cast<std::uint32_t>(signed_value)));
        } else {
            const auto unsigned_value = static_cast<std::uint32_t>(value);
            if (unsigned_value == 0) {
                return -1;
            }
            return static_cast<std::int64_t>(31 - std::countl_zero(unsigned_value));
        }
    }

    template <typename Ty>
    Ty BitfieldReverse(Ty value) {
        auto bit_pattern = static_cast<std::uint32_t>(value);

        bit_pattern = ((bit_pattern >> 1) & 0x55555555u) | ((bit_pattern & 0x55555555u) << 1);
        bit_pattern = ((bit_pattern >> 2) & 0x33333333u) | ((bit_pattern & 0x33333333u) << 2);
        bit_pattern = ((bit_pattern >> 4) & 0x0F0F0F0Fu) | ((bit_pattern & 0x0F0F0F0Fu) << 4);
        bit_pattern = ((bit_pattern >> 8) & 0x00FF00FFu) | ((bit_pattern & 0x00FF00FFu) << 8);
        bit_pattern = (bit_pattern >> 16) | (bit_pattern << 16);

        if constexpr (std::is_signed_v<Ty>) {
            return static_cast<Ty>(static_cast<std::int32_t>(bit_pattern));
        } else {
            return static_cast<Ty>(bit_pattern);
        }
    }

    template <typename Ty>
    Ty BitfieldExtract(Ty value, std::int64_t offset, std::int64_t bits) {
        if constexpr (std::is_signed_v<Ty>) {
            auto int_value = static_cast<std::int32_t>(value);
            int_value = (int_value << (32 - (offset + bits))) >> (32 - bits);
            return static_cast<Ty>(int_value);
        } else {
            auto uint_value = static_cast<std::uint32_t>(value);
            uint_value = (uint_value << (32 - (offset + bits))) >> (32 - bits);
            return static_cast<Ty>(uint_value);
        }
    }

    template <typename Ty>
    Ty BitfieldInsert(Ty base, Ty insert, std::int64_t offset, std::int64_t bits) {
        const std::uint32_t mask        = (bits == 32) ? 0xFFFFFFFFu : ((1u << bits) - 1u);
        const std::uint32_t base_bits   = static_cast<std::uint32_t>(base);
        const std::uint32_t insert_bits = static_cast<std::uint32_t>(insert) & mask;
        const std::uint32_t field_mask  = mask << offset;

        const std::uint32_t result = (base_bits & ~field_mask) | (insert_bits << offset);
        return static_cast<Ty>(result);
    }

    // 64位乘法展开 (umulExtended / imulExtended)
    // 乘法产生完整 64 位结果，返回低 32 位
    inline std::uint64_t UmulExtended(std::uint64_t multiplier, std::uint64_t multiplicand) {
        const auto u32_a = static_cast<std::uint64_t>(static_cast<std::uint32_t>(multiplier));
        const auto u32_b = static_cast<std::uint64_t>(static_cast<std::uint32_t>(multiplicand));
        return u32_a * u32_b; // 完整的无符号 64 位乘积
    }

    inline std::int64_t ImulExtended(std::int64_t multiplier, std::int64_t multiplicand) {
        const auto i32_a = static_cast<std::int64_t>(static_cast<std::int32_t>(multiplier));
        const auto i32_b = static_cast<std::int64_t>(static_cast<std::int32_t>(multiplicand));
        return i32_a * i32_b; // 完整的有符号 64 位乘积
    }

    // 带进位加法 (uaddCarry: 返回 low32 | carry)
    inline std::uint64_t UaddCarry(std::uint64_t lhs, std::uint64_t rhs) {
        const auto u32_lhs = static_cast<std::uint32_t>(lhs);
        const auto u32_rhs = static_cast<std::uint32_t>(rhs);
        const std::uint64_t sum = static_cast<std::uint64_t>(u32_lhs) + static_cast<std::uint64_t>(u32_rhs);
        return sum; // [32位进位 : 32位加和结果]
    }

    // 带借位减法 (usubBorrow: 返回 diff | borrow)
    inline std::uint64_t UsubBorrow(std::uint64_t lhs, std::uint64_t rhs) {
        const auto u32_lhs = static_cast<std::uint32_t>(lhs);
        const auto u32_rhs = static_cast<std::uint32_t>(rhs);
        const std::uint32_t diff   = u32_lhs - u32_rhs;
        const std::uint64_t borrow = (u32_lhs < u32_rhs) ? 1u : 0u;
        return (borrow << 32) | diff;
    }
#pragma endregion
}
