#pragma once

#include <cstddef>
#include <concepts>
#include <string>
#include <string_view>

#include <ankerl/unordered_dense.h>
#include <rapidhash.h>

namespace glsld {
    struct StringViewHeteroEqual {
        using is_transparent = void;

        bool operator()(std::string_view lhs, std::string_view rhs) const {
            return lhs == rhs;
        }
    };

    struct StringViewHeteroHash {
        using is_transparent = void;

        std::size_t operator()(std::string_view key) const {
            return rapidhashMicro(key.data(), key.size());
        }
    };

    template <typename Value>
    using StringHeteroHashMap = ankerl::unordered_dense::map<std::string, Value, StringViewHeteroHash, StringViewHeteroEqual>;
    using StringHeteroHashSet = ankerl::unordered_dense::set<std::string,        StringViewHeteroHash, StringViewHeteroEqual>;

    template <typename Ty>
    void HashCombine(std::size_t& seed, const Ty& value) {
        auto hash = 0uz;
        if constexpr (std::same_as<Ty, std::string> || std::same_as<Ty, std::string_view>) {
            hash = rapidhashMicro(value.data(), value.size());
        } else {
            std::hash<Ty> hasher;
            hash = hasher(value);
        }

        seed ^= hash + 0x9e3779b97f4a7c15uz + (seed << 6) + (seed >> 2);
    }
}
