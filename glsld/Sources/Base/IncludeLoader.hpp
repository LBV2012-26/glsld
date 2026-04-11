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
#include "Base/Hash.hpp"
#include "Base/Source.hpp"
#include "Base/ThreadPool.hpp"

namespace glsld {
    struct IncludeFileSnapshot {
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

    class IncludeLoader {
    public:
        using Snapshot       = std::shared_ptr<const IncludeFileSnapshot>;
        using SnapshotFuture = std::shared_future<Snapshot>;

        IncludeLoader(SourceTable& source_table, ThreadPool& thread_pool);

        SnapshotFuture Include(
            std::string_view includer_uri,
            std::string_view include_expr,
            std::span<const std::filesystem::path> include_dirs);

        SnapshotFuture Include(
            std::string_view includer_uri,
            std::span<const Token> body_tokens,
            std::span<const std::filesystem::path> include_dirs);

        void Prefetch(
            std::string_view includer_uri,
            std::string_view include_expr,
            std::span<const std::filesystem::path> include_dirs);

        void Invalidate(std::string_view filename);
        void Clear();

    private:
        struct IncludeTarget {
            std::string relative_path;
            bool        system_include{ false };
        };

        SnapshotFuture Include(
            std::string_view includer_uri,
            std::string_view include_expr,
            std::span<const std::filesystem::path> include_dirs,
            std::optional<IncludeTarget> parsed_target);

        std::optional<IncludeTarget> ParseIncludeExpr(std::string_view include_expr) const;
        std::optional<IncludeTarget> ParseIncludeFromTokens(std::span<const Token> body_tokens) const;

        std::optional<std::filesystem::path> ResolveIncludePath(
            const std::filesystem::path& includer_path,
            const IncludeTarget& target,
            std::span<const std::filesystem::path> include_dirs) const;

        Snapshot LoadIncludeFile(
            const std::filesystem::path& normalized_path,
            std::span<const std::filesystem::path> include_dirs);

        SnapshotFuture MakeReadyFuture(Snapshot snapshot) const;

        SourceTable&                          source_table_;
        ThreadPool&                           thread_pool_;
        std::shared_mutex                     mutex_;
        StringHeteroHashTable<Snapshot>       cache_;
        StringHeteroHashTable<SnapshotFuture> inflight_;
    };
}
