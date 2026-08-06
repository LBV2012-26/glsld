#pragma once

#include <condition_variable>
#include <filesystem>
#include <memory>
#include <shared_mutex>
#include <optional>
#include <queue>
#include <stop_token>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "Analyzer/Syntax/Document.hpp"
#include "Base/FileSystem/Source.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    struct DiagnosticTask {
        std::string                uri;
        std::string                source;
        std::string                filename; // 给编译器报错用
        IncludeDirectoryHandle     include_dirs;
        std::optional<std::string> shader_stage;
        std::optional<std::string> target_env;
        std::optional<std::string> target_spv;
        int                        version_replica;
        VersionPointer             version_pointer;
    };

    enum class DiagnosticSeverity {
        kError   = 1,
        kWarning = 2
    };

    struct Diagnostic {
        int                line;          // 0-based
        int                character;     // 0-based
        int                end_line;
        int                end_character;
        DiagnosticSeverity severity;
        std::string        message;
    };

    class DiagnosticEngine {
    public:
        using Callback = std::function<void(std::string_view uri, int version, std::vector<Diagnostic>)>;

        DiagnosticEngine();
        ~DiagnosticEngine();

        void SetCallback(Callback callback);
        void Submit(DiagnosticTask task);
        void Stop();

        void set_glslc_path(const std::filesystem::path& filename);

    private:
        void Run();
        std::vector<Diagnostic> Compile(const DiagnosticTask& task);

        Callback                    callback_;
        std::string                 glslc_path_;
        std::shared_mutex           mutex_;
        std::condition_variable_any condition_;
        std::queue<DiagnosticTask>  queue_;
        std::stop_source            stop_source_;
        std::jthread                thread_;
    };
}
