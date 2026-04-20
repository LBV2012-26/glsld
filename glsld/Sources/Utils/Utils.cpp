#include "stdafx.h"
#include "Utils.hpp"

#include <cctype>
#include <array>
#include <format>
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

    namespace {
        std::string SerializeSpirvArguments(const SpirvArgumentNode* argument) {
            if (argument == nullptr) {
                return {};
            }

            switch (argument->arg_kind) {
            case SpirvArgumentKind::kIdentifier:
            case SpirvArgumentKind::kNumberLiteral:
            case SpirvArgumentKind::kStringLiteral:
            case SpirvArgumentKind::kBoolLiteral:
            case SpirvArgumentKind::kUnknown:
                return argument->token.text;

            case SpirvArgumentKind::kAssignment: {
                std::string lhs = argument->children.size() > 0 ? SerializeSpirvArguments(argument->children[0].get()) : "";
                std::string rhs = argument->children.size() > 1 ? SerializeSpirvArguments(argument->children[1].get()) : "";
                return std::format("{} = {}", lhs, rhs);
            }

            case SpirvArgumentKind::kArray: {
                std::string result = "[";
                for (auto i = 0uz; i != argument->children.size(); ++i) {
                    result += SerializeSpirvArguments(argument->children[i].get());
                    if (i + 1 != argument->children.size()) {
                        result += ", ";
                    }
                }

                result += "]";
                return result;
            }

            case SpirvArgumentKind::kGroup: {
                std::string result = "(";
                for (auto i = 0uz; i != argument->children.size(); ++i) {
                    result += SerializeSpirvArguments(argument->children[i].get());
                    if (i + 1 != argument->children.size()) {
                        result += ", ";
                    }
                }

                result += ")";
                return result;
            }

            case SpirvArgumentKind::kSequence: {
                std::string result;
                for (auto i = 0uz; i != argument->children.size(); ++i) {
                    result += SerializeSpirvArguments(argument->children[i].get());
                    if (i + 1 != argument->children.size()) {
                        result += " ";
                    }
                }

                return result;
            }
            }

            return {};
        }
    }

    std::string BuildSpirvTypeIdentity(const SpirvIntrinsicNode* spirv_type) {
        if (spirv_type == nullptr) {
            return {};
        }

        std::string params;
        for (auto i = 0uz; i != spirv_type->params.size(); ++i) {
            params += SerializeSpirvArguments(spirv_type->params[i].get());
            if (i + 1 != spirv_type->params.size()) {
                params += ", ";
            }
        }

        return std::format("spirv_type({})", params);
    }
}
