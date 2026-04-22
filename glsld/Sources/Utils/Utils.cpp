#include "stdafx.h"
#include "Utils.hpp"

#include <cctype>
#include <array>
#include <charconv>
#include <format>
#include <limits>
#include <system_error>

#include <Windows.h>
#include "Analyzer/Ast/Ast.hpp"

namespace glsld::utils {
    namespace {
        int HexToInt(char hex) {
            if (hex >= '0' && hex <= '9') {
                return hex - '0';
            } else if (hex >= 'a' && hex <= 'f') {
                return hex - 'a' + 10;
            } else if (hex >= 'A' && hex <= 'F') {
                return hex - 'A' + 10;
            } else {
                return -1;
            }
        }

        std::string PercentDecode(std::string_view input) {
            std::string result;
            result.reserve(input.size());

            for (auto i = 0uz; i < input.size(); ++i) {
                if (input[i] == '%' && i + 2 < input.size()) {
                    int high = HexToInt(input[i + 1]);
                    int low  = HexToInt(input[i + 2]);
                    if (high != -1 && low != -1) {
                        result += static_cast<char>((high << 4) | low);
                        i += 2;
                        continue;
                    }
                }

                result.push_back(input[i]);
            }

            return result;
        }
    }

    std::string GetFilePath(std::string_view filename) {
        std::array<wchar_t, MAX_PATH> buffer{};
        GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));

        auto work_directory = std::filesystem::path(buffer.data()).parent_path().parent_path();
        auto file_path = work_directory / std::filesystem::path(filename);

        return file_path.generic_string();
    }

    std::filesystem::path UriToPath(std::string_view uri) {
        constexpr std::string_view kPrefix = "file://";

        if (!uri.starts_with(kPrefix)) {
            return NormalizePath(std::filesystem::path(uri));
        }

        auto raw     = uri.substr(kPrefix.size());
        auto decoded = PercentDecode(raw);

        if (decoded.size() >= 3 && decoded[0] == '/' &&
            std::isalpha(static_cast<unsigned char>(decoded[1])) && decoded[2] == ':')
        {
            decoded.erase(decoded.begin()); // /C:/Path -> C:/Path
        }

        return NormalizePath(std::filesystem::path(decoded));
    }

    std::string PathToUri(const std::filesystem::path& path) {
        auto normalized = NormalizePath(path).generic_string();

        std::string encoded;
        encoded.reserve(normalized.size() + 16);
        for (auto ch : normalized) {
            if (std::isalnum(static_cast<unsigned char>(ch)) ||
                ch == '/' || ch == '-' || ch == '_' || ch == '.' || ch == '~' || ch == ':')
            {
                encoded.push_back(ch);
            } else {
                encoded += '%' + std::format("{:02X}", static_cast<unsigned char>(ch));
            }
        }

        return "file:///" + encoded;
    }

    std::filesystem::path NormalizePath(const std::filesystem::path& path) {
        std::error_code ec;
        auto normalized = std::filesystem::weakly_canonical(path, ec);
        if (ec) {
            return path.lexically_normal();
        }

        return normalized.lexically_normal();
    }

    std::string_view UnmangleFunctionName(std::string_view mangled_name) {
        std::string_view raw_name = mangled_name;
        // __Impl_main(void) -> main
        if (raw_name.starts_with("__Decl_") || raw_name.starts_with("__Impl_")) {
            raw_name = raw_name.substr(7);
            auto paren = raw_name.find('(');
            if (paren != std::string_view::npos) {
                raw_name = raw_name.substr(0, paren);
            }
        } else {
            return {};
        }

        return raw_name;
    }

    void PrintIndent(int level) {
        for (int i = 0; i < level; ++i) {
            std::print("  "); // 2 spaces per indent level
        }
    }

    std::string SerializeQualifierArguments(const QualifierArgumentNode* argument) {
        if (argument == nullptr) {
            return {};
        }

        switch (argument->arg_kind) {
        case QualifierArgumentKind::kIdentifier:
        case QualifierArgumentKind::kNumberLiteral:
        case QualifierArgumentKind::kStringLiteral:
        case QualifierArgumentKind::kBoolLiteral:
        case QualifierArgumentKind::kUnknown:
            return argument->token.text;

        case QualifierArgumentKind::kAssignment: {
            std::string lhs = argument->children.size() > 0 ? SerializeQualifierArguments(argument->children[0].get()) : "";
            std::string rhs = argument->children.size() > 1 ? SerializeQualifierArguments(argument->children[1].get()) : "";
            return std::format("{} = {}", lhs, rhs);
        }

        case QualifierArgumentKind::kArray: {
            std::string result = "[";
            for (auto i = 0uz; i != argument->children.size(); ++i) {
                result += SerializeQualifierArguments(argument->children[i].get());
                if (i + 1 != argument->children.size()) {
                    result += ", ";
                }
            }

            result += "]";
            return result;
        }

        case QualifierArgumentKind::kGroup: {
            std::string result = "(";
            for (auto i = 0uz; i != argument->children.size(); ++i) {
                result += SerializeQualifierArguments(argument->children[i].get());
                if (i + 1 != argument->children.size()) {
                    result += ", ";
                }
            }

            result += ")";
            return result;
        }

        case QualifierArgumentKind::kSequence: {
            std::string result;
            for (auto i = 0uz; i != argument->children.size(); ++i) {
                result += SerializeQualifierArguments(argument->children[i].get());
                if (i + 1 != argument->children.size()) {
                    result += " ";
                }
            }

            return result;
        }
        }

        return {};
    }

    std::int64_t ParseNumberLiteralToInteger(std::string_view text) {
        auto IsSuffix = [](char ch) -> bool {
            return ch == 'u' || ch == 'U' ||
                   ch == 'l' || ch == 'L' ||
                   ch == 's' || ch == 'S' ||
                   ch == 'f' || ch == 'F';
        };

        auto end = text.size();
        while (end > 0 && IsSuffix(text[end - 1])) {
            --end;
        }

        auto core = text.substr(0, end);
        if (core.empty()) {
            return 0;
        }

        bool maybe_float =
            core.find('.') != std::string_view::npos ||
            core.find('e') != std::string_view::npos ||
            core.find('E') != std::string_view::npos ||
            core.find('p') != std::string_view::npos ||
            core.find('P') != std::string_view::npos;

        if (maybe_float) {
            double float_value = 0.0;
            auto [ptr, ec] = std::from_chars(core.data(), core.data() + core.size(), float_value);
            if (ec == std::errc{} && ptr == core.data() + core.size() && std::isfinite(float_value)) {
                if (float_value >= static_cast<double>(std::numeric_limits<std::int64_t>::max()))
                    return std::numeric_limits<std::int64_t>::max();
                if (float_value <= static_cast<double>(std::numeric_limits<std::int64_t>::min()))
                    return std::numeric_limits<std::int64_t>::min();
                return static_cast<std::int64_t>(float_value);
            } else {
                return 0;
            }
        }

        bool negative = false;
        if (!core.empty() && (core.front() == '+' || core.front() == '-')) {
            // from_chars doesn't support leading +/- in integer
            negative = (core.front() == '-');
            core.remove_prefix(1);
        }

        if (core.empty()) {
            return 0;
        }

        int base = 10;
        if (core.size() >= 2 && core[0] == '0' && (core[1] == 'x' || core[1] == 'X')) {
            base = 16;
            core.remove_prefix(2);
        } else if (core.size() > 1 && core[0] == '0') {
            base = 8;
            // 八进制保留前导 0，from_chars(base=8) 可正常处理
        }

        if (core.empty()) {
            return 0;
        }

        std::uint64_t magnitude = 0;
        auto [ptr, ec] = std::from_chars(core.data(), core.data() + core.size(), magnitude, base);
        if (ec != std::errc{} || ptr != core.data() + core.size()) {
            return 0;
        }

        if (!negative) {
            if (magnitude > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
                return std::numeric_limits<std::int64_t>::max();
            };

            return static_cast<std::int64_t>(magnitude);
        }

        constexpr auto kMinAbsolute = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) + 1ull;
        if (magnitude > kMinAbsolute) {
            return std::numeric_limits<std::int64_t>::min();
        }

        return -static_cast<std::int64_t>(magnitude);
    }
}
