#include "stdafx.h"
#include "IncludeLoader.hpp"

#include <cstddef>
#include <exception>
#include <fstream>
#include <ios>
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

    IncludeLoader::SnapshotFuture IncludeLoader::Include(
        std::string_view includer_uri,
        std::string_view include_expr,
        std::span<const std::filesystem::path> include_dirs)
    {
        return Include(includer_uri, include_expr, include_dirs, std::nullopt);
    }

    IncludeLoader::SnapshotFuture IncludeLoader::Include(
        std::string_view includer_uri,
        std::span<const Token> body_tokens,
        std::span<const std::filesystem::path> include_dirs)
    {
        auto target = ParseIncludeFromTokens(body_tokens);
        if (!target.has_value()) {
            auto failed = std::make_shared<IncludeFileSnapshot>();
            failed->error = "Invalid #include syntax";
            return MakeReadyFuture(std::move(failed));
        }

        return Include(includer_uri, {}, include_dirs, std::move(target));
    }

    void IncludeLoader::Prefetch(
        std::string_view includer_uri,
        std::string_view include_expr,
        std::span<const std::filesystem::path> include_dirs)
    {
        std::ignore = Include(includer_uri, include_expr, include_dirs);
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

    IncludeLoader::SnapshotFuture IncludeLoader::Include(
        std::string_view includer_uri,
        std::string_view include_expr,
        std::span<const std::filesystem::path> include_dirs,
        std::optional<IncludeTarget> parsed_target)
    {
        auto target = parsed_target.has_value() ? std::move(parsed_target) : ParseIncludeExpr(include_expr);
        if (!target.has_value()) {
            auto failed = std::make_shared<IncludeFileSnapshot>();
            failed->error = "Invalid include expression";
            return MakeReadyFuture(std::move(failed));
        }

        auto includer_path = utils::UriToPath(includer_uri);
        auto resolved_path = ResolveIncludePath(includer_path, *target, include_dirs);
        if (!resolved_path.has_value()) {
            auto failed = std::make_shared<IncludeFileSnapshot>();
            failed->error = "Failed to resolve include path";
            return MakeReadyFuture(std::move(failed));
        }

        auto normalized = utils::NormalizePath(*resolved_path);
        auto filename   = normalized.generic_string();

        {
            std::shared_lock lock(mutex_);

            auto cache_it = cache_.find(filename);
            if (cache_it != cache_.end()) {
                std::error_code ec;
                auto latest = std::filesystem::last_write_time(normalized, ec);
                if (!ec && latest == cache_it->second->write_time) {
                    return MakeReadyFuture(cache_it->second);
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
                std::error_code ec;
                auto latest = std::filesystem::last_write_time(normalized, ec);
                if (!ec && latest == cache_it->second->write_time) {
                    return MakeReadyFuture(cache_it->second);
                }

                cache_.erase(cache_it);
            }

            auto inflight_it = inflight_.find(filename);
            if (inflight_it != inflight_.end()) {
                return inflight_it->second;
            }
        }

        auto task = [this, normalized, filename, include_dirs = std::move(include_dirs)]()
            -> Snapshot
        {
            auto loaded = LoadIncludeFile(normalized, include_dirs);
            {
                std::unique_lock lock(mutex_);
                inflight_.erase(filename);

                if (loaded != nullptr && loaded->valid()) {
                    cache_[filename] = loaded;
                }
            }

            return loaded;
        };

        SnapshotFuture future;
        if (thread_pool_.max_thread_count() == 0) {
            future = MakeReadyFuture(task());
        } else {
            future = thread_pool_.Submit(std::move(task)).share();
        }

        inflight_[filename] = future;
        return future;
    }

    std::optional<IncludeLoader::IncludeTarget>
    IncludeLoader::ParseIncludeExpr(std::string_view include_expr) const {
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

    std::optional<IncludeLoader::IncludeTarget>
    IncludeLoader::ParseIncludeFromTokens(std::span<const Token> body_tokens) const {
        if (body_tokens.empty()) {
            return std::nullopt;
        }

        if (body_tokens.front().type == TokenType::kStringLiteral) {
            return ParseIncludeExpr(body_tokens.front().text);
        }

        if (body_tokens.front().type == TokenType::kLessThan) {
            std::string inner;
            bool closed = false;

            for (auto i = 1uz; i != body_tokens.size(); ++i) {
                if (body_tokens[i].type == TokenType::kGreaterThan) {
                    closed = true;
                    break;
                }

                if (body_tokens[i].type == TokenType::kEndOfFile) {
                    break;
                }

                inner += body_tokens[i].text;
            }

            if (!closed || inner.empty()) {
                return std::nullopt;
            }

            return IncludeTarget{
                .relative_path  = std::move(inner),
                .system_include = true
            };
        }

        return std::nullopt;
    }

    std::optional<std::filesystem::path> IncludeLoader::ResolveIncludePath(
        const std::filesystem::path& includer_path,
        const IncludeTarget& target,
        std::span<const std::filesystem::path> include_dirs) const
    {
        std::vector<std::filesystem::path> candidates;
        candidates.reserve(include_dirs.size() + 1);

        if (!target.system_include) {
            candidates.push_back(includer_path.parent_path() / target.relative_path);
        }

        for (const auto& dir : include_dirs) {
            candidates.push_back(dir / target.relative_path);
        }

        for (const auto& candidate : candidates) {
            auto normalized = utils::NormalizePath(candidate);
            std::error_code ec;
            if (std::filesystem::exists(normalized, ec) && !ec) {
                return normalized;
            }
        }

        return std::nullopt;
    }

    namespace {
        std::pair<std::string, std::string> LoadSource(const std::filesystem::path& path) {
            std::ifstream stream(path, std::ios::binary);

            if (!stream.is_open()) {
                return { "", "Failed to open file" };
            }

            std::error_code ec;
            auto size = std::filesystem::file_size(path, ec);
            if (ec) {
                return { "", "Failed to get file size" };
            }

            std::vector<std::byte> pubsetbuf(1024 * 1024);
            stream.rdbuf()->pubsetbuf(reinterpret_cast<char*>(pubsetbuf.data()), pubsetbuf.size());

            std::string source;
            source.resize_and_overwrite(size, [&stream](char* data, auto size) -> std::size_t {
                stream.read(data, size);
                return stream.gcount();
            });

            if (!stream) {
                return { "", "Failed to read file" };
            }

            return { std::move(source), "" };
        }
    }

    IncludeLoader::Snapshot IncludeLoader::LoadIncludeFile(
        const std::filesystem::path& normalized_path,
        std::span<const std::filesystem::path> include_dirs)
    {
        auto snapshot = std::make_shared<IncludeFileSnapshot>();
        snapshot->filename = normalized_path.generic_string();
        snapshot->uri      = utils::PathToUri(normalized_path);

        std::error_code ec;
        snapshot->write_time = std::filesystem::last_write_time(normalized_path, ec);
        if (ec) {
            snapshot->error = "Failed to query file timestamp";
            return snapshot;
        }

        auto [source, error] = LoadSource(normalized_path);
        snapshot->source = std::move(source);
        snapshot->error  = std::move(error);
        if (!snapshot->valid()) {
            return snapshot;
        }

        try {
            const auto* source_file = source_table_.Intern(snapshot->filename, snapshot->uri);
            Lexer lexer(source_file, snapshot->source, *this, include_dirs);

            snapshot->tokens.reserve(snapshot->source.length() / 5);

            do {
                auto token = lexer.AcquireNextToken();
                snapshot->tokens.push_back(std::move(token));
            } while (snapshot->tokens.back().type != TokenType::kEndOfFile);
        } catch (const std::exception& e) {
            snapshot->error = e.what();
        }

        return snapshot;
    }

    IncludeLoader::SnapshotFuture IncludeLoader::MakeReadyFuture(Snapshot snapshot) const {
        std::promise<Snapshot> promise;
        promise.set_value(std::move(snapshot));
        return promise.get_future().share();
    }
}
