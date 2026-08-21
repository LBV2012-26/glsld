#include "pch.hpp"
#include "DiagnosticEngine.hpp"

#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <array>
#include <charconv>
#include <format>
#include <fstream>
#include <mutex>
#include <ranges>
#include <system_error>
#include <utility>

#include "Utils/Utils.hpp"

namespace glsld {
    namespace {
        std::string FindGlslc() {
            const char* vksdk = std::getenv("VULKAN_SDK");
            if (vksdk != nullptr) {
#ifdef _WIN64
                const auto path = std::filesystem::path(vksdk) / "Bin" / "glslc.exe";
#else
                const auto path = std::filesystem::path(vksdk) / "bin" / "glslc";
#endif
                if (std::filesystem::exists(path)) {
                    return path.string();
                }
            }

#ifdef _WIN64
            return "glslc.exe";
#else
            return "glslc";
#endif
        }
    }

    DiagnosticEngine::DiagnosticEngine()
        : glslc_path_{ FindGlslc() }
    {
        thread_ = std::jthread([this]() -> void { Run(); });
    }

    DiagnosticEngine::~DiagnosticEngine() {
        Stop();
    }

    void DiagnosticEngine::SetCallback(Callback callback) {
        callback_ = std::move(callback);
    }

    void DiagnosticEngine::Submit(DiagnosticTask task) {
        {
            std::lock_guard lock(mutex_);
            queue_.push(std::move(task));
        }
        condition_.notify_one();
    }

    void DiagnosticEngine::Stop() {
        stop_source_.request_stop();
        condition_.notify_all();

        if (thread_.joinable()) {
            thread_.join();
        }
    }

    void DiagnosticEngine::set_glslc_path(const std::filesystem::path& filename) {
        std::lock_guard lock(mutex_);
        glslc_path_ = filename.empty() ? FindGlslc() : filename.generic_string();
    }

    void DiagnosticEngine::Run() {
        const auto stop_token = stop_source_.get_token();
        while (!stop_token.stop_requested()) {
            DiagnosticTask task;
            {
                std::unique_lock lock(mutex_);
                condition_.wait(lock, stop_token, [this]() -> bool {
                    return !queue_.empty();
                });

                if (stop_token.stop_requested()) {
                    return;
                }

                task = std::move(queue_.front());
                queue_.pop();
            }

            if (task.version_replica != task.version_pointer->load(std::memory_order::relaxed)) {
                continue;
            }

            auto diagnostics = Compile(task);
            if (task.version_replica == task.version_pointer->load(std::memory_order::relaxed) && callback_) {
                callback_(task.uri, task.version_replica, std::move(diagnostics));
            }
        }
    }

    namespace {
        std::vector<std::string_view> SplitLines(std::string_view text) {
            return text
                | std::views::split('\n')
                | std::views::transform([](auto&& subrange) -> std::string_view {
                      return std::string_view(subrange);
                  })
                | std::ranges::to<std::vector<std::string_view>>();
        }

        std::string_view Trim(std::string_view text) {
            const auto begin = text.find_first_not_of(" \t");
            if (begin == std::string_view::npos) {
                return {};
            }

            const auto end = text.find_last_not_of(" \t\r");
            return text.substr(begin, end - begin + 1);
        }

