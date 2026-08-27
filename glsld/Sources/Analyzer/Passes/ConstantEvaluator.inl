#include "ConstantEvaluator.hpp"

#include <cmath>
#include <format>
#include <limits>
#include <type_traits>
#include <utility>

namespace glsld {
    template<typename Ty>
    requires std::same_as<Ty, std::int64_t>
          || std::same_as<Ty, std::uint64_t>
          || std::same_as<Ty, double>
          || std::same_as<Ty, bool>
          || std::same_as<Ty, std::string>
    std::optional<Ty> ConstantEvaluator::EvaluateAs(ExpressionNode* node) {
        const auto result = Evaluate(node);
        if (!result.has_value()) {
            return std::nullopt;
        }

        if constexpr (std::same_as<Ty, std::string>) {
            return FormatValue(*result);
        } else {
            const auto* scalar = std::get_if<ScalarValue>(&*result);
            if (scalar == nullptr) {
                return std::nullopt;
            }

            if (const auto* value = std::get_if<Ty>(scalar)) {
                return *value;
            }

            if constexpr (std::same_as<Ty, std::uint64_t>) {
                if (const auto* value = std::get_if<std::int64_t>(scalar)) {
                    if (*value >= 0) {
                        return static_cast<std::uint64_t>(*value);
                    }
                }
            }

            if constexpr (std::same_as<Ty, std::int64_t>) {
                if (const auto* value = std::get_if<std::uint64_t>(scalar)) {
                    if (*value <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
                        return static_cast<std::int64_t>(*value);
                    }
                }
            }

            if constexpr (std::same_as<Ty, double>) {
                if (const auto* value = std::get_if<std::int64_t>(scalar))
                    return static_cast<double>(*value);
                if (const auto* value = std::get_if<std::uint64_t>(scalar))
                    return static_cast<double>(*value);
            }

            if constexpr (std::same_as<Ty, bool>) {
                if (const auto* value = std::get_if<std::int64_t>(scalar))
                    return static_cast<bool>(*value);
                if (const auto* value = std::get_if<std::uint64_t>(scalar))
                    return static_cast<bool>(*value);
            }

            return std::nullopt;
        }
    }

    inline void ConstantEvaluator::Register(std::string_view name, EvaluatorFunc func) {
        registry_[name] = std::move(func);
    }
}
