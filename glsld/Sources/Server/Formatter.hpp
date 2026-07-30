#pragma once

#include <chrono>
#include <expected>
#include <filesystem>
#include <mutex>
#include <string>
#include <string_view>

namespace glsld {
    struct FormatterConfig {
        std::filesystem::path executable{ "clang-format.exe" };
        std::filesystem::path style_file{ ".clang-format" };
        std::chrono::milliseconds timeout{ 10'000 };
    };

    class Formatter {
    public:
        Formatter() = default;
        Formatter(FormatterConfig config);

        std::expected<std::string, std::string> Format(
            std::string_view source,
            const std::filesystem::path& filename) const;

        void set_config(FormatterConfig config);

    private:
        mutable std::mutex mutex_;
        FormatterConfig    config_;
    };
}
