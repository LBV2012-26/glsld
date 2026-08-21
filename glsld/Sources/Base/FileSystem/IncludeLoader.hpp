#pragma once

#include <filesystem>
#include <future>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "Analyzer/Syntax/Token.hpp"
#include "Base/FileSystem/Source.hpp"
#include "Base/Hash.hpp"
#include "Base/ThreadPool.hpp"

namespace glsld {
    struct IncludeData {
        std::string                     filename;
        std::string                     uri;
        std::string                     source;
        std::string                     error;
        std::vector<Token>              tokens;
        std::filesystem::file_time_type write_time;

        bool valid() const {
            return error.empty();
        }
    };

    using IncludeSnapshot       = std::shared_ptr<const IncludeData>;
    using IncludeSnapshotFuture = std::shared_future<IncludeSnapshot>;

    class IncludeLoader {
    public:
        IncludeLoader(SourceTable& source_table, ThreadPool& thread_pool);

        IncludeSnapshotFuture Include(
            std::string_view includer_uri,
            std::string_view include_expr,
            IncludeDirectoryHandle include_dirs);

        IncludeSnapshotFuture Include(
            std::string_view includer_uri,
            std::span<const Token> body_tokens,
            IncludeDirectoryHandle include_dirs);

        void Prefetch(
            std::string_view includer_uri,
            std::string_view include_expr,
            IncludeDirectoryHandle include_dirs);

        void Invalidate(std::string_view filename);
        void Clear();

    private:
        struct IncludeTarget {
            std::string relative_path;
            bool        system_include{ false };
        };

        IncludeSnapshotFuture Include(
            std::string_view includer_uri,
            std::string_view include_expr,
            IncludeDirectoryHandle include_dirs,
            std::optional<IncludeTarget> parsed_target);

        std::optional<IncludeTarget> ParseIncludeExpr(std::string_view include_expr) const;
        std::optional<IncludeTarget> ParseIncludeFromTokens(std::span<const Token> body_tokens) const;

        std::optional<std::filesystem::path> ResolveIncludePath(
            const std::filesystem::path& includer_path,
            const IncludeTarget& target,
            IncludeDirectoryHandle include_dirs) const;

        IncludeSnapshot LoadIncludeFile(
            const std::filesystem::path& normalized_path,
            IncludeDirectoryHandle include_dirs);

        IncludeSnapshotFuture MakeReadyFuture(IncludeSnapshot snapshot) const;

        SourceTable&                               source_table_;
        ThreadPool&                                thread_pool_;
        std::shared_mutex                          mutex_;
        StringHeteroHashMap<IncludeSnapshot>       cache_;
        StringHeteroHashMap<IncludeSnapshotFuture> inflight_;
    };
}