        bool TryParseInteger(std::string_view text, int& result) {
            const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), result);
            return ec == std::errc() && ptr == text.data() + text.size();
        }

        std::string_view ExtractSymbol(std::string_view message) {
            const auto begin = message.find('\'');
            if (begin == std::string_view::npos) {
                return {};
            }

            const auto end = message.find('\'', begin + 1);
            if (end == std::string_view::npos) {
                return {};
            }

            return message.substr(begin + 1, end - begin - 1);
        }

        struct ColumnRange {
            int column{};
            int end{};
        };

        ColumnRange LocateSymbol(std::string_view source, std::string_view symbol) {
            if (symbol.empty() || source.empty()) {
                return { 0, static_cast<int>(source.size()) };
            }

            const auto pos = source.find(symbol);
            if (pos == std::string_view::npos) {
                return { 0, static_cast<int>(source.size()) };
            }

            return { static_cast<int>(pos), static_cast<int>(pos + symbol.size()) };
        }

        std::string_view GetFilename(std::string_view path) {
            const auto last_slash = path.find_last_of("\\/");
            if (last_slash == std::string_view::npos) {
                return path;
            }
            return path.substr(last_slash + 1);
        }

        bool EqualsIgnoreCase(std::string_view lhs, std::string_view rhs) {
            if (lhs.size() != rhs.size()) {
                return false;
            }

            for (auto i = 0uz; i < lhs.size(); ++i) {
                if (std::tolower(static_cast<unsigned char>(lhs[i])) !=
                    std::tolower(static_cast<unsigned char>(rhs[i])))
                {
                    return false;
                }
            }

            return true;
        }

        bool ContainsIgnoreCase(std::string_view haystack, std::string_view needle) {
            if (needle.empty())
                return true;
            if (haystack.size() < needle.size())
                return false;

            for (auto i = 0uz; i <= haystack.size() - needle.size(); ++i) {
                bool match = true;
                for (auto j = 0uz; j < needle.size(); ++j) {
                    if (std::tolower(static_cast<unsigned char>(haystack[i + j])) !=
                        std::tolower(static_cast<unsigned char>(needle[j])))
                    {
                        match = false;
                        break;
                    }
                }

                if (match) {
                    return true;
                }
            }

            return false;
        }

        std::vector<Diagnostic> ParseErrorOutput(
            std::string_view error,
            std::string_view filename,
            std::string_view source)
        {
            std::vector<Diagnostic> results;
            const auto source_lines  = SplitLines(source);
            const auto filename_base = GetFilename(filename);

            struct SeverityPattern {
                std::string_view   pattern;
                DiagnosticSeverity severity;
            };

            static constexpr std::array<SeverityPattern, 3> kPatterns{ {
                { ": error:",       DiagnosticSeverity::kError },
                { ": warning:",     DiagnosticSeverity::kWarning },
                { ": fatal error:", DiagnosticSeverity::kError }
            } };

            for (auto raw : SplitLines(error)) {
                const auto line = Trim(raw);
                if (line.empty()) {
                    continue;
                }

                // 寻找严重性标识符
                auto severity_pos   = std::string_view::npos;
                auto severity       = DiagnosticSeverity::kError;
                auto pattern_length = 0uz;

                for (const auto& pattern : kPatterns) {
                    auto pos = line.find(pattern.pattern);
                    if (pos != std::string_view::npos) {
                        if (severity_pos == std::string_view::npos || pos < severity_pos) {
                            severity_pos   = pos;
                            severity       = pattern.severity;
                            pattern_length = pattern.pattern.size();
                        }
                    }
                }

                // 如果不包含任何已知严重性前缀，说明是类似 "1 error generated." 的非诊断行，直接跳过
                if (severity_pos == std::string_view::npos) {
                    continue;
                }

                // 拆分前缀部分 [Path]:[Line] 与 消息部分 [Message]
                const auto prefix  = Trim(line.substr(0, severity_pos));
                const auto message = Trim(line.substr(severity_pos + pattern_length));

                int  zero_based_line     = 0;
                bool is_valid_diagnostic = false;

                // 从前缀末尾解析行号
                const auto last_colon = prefix.find_last_of(':');
                if (last_colon != std::string_view::npos) {
                    const auto path_part = Trim(prefix.substr(0, last_colon));
                    const auto line_part = Trim(prefix.substr(last_colon + 1));

                    int line_num = 0;
                    if (TryParseInteger(line_part, line_num)) {
                        if (EqualsIgnoreCase(GetFilename(path_part), filename_base)) {
                            zero_based_line = line_num - 1;
                            if (zero_based_line < 0) {
                                zero_based_line = 0;
                            }

                            is_valid_diagnostic = true;
                        }
                    }
                }

                if (!is_valid_diagnostic) {
                    if (ContainsIgnoreCase(line, filename_base)) {
                        zero_based_line     = 0;
                        is_valid_diagnostic = true;
                    }
                }

                if (!is_valid_diagnostic) {
                    continue;
                }

                const auto symbol      = ExtractSymbol(message);
                const auto source_line = static_cast<std::size_t>(zero_based_line) < source_lines.size()
                                       ? source_lines[zero_based_line]
                                       : std::string_view{};

                const auto range = LocateSymbol(source_line, symbol);

                results.push_back(Diagnostic{
                    .line          = zero_based_line,
                    .character     = range.column,
                    .end_line      = zero_based_line,
                    .end_character = range.end,
                    .severity      = severity,
                    .message       = std::string(message)
                });
            }

            return results;
        }
    }

    std::vector<Diagnostic> DiagnosticEngine::Compile(const DiagnosticTask& task) {
        std::string glslc_path;
        {
            std::shared_lock lock(mutex_);
            glslc_path = glslc_path_;
        }

        const auto extension_name = std::filesystem::path(task.filename).extension().string();
        const auto compile_path   = (std::filesystem::temp_directory_path() / std::filesystem::path(task.filename).filename()).generic_string();

        std::ofstream(compile_path, std::ios::binary) << task.source;
        const auto target_path = std::format("{}.spv", compile_path);

        auto command = std::format("\"{}\" -o {} ", glslc_path, target_path);
        if (task.shader_stage.has_value()) {
            command += std::format("-fshader-stage={} -D_GLSLD ", *task.shader_stage);
        }

        command += std::format("-I \"{}\" ", std::filesystem::path(task.filename).parent_path().generic_string());
        for (const auto& dir : *task.include_dirs) {
            command += std::format("-I \"{}\" ", dir.generic_string());
        }

        command += std::format("\"{}\" ", compile_path);

        if (task.target_env.has_value())
            command += std::format("--target-env={} ", *task.target_env);
        if (task.target_spv.has_value())
            command += std::format("--target-spv={} ", *task.target_spv);

        const auto output = Utils::ExecuteCommand(command);
        std::filesystem::remove(compile_path);
        std::filesystem::remove(target_path);

        return ParseErrorOutput(output, task.filename, task.source);
    }
}
