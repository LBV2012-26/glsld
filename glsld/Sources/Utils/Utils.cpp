#include "pch.hpp"
#include "Utils.hpp"

#include <cctype>
#include <cmath>
#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <format>
#include <fstream>
#include <limits>
#include <system_error>
#include <variant>

#ifdef _WIN64
#include <Windows.h>
#else
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "Analyzer/Ast/Ast.hpp"
#include "Base/Unicode.hpp"

namespace glsld::Utils {
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
                    const auto high = HexToInt(input[i + 1]);
                    const auto low  = HexToInt(input[i + 2]);
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
#ifdef _WIN64
        std::array<wchar_t, MAX_PATH> buffer{};
        GetModuleFileName(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));

        const auto work_directory = std::filesystem::path(buffer.data()).parent_path().parent_path();
#else
        const auto work_directory = std::filesystem::canonical("/proc/self/exe").parent_path().parent_path();
#endif
        const auto file_path = work_directory / std::filesystem::path(filename);
        return file_path.generic_string();
    }

    std::filesystem::path UriToPath(std::string_view uri) {
        constexpr std::string_view kPrefix = "file://";

        if (!uri.starts_with(kPrefix)) {
            return NormalizePath(std::filesystem::path(uri));
        }

        const auto raw = uri.substr(kPrefix.size());
        auto decoded = PercentDecode(raw);

        if (decoded.size() >= 3 && decoded[0] == '/' &&
            std::isalpha(static_cast<unsigned char>(decoded[1])) && decoded[2] == ':')
        {
            decoded.erase(decoded.begin()); // /C:/Path -> C:/Path
        }

        while (decoded.size() > 1 && decoded[0] == '/' && decoded[1] == '/') {
            decoded.erase(decoded.begin());
        }

        return NormalizePath(std::filesystem::path(decoded));
    }

    std::string PathToUri(const std::filesystem::path& path) {
        const auto normalized = NormalizePath(path).generic_string();

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

        if (encoded.starts_with('/')) {
            return "file://" + encoded;  // "file://" + "/home/..." -> "file:///home/..."
        }
        return "file:///" + encoded;     // "file:///" + "C:/..."   -> "file:///C:/..."
    }

    std::filesystem::path NormalizePath(const std::filesystem::path& path) {
        std::error_code ec;
        const auto normalized = std::filesystem::weakly_canonical(path, ec);
        if (ec) {
            return path.lexically_normal();
        }

        return normalized.lexically_normal();
    }

    std::expected<std::vector<std::byte>, std::string> LoadBinary(const std::filesystem::path& filename) {
        std::ifstream stream(filename, std::ios::binary);
        if (!stream.is_open()) {
            return std::unexpected(std::format("Failed to open {}: no such file or directory.", filename.generic_string()));
        }

        std::error_code ec;
        const auto size = std::filesystem::file_size(filename, ec);
        if (ec) {
            return std::unexpected(std::format("Failed to get {} size", filename.generic_string()));
        }

        std::vector<char> pubsetbuf(64 * 1024);
        stream.rdbuf()->pubsetbuf(pubsetbuf.data(), pubsetbuf.size());

        std::vector<std::byte> binary(size);
        stream.read(reinterpret_cast<char*>(binary.data()), size);

        if (!stream) {
            return std::unexpected(std::format("Failed to read {}", filename.generic_string()));
        }

        if (const auto gcount = static_cast<std::size_t>(stream.gcount()); gcount != size) {
            binary.resize(gcount);
        }

        return binary;
    }

    std::expected<std::string, std::string> LoadSource(const std::filesystem::path& filename) {
        auto binary = LoadBinary(filename);
        if (!binary.has_value()) {
            return binary.error();
        }

        std::string_view source(reinterpret_cast<const char*>(binary->data()), binary->size());
        return Unicode::SanitizeUtf8(source);
    }

    std::string_view UnmangleFunctionName(std::string_view mangled_name) {
        auto raw_name = mangled_name;
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
            return std::string(argument->token.text);

        case QualifierArgumentKind::kAssignment: {
            const auto lhs = argument->children.size() > 0 ? SerializeQualifierArguments(argument->children[0]) : "";
            const auto rhs = argument->children.size() > 1 ? SerializeQualifierArguments(argument->children[1]) : "";
            return std::format("{} = {}", lhs, rhs);
        }

        case QualifierArgumentKind::kArray: {
            std::string result = "[";
            for (auto i = 0uz; i != argument->children.size(); ++i) {
                result += SerializeQualifierArguments(argument->children[i]);
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
                result += SerializeQualifierArguments(argument->children[i]);
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
                const auto* current = argument->children[i];
                result += SerializeQualifierArguments(current);
                if (i + 1 < argument->children.size()) {
                    const auto& next = *argument->children[i + 1];
                    if (current->token.type != TokenType::kDot &&
                        next.token.type != TokenType::kDot)
                    {
                        result += " ";
                    }
                }
            }

            return result;
        }
        }

        return {};
    }

    std::string UnquoteStringLiteral(std::string_view text) {
        if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
            return std::string(text.substr(1, text.size() - 2));
        }
        return std::string(text);
    }

    NumberLiteralInfo::operator bool() const {
        return kind != NumberLiteralKind::kInvalid;
    }

    NumberLiteralInfo AnalyzeNumberLiteral(std::string_view text) {
        if (text.empty()) {
            return {};
        }

        auto EndsWithInsensitive = [text](std::string_view suffix) -> bool {
            if (text.size() < suffix.size()) {
                return false;
            }

            const auto actual = text.substr(text.size() - suffix.size());
            for (auto i = 0uz; i != suffix.size(); ++i) {
                const auto lhs = static_cast<unsigned char>(actual[i]);
                const auto rhs = static_cast<unsigned char>(suffix[i]);
                if (std::tolower(lhs) != std::tolower(rhs)) {
                    return false;
                }
            }

            return true;
        };

        const bool is_hex = text.size() >= 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X');
        const bool has_float_syntax =
            text.find('.') != std::string_view::npos ||
            text.find_first_of(is_hex ? "pP" : "eE") != std::string_view::npos;

        if (EndsWithInsensitive("lf") && (has_float_syntax || !is_hex)) {
            return {
                .kind = NumberLiteralKind::kFloatingPoint,
                .core = text.substr(0, text.size() - 2),
                .base = is_hex ? 16 : 10,
                .bits = 64
            };
        }

        if (EndsWithInsensitive("hf") && (has_float_syntax || !is_hex)) {
            return {
                .kind = NumberLiteralKind::kFloatingPoint,
                .core = text.substr(0, text.size() - 2),
                .base = is_hex ? 16 : 10,
                .bits = 16
            };
        }

        if (EndsWithInsensitive("f") && (has_float_syntax || !is_hex)) {
            return {
                .kind = NumberLiteralKind::kFloatingPoint,
                .core = text.substr(0, text.size() - 1),
                .base = is_hex ? 16 : 10,
                .bits = 32
            };
        }

        if (has_float_syntax) {
            return {
                .kind = NumberLiteralKind::kFloatingPoint,
                .core = text,
                .base = is_hex ? 16 : 10,
                .bits = 32
            };
        }

        struct IntegerSuffix {
            std::string_view  text;
            NumberLiteralKind kind;
            int               bits;
        };

        static constexpr std::array<IntegerSuffix, 5> kIntegerSuffixes = {
            IntegerSuffix{ "ul", NumberLiteralKind::kUnsignedInteger, 64 },
            IntegerSuffix{ "us", NumberLiteralKind::kUnsignedInteger, 16 },
            IntegerSuffix{ "u",  NumberLiteralKind::kUnsignedInteger, 32 },
            IntegerSuffix{ "l",  NumberLiteralKind::kSignedInteger,   64 },
            IntegerSuffix{ "s",  NumberLiteralKind::kSignedInteger,   16 }
        };

        auto core = text;
        auto kind = NumberLiteralKind::kSignedInteger;
        int  bits = 32;

        for (const auto& suffix : kIntegerSuffixes) {
            if (EndsWithInsensitive(suffix.text)) {
                core = text.substr(0, text.size() - suffix.text.size());
                kind = suffix.kind;
                bits = suffix.bits;
                break;
            }
        }

        if (core.empty()) {
            return {};
        }

        int base = 10;

        if (core.size() >= 2 && core[0] == '0' && (core[1] == 'x' || core[1] == 'X')) {
            base = 16;
            core.remove_prefix(2);
        } else if (core.size() > 1 && core[0] == '0') {
            base = 8;
        }

        if (core.empty()) {
            return {};
        }

        return {
            .kind = kind,
            .core = core,
            .base = base,
            .bits = bits
        };
    }

    std::int64_t ParseNumberLiteralToInteger(std::string_view text) {
        const auto literal = AnalyzeNumberLiteral(text);
        if (!literal) {
            return 0;
        }

        if (literal.kind == NumberLiteralKind::kFloatingPoint) {
            auto core   = literal.core;
            auto format = std::chars_format::general;

            if (literal.base == 16) {
                if (core.size() >= 2 && core[0] == '0' && (core[1] == 'x' || core[1] == 'X')) {
                    core.remove_prefix(2);
                }
                format = std::chars_format::hex;
            }

            if (core.empty()) {
                return 0;
            }

            double      value = 0.0;
            const auto* begin = core.data();
            const auto* end   = begin + core.size();

            const auto [ptr, ec] = std::from_chars(begin, end, value, format);
            if (ec != std::errc{} || ptr != end || !std::isfinite(value)) {
                return 0;
            }

            constexpr auto kInt64Min          = static_cast<double>(std::numeric_limits<std::int64_t>::min());
            constexpr auto kInt64MaxExclusive = static_cast<double>(std::numeric_limits<std::int64_t>::max());

            if (value <= kInt64Min)
                return std::numeric_limits<std::int64_t>::min();
            if (value >= kInt64MaxExclusive)
                return std::numeric_limits<std::int64_t>::max();
            return static_cast<std::int64_t>(value);
        }

        auto core     = literal.core;
        bool negative = false;
        if (!core.empty() && (core.front() == '+' || core.front() == '-')) {
            negative = core.front() == '-';
            core.remove_prefix(1);
        }

        if (core.empty()) {
            return 0;
        }

        auto magnitude = 0uz;
        const auto* begin = core.data();
        const auto* end   = begin + core.size();

        const auto [ptr, ec] = std::from_chars(begin, end, magnitude, literal.base);
        if (ec != std::errc{} || ptr != end) {
            return 0;
        }

        constexpr auto kInt64Max = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
        constexpr auto kInt64MinMagnitude = kInt64Max + 1;

        if (!negative) {
            if (magnitude > kInt64Max) {
                return std::numeric_limits<std::int64_t>::max();
            }
            return static_cast<std::int64_t>(magnitude);
        }

        if (magnitude >= kInt64MinMagnitude) {
            return std::numeric_limits<std::int64_t>::min();
        }

        return -static_cast<std::int64_t>(magnitude);
    }

