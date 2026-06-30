#pragma once

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "Base/Hash.hpp"

namespace glsld {
    struct DiagnosticTask {
        std::string                             uri;
        std::string                             source;
        std::string                             filename; // 给编译器报错用
        std::span<const std::filesystem::path>  include_dirs;
        std::optional<std::string>              shader_stage;
        std::optional<std::string>              target_env;
        std::optional<std::string>              target_spv;
        int                                     version_replica;
        std::shared_ptr<const std::atomic<int>> version_pointer;
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

    private:
        void Run();
        std::vector<Diagnostic> Compile(const DiagnosticTask& task);

        Callback                   callback_;
        std::string                glslc_path_;
        std::mutex                 mutex_;
        std::condition_variable    condition_;
        std::queue<DiagnosticTask> queue_;
        std::atomic<bool>          running_{ true };
    };
}
