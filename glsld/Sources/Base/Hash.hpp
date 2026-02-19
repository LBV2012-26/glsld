#pragma once

#include <cstddef>
#include <concepts>
#include <string>
#include <string_view>
#include <unordered_map>

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
            return std::hash<std::string_view>()(key);
        }
    };

    template <typename Key, typename Value>
    requires std::same_as<Key, std::string>
    using StringHeteroHashTable = std::unordered_map<Key, Value, StringViewHeteroHash, StringViewHeteroEqual>;

    template <typename Ty>
    inline void HashCombine(std::size_t& seed, const Ty& value) {
        std::hash<Ty> hasher;
        seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}