#ifndef _WIN64
    namespace {
        std::vector<std::string> ParseCommand(std::string_view command) {
            std::vector<std::string> args;
            std::string current;
            char quote  = 0;
            bool escape = false;

            for (char ch : command) {
                if (escape) {
                    current += ch;
                    escape = false;
                    continue;
                }

                if (ch == '\\' && quote != '\'') {
                    escape = true;
                    continue;
                }

                if (quote) {
                    if (ch == quote) {
                        quote = 0;
                    } else {
                        current += ch;
                    }
                    continue;
                }

                if (ch == '\'' || ch == '"') {
                    quote = ch;
                } else if (std::isspace(static_cast<unsigned char>(ch))) {
                    if (!current.empty()) {
                        args.emplace_back(std::move(current));
                        current.clear();
                    }
                } else {
                    current += ch;
                }
            }

            if (escape)
                current += '\\';
            if (!current.empty())
                args.emplace_back(std::move(current));

            return args;
        };
    }
#endif

    std::string ExecuteCommand(
        std::string_view command,
        std::string_view working_dir,
        std::string_view input,
        int timeout_ms,
        int* exit_code)
    {
#ifdef _WIN64
        SECURITY_ATTRIBUTES attributes{
            .nLength              = sizeof(attributes),
            .lpSecurityDescriptor = nullptr,
            .bInheritHandle       = TRUE
        };

        HANDLE stdout_read  = nullptr;
        HANDLE stdout_write = nullptr;
        if (!CreatePipe(&stdout_read, &stdout_write, &attributes, 0)) {
            return {};
        }
        SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0);

        HANDLE stdin_read  = nullptr;
        HANDLE stdin_write = nullptr;
        if (!input.empty()) {
            if (!CreatePipe(&stdin_read, &stdin_write, &attributes, 0)) {
                CloseHandle(stdout_read);
                CloseHandle(stdout_write);
                return {};
            }
            SetHandleInformation(stdin_write, HANDLE_FLAG_INHERIT, 0);
        }

        if (command.empty() || command.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            if (stdin_read != nullptr)
                CloseHandle(stdin_read);
            if (stdin_write != nullptr)
                CloseHandle(stdin_write);
            if (stdout_read != nullptr)
                CloseHandle(stdout_read);
            if (stdout_write != nullptr)
                CloseHandle(stdout_write);
            return {};
        }

        const auto command_size = MultiByteToWideChar(CP_UTF8, 0, command.data(), -1, nullptr, 0);
        std::wstring wcommand;
        wcommand.resize_and_overwrite(command_size, [&](auto* buffer, std::size_t buffer_size) -> std::size_t {
            return MultiByteToWideChar(CP_UTF8, 0, command.data(), -1, buffer, static_cast<int>(buffer_size));
        });

        std::wstring wworking_dir;
        if (!working_dir.empty()) {
            const auto dir_size = MultiByteToWideChar(CP_UTF8, 0, working_dir.data(), -1, nullptr, 0);
            wworking_dir.resize_and_overwrite(dir_size, [&](auto* buffer, std::size_t buffer_size) -> std::size_t {
                return MultiByteToWideChar(CP_UTF8, 0, working_dir.data(), -1, buffer, static_cast<int>(buffer_size));
            });
        }

        STARTUPINFO startup{
            .cb         = sizeof(startup),
            .dwFlags    = STARTF_USESTDHANDLES,
            .hStdInput  = stdin_read,
            .hStdOutput = stdout_write,
            .hStdError  = stdout_write
        };

        PROCESS_INFORMATION info{};
        auto ok = CreateProcess(
            nullptr,
            wcommand.data(),
            nullptr,
            nullptr,
            TRUE,
            CREATE_NO_WINDOW,
            nullptr,
            working_dir.empty() ? nullptr : wworking_dir.data(),
            &startup,
            &info
        );

        if (stdin_read != nullptr)
            CloseHandle(stdin_read);
        if (stdout_write != nullptr)
            CloseHandle(stdout_write);

        if (!ok) {
            if (stdin_write != nullptr)
                CloseHandle(stdin_write);
            if (stdout_read != nullptr)
                CloseHandle(stdout_read);
            return {};
        }

        if (!input.empty() && stdin_write != nullptr) {
            DWORD written_bytes = 0;
            WriteFile(stdin_write, input.data(), static_cast<DWORD>(input.size()), &written_bytes, nullptr);
            CloseHandle(stdin_write);
        }

        std::string output;
        std::array<char, 4096> buffer{};

        auto DrainPipe = [&]() -> void {
            while (true) {
                DWORD available = 0;
                if (!PeekNamedPipe(stdout_read, nullptr, 0, nullptr, &available, nullptr) || available == 0) {
                    return;
                }

                DWORD read_bytes = 0;
                const auto read_size = std::min<DWORD>(available, static_cast<DWORD>(buffer.size()));
                if (!ReadFile(stdout_read, buffer.data(), read_size, &read_bytes, nullptr) || read_bytes == 0) {
                    return;
                }

                output.append(buffer.data(), read_bytes);
            }
        };

        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(std::max(timeout_ms, 0));

        while (true) {
            DrainPipe();

            const auto wait = WaitForSingleObject(info.hProcess, 10);
            if (wait == WAIT_OBJECT_0) {
                DrainPipe();
                break;
            }

            const bool timed_out = timeout_ms >= 0 && std::chrono::steady_clock::now() >= deadline;
            if (wait == WAIT_FAILED || timed_out) {
                TerminateProcess(info.hProcess, timed_out ? WAIT_TIMEOUT : ERROR_PROCESS_ABORTED);
                WaitForSingleObject(info.hProcess, INFINITE);
                DrainPipe();
                break;
            }
        }

        DWORD process_exit_code = 0;
        if (GetExitCodeProcess(info.hProcess, &process_exit_code) && exit_code != nullptr) {
            *exit_code = static_cast<int>(process_exit_code);
        }

        CloseHandle(info.hProcess);
        CloseHandle(info.hThread);
        CloseHandle(stdout_read);

        return output;
