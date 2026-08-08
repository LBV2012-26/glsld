#include "pch.hpp"
#include "Formatter.hpp"

#include <format>
#include <mutex>
#include <utility>

#include "Base/FileSystem/Source.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    std::string Formatter::Format(std::string_view source, const std::filesystem::path& filename) const {
        std::string clang_format_path;
        {
            std::shared_lock lock(mutex_);
            clang_format_path = clang_format_path_;
        }

        auto command = std::format(
            "\"{}\" --style=file --fallback-style=Microsoft --assume-filename=main.cpp",
            clang_format_path);

        int  exit_code = -1;
        auto formatted = utils::ExecuteCommand(command, filename.parent_path().generic_string(), source, 10'000, &exit_code);

        if (exit_code != 0 || formatted.empty()) {
            return {};
        }

        return formatted;
    }

    std::string Formatter::FormatRange(
        std::string_view source,
        const std::filesystem::path& filename,
        std::size_t start_line,
        std::size_t end_line) const
    {
        std::string clang_format_path;
        {
            std::shared_lock lock(mutex_);
            clang_format_path = clang_format_path_;
        }

        auto command = std::format(
            "\"{}\" --style=file --fallback-style=Microsoft --assume-filename=main.cpp "
            "--lines={}:{}",
            clang_format_path, start_line, end_line);

        int  exit_code = -1;
        auto formatted = utils::ExecuteCommand(command, filename.parent_path().generic_string(), source, 10'000, &exit_code);

        if (exit_code != 0 && formatted.empty()) {
            return {};
        }

        return formatted;
    }

    std::string Formatter::FormatSnippet(std::string_view source, const std::filesystem::path& filename) const {
        std::string clang_format_path;
        {
            std::shared_lock lock(mutex_);
            clang_format_path = clang_format_path_;
        }

        auto command   = std::format("\"{}\" --style=file --fallback-style=Microsoft --assume-filename=main.cpp", clang_format_path);
        int  exit_code = -1;
        auto formatted = utils::ExecuteCommand(command, filename.parent_path().generic_string(), source, 1'000, &exit_code);

        if (exit_code != 0 || formatted.empty()) {
            return {};
        }

        while (!formatted.empty() && (formatted.back() == '\r' || formatted.back() == '\n')) {
            formatted.pop_back();
        }

        return formatted;
    }

    void Formatter::set_clang_format_path(const std::filesystem::path& filename) {
        std::lock_guard lock(mutex_);
#ifdef _WIN64
        auto default_path = "clang-format.exe";
#else
        auto default_path = "clang-format";
#endif
        clang_format_path_ = filename.empty() ? default_path : filename.generic_string();
    }
}
