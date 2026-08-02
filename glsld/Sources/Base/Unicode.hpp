#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace glsld {
    struct Utf8CodePoint {
        char32_t    value{ 0xFFFD };
        std::size_t byte_count{ 1 };
        std::size_t utf16_units{ 1 };
        bool        valid{};
    };

    constexpr bool IsAsciiAlpha(unsigned char ch) noexcept {
        return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
    }

    constexpr bool IsAsciiDigit(unsigned char ch) noexcept {
        return (ch >= '0' && ch <= '9');
    }

    constexpr bool IsAsciiAlnum(unsigned char ch) noexcept {
        return IsAsciiAlpha(ch) || IsAsciiDigit(ch);
    }

    constexpr bool IsAsciiSpace(unsigned char ch) noexcept {
        return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\v' || ch == '\f';
    }

    constexpr bool IsContinuationByte(unsigned char ch) noexcept {
        return (ch & 0xC0) == 0x80;
    }

    Utf8CodePoint DecodeUtf8(std::string_view text) noexcept;
    bool IsValidUtf8(std::string_view text) noexcept;
    std::size_t Utf16Length(std::string_view text) noexcept;
    std::size_t Utf16OffsetToByteOffset(std::string_view text, std::size_t utf16_offset) noexcept;
    std::string SanitizeUtf8(std::string_view text) noexcept;

    class PositionMapper {
    public:
        explicit PositionMapper(std::string_view source);

        std::uint32_t ToByteColumn(std::uint32_t zero_based_line, std::uint32_t utf16_character) const;
        std::uint32_t ToUtf16Character(std::uint32_t one_based_line, std::uint32_t one_based_byte_column) const;

    private:
        std::string_view GetLine(std::uint32_t zero_based_line) const;

        std::string_view         source_;
        std::vector<std::size_t> line_starts_;
    };
}