#else
        if (command.empty()) {
            return {};
        }

        auto args = ParseCommand(command);
        if (args.empty()) {
            return {};
        }

        std::vector<char*> argv;
        argv.reserve(args.size() + 1);
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.data()));
        }

        std::array<int, 2> stdout_pipe{};
        std::array<int, 2> stdin_pipe{};

        if (pipe(stdout_pipe.data()) != 0) {
            return {};
        }

        bool use_stdin = !input.empty();

        if (use_stdin && pipe(stdin_pipe.data()) != 0) {
            close(stdout_pipe[0]);
            close(stdout_pipe[1]);
            return {};
        }

        auto pid = fork();

        if (pid < 0) {
            close(stdout_pipe[0]);
            close(stdout_pipe[1]);
            if (use_stdin) {
                close(stdin_pipe[0]);
                close(stdin_pipe[1]);
            }

            return {};
        }

        if (pid == 0) {
            // child
            close(stdout_pipe[0]);
            // stdout/stderr
            if (dup2(stdout_pipe[1], STDOUT_FILENO) < 0 ||
                dup2(stdout_pipe[1], STDERR_FILENO) < 0)
            {
                _exit(126);
            }

            close(stdout_pipe[1]);

            if (use_stdin) {
                close(stdin_pipe[1]);
                if (dup2(stdin_pipe[0], STDIN_FILENO) < 0) {
                    _exit(126);
                }
                close(stdin_pipe[0]);
            } else {
                int devnull = open("/dev/null", O_RDONLY);
                if (devnull >= 0) {
                    dup2(devnull, STDIN_FILENO);
                    close(devnull);
                }
            }

            if (!working_dir.empty()) {
                if (chdir(std::string(working_dir).c_str()) != 0) {
                    _exit(126);
                }
            }

            execvp(argv[0], argv.data());
            _exit(errno == ENOENT ? 127 : 126);
        }

        // parent

        close(stdout_pipe[1]);

        if (use_stdin) {
            close(stdin_pipe[0]);

            ssize_t total = 0;
            while (total < static_cast<ssize_t>(input.size())) {
                ssize_t n = write(stdin_pipe[1], input.data() + total, input.size() - total);
                if (n <= 0) {
                    break;
                }
                total += n;
            }

            close(stdin_pipe[1]);
        }

        std::string output;
        std::array<char, 4096> buffer{};
        auto start = std::chrono::steady_clock::now();

        while (true) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(stdout_pipe[0], &readfds);

            timeval tv{};
            tv.tv_sec  = 0;
            tv.tv_usec = 10000; // 10ms

            int ret = select(stdout_pipe[0] + 1, &readfds, nullptr, nullptr, &tv);
            if (ret > 0 && FD_ISSET(stdout_pipe[0], &readfds)) {
                ssize_t n = read(stdout_pipe[0], buffer.data(), buffer.size());
                if (n > 0) {
                    output.append(buffer.data(), n);
                }
            }

            int  status = 0;
            auto result = waitpid(pid, &status, WNOHANG);
            if (result == pid) {
                // drain remaining pipe
                while (true) {
                    ssize_t n = read(stdout_pipe[0], buffer.data(), buffer.size());
                    if (n <= 0) {
                        break;
                    }
                    output.append(buffer.data(), n);
                }

                if (exit_code != nullptr) {
                    if (WIFEXITED(status)) {
                        *exit_code = WEXITSTATUS(status);
                    } else if (WIFSIGNALED(status)) {
                        *exit_code = 128 + WTERMSIG(status);
                    } else {
                        *exit_code = -1;
                    }
                }

                break;
            }

            bool timed_out = timeout_ms >= 0 && std::chrono::steady_clock::now() - start >= std::chrono::milliseconds(timeout_ms);
            if (timed_out) {
                kill(pid, SIGTERM);
                bool segterm_success = true;

                usleep(100000);
                if (waitpid(pid, nullptr, WNOHANG) == 0) {
                    kill(pid, SIGKILL);
                    segterm_success = false;
                }

                int status = 0;
                waitpid(pid, &status, 0);

                if (exit_code) {
                    if (WIFSIGNALED(status)) {
                        *exit_code = 128 + WTERMSIG(status);
                    } else if (WIFEXITED(status)) {
                        *exit_code = WEXITSTATUS(status);
                    }
                }

                break;
            }
        }

        close(stdout_pipe[0]);
        return output;
#endif
    }
}
