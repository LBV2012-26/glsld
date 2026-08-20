#pragma once

#include <cstddef>
#include <filesystem>
#include <shared_mutex>
#include <string>
#include <string_view>

namespace glsld {
    class Formatter {
    public:
        std::string Format(std::string_view source, const std::filesystem::path& filename) const;

        std::string FormatRange(
            std::string_view source,
            const std::filesystem::path& filename,
            std::size_t start_line,
            std::size_t end_line) const;

        std::string FormatSnippet(std::string_view source, const std::filesystem::path& filename) const;

        void set_clang_format_path(const std::filesystem::path& filename);

    private:
        mutable std::shared_mutex mutex_;
#ifdef _WIN64
        std::string clang_format_path_{ "clang-format.exe" };
#else
        std::string clang_format_path_{ "clang-format" };
#endif
    };
}
