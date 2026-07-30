#include "pch.hpp"
#include "Formatter.hpp"

#include <format>
#include <fstream>
#include <utility>

#include "Base/FileSystem/Source.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    Formatter::Formatter(FormatterConfig config)
        : config_{ std::move(config) }
    {}

    std::expected<std::string, std::string> Formatter::Format(
        std::string_view source,
        const std::filesystem::path& filename) const
    {
        FormatterConfig config;
        {
            std::lock_guard lock(mutex_);
            config = config_;
        }

        auto temporary = filename.parent_path() / (filename.filename().string() + ".cpp");

        std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
        stream.write(source.data(), source.size());
        if (!stream) {
            std::filesystem::remove(temporary);
            return std::unexpected{ "Failed to write temporary file for formatting." };
        }

        stream.flush();
        stream.close();

        std::string command;

        command = std::format(
            "\"{}\" --style=file -fallback-style=Microsoft -i \"{}\"",
            config.executable.string(),
            temporary.string());

        (void)utils::ExecuteCommand(command, static_cast<int>(config.timeout.count()));
        auto formatted = LoadSource(temporary);
        std::filesystem::remove(temporary);
        return formatted;
    }

    void Formatter::set_config(FormatterConfig config) {
        std::lock_guard lock(mutex_);
        config_ = std::move(config);
    }
}
