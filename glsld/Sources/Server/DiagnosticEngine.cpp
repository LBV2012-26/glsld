#include "stdafx.h"
#include "DiagnosticEngine.hpp"

#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <array>
#include <charconv>
#include <format>
#include <fstream>
#include <ios>
#include <ranges>
#include <system_error>
#include <thread>
#include <tuple>
#include <utility>

namespace glsld {
    namespace {
        std::string FindGlslc() {
            char* vksdk = nullptr;
            auto length = 0uz;
            if (_dupenv_s(&vksdk, &length, "VULKAN_SDK") == 0 && vksdk != nullptr && length > 0) {
                auto path = std::format("{}/Bin/glslc.exe", vksdk);
                std::free(vksdk);

                if (std::filesystem::exists(path)) {
                    return path;
                }
            }

            return "glslc.exe";
        }
    }

    DiagnosticEngine::DiagnosticEngine()
        : glslc_path_{ FindGlslc() }
    {
        std::jthread thread([this]() -> void { Run(); });
        thread.detach();
    }

    DiagnosticEngine::~DiagnosticEngine() {
        running_.store(false, std::memory_order::relaxed);
        condition_.notify_all();
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

    void DiagnosticEngine::Run() {
        while (running_.load(std::memory_order::relaxed)) {
            DiagnosticTask task;
            {
                std::unique_lock lock(mutex_);
                condition_.wait(lock, [this]() -> bool {
                    return !queue_.empty() || !running_.load(std::memory_order::relaxed);
                });

                if (!running_.load(std::memory_order::relaxed)) {
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
            auto begin = text.find_first_not_of(" \t");
            if (begin == std::string_view::npos) {
                return {};
            }

            auto end = text.find_last_not_of(" \t\r");
            return text.substr(begin, end - begin + 1);
        }

        bool TryParseInteger(std::string_view text, int& result) {
            auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), result);
            return ec == std::errc() && ptr == text.data() + text.size();
        }

        std::string_view ExtractSymbol(std::string_view message) {
            auto begin = message.find('\'');
            if (begin == std::string_view::npos) {
                return {};
            }

            auto end = message.find('\'', begin + 1);
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

            auto pos = source.find(symbol);
            if (pos == std::string_view::npos) {
                return { 0, static_cast<int>(source.size()) };
            }

            return { static_cast<int>(pos), static_cast<int>(pos + symbol.size()) };
        }

        std::string_view GetFilename(std::string_view path) {
            auto last_slash = path.find_last_of("\\/");
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
                    std::tolower(static_cast<unsigned char>(rhs[i]))) {
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
                        std::tolower(static_cast<unsigned char>(needle[j]))) {
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
            auto source_lines  = SplitLines(source);
            auto filename_base = GetFilename(filename);

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
                auto line = Trim(raw);
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
                auto prefix  = Trim(line.substr(0, severity_pos));
                auto message = Trim(line.substr(severity_pos + pattern_length));

                int  zero_based_line     = 0;
                bool is_valid_diagnostic = false;

                // 从前缀末尾解析行号
                auto last_colon = prefix.find_last_of(':');
                if (last_colon != std::string_view::npos) {
                    auto path_part = Trim(prefix.substr(0, last_colon));
                    auto line_part = Trim(prefix.substr(last_colon + 1));

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

                auto symbol      = ExtractSymbol(message);
                auto source_line = static_cast<std::size_t>(zero_based_line) < source_lines.size()
                                 ? source_lines[zero_based_line]
                                 : std::string_view{};

                auto range = LocateSymbol(source_line, symbol);

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

        std::string ExecuteCommand(std::string_view command) {
            SECURITY_ATTRIBUTES attributes{ sizeof(attributes), nullptr, TRUE };
            HANDLE read  = nullptr;
            HANDLE write = nullptr;
            CreatePipe(&read, &write, &attributes, 0);

            STARTUPINFO startup{ sizeof(startup) };
            startup.dwFlags    = STARTF_USESTDHANDLES;
            startup.hStdOutput = write;
            startup.hStdError  = write;

            auto size = MultiByteToWideChar(CP_UTF8, 0, command.data(), -1, nullptr, 0);
            std::wstring wcommand;
            wcommand.resize_and_overwrite(size, [&](auto* buffer, std::size_t buffer_size) -> std::size_t {
                return MultiByteToWideChar(CP_UTF8, 0, command.data(), -1, buffer, static_cast<int>(buffer_size));
            });

            PROCESS_INFORMATION info{};
            auto ok = CreateProcess(nullptr, wcommand.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &info);

            CloseHandle(write);

            std::string output;
            if (ok) {
                WaitForSingleObject(info.hProcess, 30000);
                std::array<char, 4096> buffer{};
                DWORD read_bytes = 0;
                while (ReadFile(read, buffer.data(), 4095, &read_bytes, nullptr) && read_bytes > 0) {
                    buffer[read_bytes] = '\0';
                    output += buffer.data();
                }
            }

            CloseHandle(info.hProcess);
            CloseHandle(info.hThread);
            CloseHandle(read);

            return output;
        }
    }

    std::vector<Diagnostic> DiagnosticEngine::Compile(const DiagnosticTask& task) {
        auto extension_name = std::filesystem::path(task.filename).extension().string();
        auto compile_path   = (std::filesystem::temp_directory_path() / std::filesystem::path(task.filename).filename()).generic_string();

        std::ofstream(compile_path, std::ios::binary) << task.source;
        std::string target_path = std::format("{}.spv", compile_path);

        auto command = std::format("\"{}\" -o {} ", glslc_path_, target_path);
        if (task.shader_stage.has_value()) {
            command += std::format("-fshader-stage={} -D_GLSLD ", *task.shader_stage);
        }

        command += std::format("-I \"{}\" ", std::filesystem::path(task.filename).parent_path().generic_string());
        for (const auto& dir : task.include_dirs) {
            command += std::format("-I \"{}\" ", dir.generic_string());
        }

        command += std::format("\"{}\" ", compile_path);

        if (task.target_env.has_value())
            command += std::format("--target-env={} ", *task.target_env);
        if (task.target_spv.has_value())
            command += std::format("--target-spv={} ", *task.target_spv);

        auto output = ExecuteCommand(command);
        std::filesystem::remove(compile_path);
        std::filesystem::remove(target_path);

        return ParseErrorOutput(output, task.filename, task.source);
    }
}
