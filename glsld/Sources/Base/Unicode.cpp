#include "pch.hpp"
#include "Unicode.hpp"

#include <algorithm>

namespace glsld {
    Utf8CodePoint DecodeUtf8(std::string_view text) noexcept {
        if (text.empty()) {
            return {
                .byte_count  = 0,
                .utf16_units = 0
            };
        }

        auto first = static_cast<unsigned char>(text.front());
        if (first <= 0x7F) {
            return {
                .value       = static_cast<char32_t>(first),
                .byte_count  = 1,
                .utf16_units = 1,
                .valid       = true
            };
        }

        auto byte_count  = 0uz;
        char32_t value   = 0;
        char32_t minimum = 0;

        if ((first & 0xE0) == 0xC0) {
            byte_count = 2;
            value      = first & 0x1F;
            minimum    = 0x80;
        } else if ((first & 0xF0) == 0xE0) {
            byte_count = 3;
            value      = first & 0x0F;
            minimum    = 0x800;
        } else if ((first & 0xF8) == 0xF0) {
            byte_count = 4;
            value      = first & 0x07;
            minimum    = 0x10000;
        } else {
            return {};
        }

        if (text.size() < byte_count) {
            return {};
        }

        for (auto i = 1uz; i != byte_count; ++i) {
            auto byte = static_cast<unsigned char>(text[i]);
            if (!IsContinuationByte(byte)) {
                return {};
            }

            value = (value << 6) | (byte & 0x3F);
        }

        if (value < minimum || value > 0x10FFFF || (value >= 0xD800 && value <= 0xDFFF)) {
            return {};
        }

        return {
            .value       = value,
            .byte_count  = byte_count,
            .utf16_units = value > 0xFFFF ? 2uz : 1uz,
            .valid       = true
        };
    }

    bool IsValidUtf8(std::string_view text) noexcept {
        auto offset = 0uz;
        while (offset < text.size()) {
            auto point = DecodeUtf8(text.substr(offset));
            if (!point.valid) {
                return false;
            }

            offset += point.byte_count;
        }

        return true;
    }

    std::size_t Utf16Length(std::string_view text) noexcept {
        auto offset = 0uz;
        auto length = 0uz;

        while (offset < text.size()) {
            auto point = DecodeUtf8(text.substr(offset));
            offset += point.byte_count;
            length += point.utf16_units;
        }

        return length;
    }

    std::size_t Utf16OffsetToByteOffset(std::string_view text, std::size_t utf16_offset) noexcept {
        auto byte_offset          = 0uz;
        auto current_utf16_offset = 0uz;

        while (byte_offset < text.size()) {
            auto point = DecodeUtf8(text.substr(byte_offset));
            if (current_utf16_offset + point.utf16_units > utf16_offset) {
                break;
            }

            byte_offset          += point.byte_count;
            current_utf16_offset += point.utf16_units;

            if (current_utf16_offset == utf16_offset) {
                break;
            }
        }

        return byte_offset;
    }

    std::string SanitizeUtf8(std::string_view text) noexcept {
        if (text.starts_with("\xEF\xBB\xBF")) {
            text.remove_prefix(3);
        }

        auto input_offset = 0uz;
        auto output_size  = 0uz;
        bool changed      = false;

        while (input_offset < text.size()) {
            auto point = DecodeUtf8(text.substr(input_offset));
            input_offset += point.byte_count;
            output_size  += point.valid ? point.byte_count : 3;
            changed |= !point.valid;
        }

        if (!changed) {
            return std::string(text);
        }

        std::string result;
        result.resize_and_overwrite(output_size, [&](char* buffer, std::size_t) -> std::size_t {
            auto src_offset = 0uz;
            auto dst_offset = 0uz;

            while (src_offset < text.size()) {
                auto point = DecodeUtf8(text.substr(src_offset));
                if (point.valid) {
                    std::ranges::copy(text.substr(src_offset, point.byte_count), buffer + dst_offset);
                    dst_offset += point.byte_count;
                } else {
                    buffer[dst_offset++] = static_cast<char>(0xEF);
                    buffer[dst_offset++] = static_cast<char>(0xBF);
                    buffer[dst_offset++] = static_cast<char>(0xBD);
                }

                src_offset += point.byte_count;
            }

            return dst_offset;
        });

        return result;
    }

    PositionMapper::PositionMapper(std::string_view source)
        : source_{ source }
    {
        line_starts_.push_back(0);

        for (auto i = 0uz; i != source_.size(); ++i) {
            if (source_[i] == '\n') {
                line_starts_.push_back(i + 1);
            }
        }
    }

    std::uint32_t PositionMapper::ToByteColumn(std::uint32_t zero_based_line, std::uint32_t utf16_character) const {
        auto line = GetLine(zero_based_line);
        return static_cast<std::uint32_t>(Utf16OffsetToByteOffset(line, utf16_character) + 1);
    }

    std::uint32_t PositionMapper::ToUtf16Character(std::uint32_t one_based_line, std::uint32_t one_based_byte_column) const {
        auto line = GetLine(one_based_line - 1);
        auto byte_offset = std::min(one_based_byte_column - 1, static_cast<std::uint32_t>(line.length()));
        return static_cast<std::uint32_t>(Utf16Length(line.substr(0, byte_offset)));
    }

    std::string_view PositionMapper::GetLine(std::uint32_t zero_based_line) const {
        if (zero_based_line >= line_starts_.size()) {
            return {};
        }

        auto begin = line_starts_[zero_based_line];
        auto end   = zero_based_line + 1 < line_starts_.size()
                   ? line_starts_[zero_based_line + 1] - 1
                   : source_.length();
        auto line  = source_.substr(begin, end - begin);

        if (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1);
        }

        return line;
    }
}
