#include "ConstantEvaluator.hpp"

#include <format>
#include <limits>

namespace glsld {
    template<typename Ty>
    requires std::same_as<Ty, std::int64_t>
          || std::same_as<Ty, std::uint64_t>
          || std::same_as<Ty, double>
          || std::same_as<Ty, bool>
          || std::same_as<Ty, std::string>
    std::optional<Ty> ConstantEvaluator::EvaluateAs(ExpressionNode* node) {
        auto result = Evaluate(node);
        if (!result.has_value()) {
            return std::nullopt;
        }

        if constexpr (std::same_as<Ty, std::string>) {
            return std::visit([](auto&& arg) -> std::string {
                using Ty = std::decay_t<decltype(arg)>;
                if constexpr (std::same_as<Ty, std::int64_t>) {
                    return std::to_string(arg);
                } else if constexpr (std::same_as<Ty, std::uint64_t>) {
                    return std::to_string(arg);
                } else if constexpr (std::same_as<Ty, double>) {
                    return std::format("{:g}", arg);
                } else if constexpr (std::same_as<Ty, bool>) {
                    return arg ? "true" : "false";
                }
            }, *result);
        } else {
            if (auto value = std::get_if<Ty>(&*result)) {
                return *value;
            }

            if constexpr (std::same_as<Ty, std::uint64_t>) {
                if (auto value = std::get_if<std::int64_t>(&*result)) {
                    if (*value >= 0) {
                        return static_cast<std::uint64_t>(*value);
                    }
                }
            }

            if constexpr (std::same_as<Ty, std::int64_t>) {
                if (auto value = std::get_if<std::uint64_t>(&*result)) {
                    if (*value <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
                        return static_cast<std::int64_t>(*value);
                    }
                }
            }

            if constexpr (std::same_as<Ty, double>) {
                if (auto value = std::get_if<std::int64_t>(&*result))
                    return static_cast<double>(*value);
                if (auto value = std::get_if<std::uint64_t>(&*result))
                    return static_cast<double>(*value);
            }

            if constexpr (std::same_as<Ty, bool>) {
                if (auto value = std::get_if<std::int64_t>(&*result))
                    return static_cast<bool>(*value);
                if (auto value = std::get_if<std::uint64_t>(&*result))
                    return static_cast<bool>(*value);
            }

            return std::nullopt;
        }
    }
}
