#pragma once

#include <cstddef>
#include <algorithm>
#include <array>
#include <string_view>

#include "Base/Config.hpp"
#include "Base/Logger.hpp"

namespace glsld {
    template <std::size_t Nx>
    struct FixedString {
        std::array<char, Nx> data{};
        std::size_t          size{ Nx - 1 };

        constexpr FixedString(const char(&str)[Nx]) {
            std::ranges::copy_n(str, Nx, data.data());
        }

        operator std::string_view() const {
            return std::string_view(data.data(), size);
        }

        auto operator<=>(const FixedString&) const = default;
    };

    template <typename Ty, FixedString Name, const Ty& DefaultValue, FixedString Description>
    struct ConfigIniter {
        inline static Ty value{ DefaultValue };

        auto GetConfig() {
            static auto config = Config::Lookup<Ty>(Name, DefaultValue, Description);
            return config;
        }

        ConfigIniter() {
            auto config = GetConfig();
            value = config->value();

            config->AddListener(reinterpret_cast<std::uintptr_t>(this),
            [](const auto&, const auto& new_value) -> void {
                value = new_value;
                GLSLD_LOG_INFO(GLSLD_LOG_ROOT(), "Configuration updated: {} = {}", std::string_view(Name), new_value);
            });
        }
    };
}

#define INIT_CONFIG(name, config_name, default_value, description)                                       \
    constexpr decltype(default_value) k##name##DefaultValue = default_value;                             \
    using name##Initer =                                                                                 \
        ::glsld::ConfigIniter<decltype(default_value), config_name, k##name##DefaultValue, description>; \
    static name##Initer k##name##Initer;
