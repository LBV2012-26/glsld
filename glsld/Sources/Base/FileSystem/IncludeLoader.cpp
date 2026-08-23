#include "pch.hpp"
#include "IncludeLoader.hpp"

#include <exception>
#include <mutex>
#include <system_error>
#include <utility>

#include "Analyzer/Syntax/Lexer.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    IncludeLoader::IncludeLoader(SourceTable& source_table, ThreadPool& thread_pool)
        : source_table_{ source_table }
        , thread_pool_{ thread_pool }
    {}

    IncludeSnapshotFuture IncludeLoader::Include(
        std::string_view includer_uri,
        std::string_view include_expr,
        IncludeDirectoryHandle include_dirs)
    {
        return Include(includer_uri, include_expr, include_dirs, std::nullopt);
    }

    IncludeSnapshotFuture IncludeLoader::Include(
        std::string_view includer_uri,
        std::span<const Token> body_tokens,
        IncludeDirectoryHandle include_dirs)
    {
        auto target = ParseIncludeFromTokens(body_tokens);
        if (!target.has_value()) {
            auto failed = std::make_shared<IncludeData>();
            failed->error = "Invalid #include syntax";
            return MakeReadyFuture(std::move(failed));
        }

        return Include(includer_uri, {}, include_dirs, std::move(target));
    }

    void IncludeLoader::Prefetch(
        std::string_view includer_uri,
        std::string_view include_expr,
        IncludeDirectoryHandle include_dirs)
    {
        (void)Include(includer_uri, include_expr, include_dirs);
    }

    void IncludeLoader::Invalidate(std::string_view filename) {
        std::unique_lock lock(mutex_);
        cache_.erase(filename);
        inflight_.erase(filename);
    }

    void IncludeLoader::Clear() {
        std::unique_lock lock(mutex_);
        cache_.clear();
        inflight_.clear();
    }

    IncludeSnapshotFuture IncludeLoader::Include(
        std::string_view includer_uri,
        std::string_view include_expr,
        IncludeDirectoryHandle include_dirs,
        std::optional<IncludeTarget> parsed_target)
    {
        const auto target = parsed_target.has_value() ? std::move(parsed_target) : ParseIncludeExpr(include_expr);
        if (!target.has_value()) {
            auto failed = std::make_shared<IncludeData>();
            failed->error = "Invalid include expression";
            return MakeReadyFuture(std::move(failed));
        }

        const auto includer_path = Utils::UriToPath(includer_uri);
        const auto resolved_path = ResolveIncludePath(includer_path, *target, include_dirs);
        if (!resolved_path.has_value()) {
            auto failed = std::make_shared<IncludeData>();
            failed->error = "Failed to resolve include path";
            return MakeReadyFuture(std::move(failed));
        }

        const auto normalized = Utils::NormalizePath(*resolved_path);
        const auto filename   = normalized.generic_string();

        std::error_code ec;
        const auto latest = std::filesystem::last_write_time(normalized, ec);
        if (ec) {
            auto failed = std::make_shared<IncludeData>();
            failed->error = "Failed to query file timestamp";
            return MakeReadyFuture(std::move(failed));
        }

        {
            std::shared_lock lock(mutex_);

            auto cache_it = cache_.find(filename);
            if (cache_it != cache_.end()) {
                if (auto snapshot = cache_it->second.lock();
                    snapshot != nullptr && latest == snapshot->write_time)
                {
                    return MakeReadyFuture(std::move(snapshot));
                }
            }

            auto inflight_it = inflight_.find(filename);
            if (inflight_it != inflight_.end()) {
                return inflight_it->second;
            }
        }

        {
            std::unique_lock lock(mutex_);

            auto cache_it = cache_.find(filename);
            if (cache_it != cache_.end()) {
                if (auto snapshot = cache_it->second.lock();
                    snapshot != nullptr && latest == snapshot->write_time)
                {
                    return MakeReadyFuture(std::move(snapshot));
                }

                cache_.erase(cache_it);
            }

            auto inflight_it = inflight_.find(filename);
            if (inflight_it != inflight_.end()) {
                return inflight_it->second;
            }
        }

        auto Task = [this, normalized, filename, include_dirs = std::move(include_dirs)]()
            -> IncludeSnapshot
        {
            auto loaded = LoadIncludeFile(normalized, include_dirs);
            {
                std::lock_guard lock(mutex_);
                inflight_.erase(filename);

                if (loaded != nullptr && loaded->valid()) {
                    cache_[filename] = loaded;
                }
            }

            return loaded;
        };

        auto task   = std::make_shared<std::packaged_task<IncludeSnapshot()>>(std::move(Task));
        auto future = task->get_future().share();

        {
            std::lock_guard lock(mutex_);
            inflight_[filename] = future;
        }

        if (thread_pool_.max_thread_count() == 0) {
            (*task)();
        } else {
            thread_pool_.Submit([task]() -> void {
                (*task)();
            });
        }

        return future;
    }

    std::optional<IncludeLoader::IncludeTarget> IncludeLoader::ParseIncludeExpr(
        std::string_view include_expr) const
    {
        if (include_expr.length() < 3) {
            return std::nullopt;
        }

        if (include_expr.front() == '"' && include_expr.back() == '"') {
            return IncludeTarget{
                .relative_path  = std::string(include_expr.substr(1, include_expr.length() - 2)),
                .system_include = false
            };
        } else if (include_expr.front() == '<' && include_expr.back() == '>') {
            return IncludeTarget{
                .relative_path  = std::string(include_expr.substr(1, include_expr.length() - 2)),
                .system_include = true
            };
        } else {
            return std::nullopt;
        }
    }

    std::optional<IncludeLoader::IncludeTarget> IncludeLoader::ParseIncludeFromTokens(
        std::span<const Token> body_tokens) const
    {
        if (body_tokens.empty()) {
            return std::nullopt;
        }

        if (body_tokens.front().type == TokenType::kStringLiteral) {
            return ParseIncludeExpr(body_tokens.front().text);
        }

        return std::nullopt;
    }

    std::optional<std::filesystem::path> IncludeLoader::ResolveIncludePath(
        const std::filesystem::path& includer_path,
        const IncludeTarget& target,
        IncludeDirectoryHandle include_dirs) const
    {
        std::vector<std::filesystem::path> candidates;
        candidates.reserve(include_dirs->size() + 1);

        if (!target.system_include) { // system include only find in -I paths
            candidates.push_back(includer_path.parent_path() / target.relative_path);
        }

        for (const auto& dir : *include_dirs) {
            candidates.push_back(dir / target.relative_path);
        }

        for (const auto& candidate : candidates) {
            auto normalized = Utils::NormalizePath(candidate);
            std::error_code ec;
            if (std::filesystem::exists(normalized, ec) && !ec) {
                return normalized;
            }
        }

        return std::nullopt;
    }

    IncludeSnapshot IncludeLoader::LoadIncludeFile(
        const std::filesystem::path& normalized_path,
        IncludeDirectoryHandle include_dirs)
    {
        auto snapshot      = std::make_shared<IncludeData>();
        snapshot->filename = normalized_path.generic_string();
        snapshot->uri      = Utils::PathToUri(normalized_path);

        std::error_code ec;
        snapshot->write_time = std::filesystem::last_write_time(normalized_path, ec);
        if (ec) {
            snapshot->error = "Failed to query file timestamp";
            return snapshot;
        }

        auto source = Utils::LoadSource(normalized_path);
        if (!source.has_value()) {
            snapshot->error = std::move(source.error());
            return snapshot;
        }

        snapshot->source = std::move(*source);
        if (!snapshot->valid()) {
            return snapshot;
        }

        try {
            const auto* source_file = source_table_.Intern(snapshot->filename, snapshot->uri);
            Lexer lexer(source_file, snapshot->source, *this, include_dirs);
            snapshot->tokens = lexer.Tokenize();
        } catch (const std::exception& e) {
            snapshot->error = e.what();
        }

        return snapshot;
    }

    IncludeSnapshotFuture IncludeLoader::MakeReadyFuture(IncludeSnapshot snapshot) const {
        std::promise<IncludeSnapshot> promise;
        promise.set_value(std::move(snapshot));
        return promise.get_future().share();
    }
}
