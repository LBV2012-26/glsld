#include "pch.hpp"
#include "Source.hpp"

#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <mutex>
#include <ranges>
#include <system_error>
#include <utility>

#include "Utils/Utils.hpp"

namespace glsld {
    std::expected<std::vector<std::byte>, std::string> LoadBinary(const std::filesystem::path& filename) {
        std::ifstream stream(filename, std::ios::binary);

        if (!stream.is_open()) {
            return std::unexpected(std::format("Failed to open {}: no such file or directory.", filename.generic_string()));
        }

        std::error_code ec;
        auto size = std::filesystem::file_size(filename, ec);
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

        if (auto gcount = static_cast<std::size_t>(stream.gcount()); gcount != size) {
            binary.resize(gcount);
        }

        return binary;
    }

    std::expected<std::string, std::string> LoadSource(const std::filesystem::path& filename) {
        auto binary = LoadBinary(filename);
        if (!binary.has_value()) {
            return std::unexpected(binary.error());
        }

        auto size  = binary->size();
        auto bytes = (*binary) | std::views::take(size) | std::views::transform([](std::byte byte) -> char {
            return static_cast<char>(byte);
        });

        std::string source;
        source.resize_and_overwrite(size, [&](char* buffer, std::size_t size) -> std::size_t {
            std::ranges::copy(bytes, buffer);
            return size;
        });

        return source;
    }

    SourceFile::SourceFile(std::string_view filename, std::string_view uri, SourceKind kind)
        : filename_{ filename }
        , uri_{ uri }
        , kind_{ kind }
    {
        HashCombine(cached_hash_, filename_);
        HashCombine(cached_hash_, uri_);
    }

    SourceTable::SourceTable(SourceKind default_kind)
        : default_kind_{ default_kind }
    {}

    const SourceFile* SourceTable::Intern(std::string_view filename, std::string_view uri) {
        std::lock_guard lock(shared_mutex_);

        auto it = sources_.find(filename);
        if (it != sources_.end()) {
            return it->second.get();
        }

        auto source = std::make_unique<SourceFile>(filename, uri, default_kind_);

        auto [inserted_it, _] = sources_.try_emplace(std::string(filename), std::move(source));
        return inserted_it->second.get();
    }

    const SourceFile* SourceTable::InternByFilename(std::string_view filename) {
        auto normalized = utils::NormalizePath(std::filesystem::path(filename));
        auto uri        = utils::PathToUri(normalized);

        return Intern(normalized.generic_string(), uri);
    }

    const SourceFile* SourceTable::InternByUri(std::string_view uri) {
        auto filename = utils::UriToPath(uri).generic_string();
        return Intern(filename, uri);
    }

    const SourceFile* SourceTable::GetByFilename(std::string_view filename) const {
        auto normalized = utils::NormalizePath(std::filesystem::path(filename)).generic_string();

        std::shared_lock lock(shared_mutex_);
        auto it = sources_.find(normalized);
        if (it != sources_.end()) {
            return it->second.get();
        }

        return nullptr;
    }

    const SourceFile* SourceTable::GetByUri(std::string_view uri) const {
        auto filename = utils::UriToPath(uri).generic_string();

        std::shared_lock lock(shared_mutex_);
        auto it = sources_.find(filename);
        if (it != sources_.end()) {
            return it->second.get();
        }

        return nullptr;
    }

    void SourceTable::RemoveByFilename(std::string_view filename) {
        auto normalized = utils::NormalizePath(std::filesystem::path(filename)).generic_string();
        std::lock_guard lock(shared_mutex_);
        sources_.erase(normalized);
    }

    void SourceTable::RemoveByUri(std::string_view uri) {
        auto filename = utils::UriToPath(uri).generic_string();
        std::lock_guard lock(shared_mutex_);
        sources_.erase(filename);
    }

    SourceLocation::SourceLocation(const SourceFile* source, std::size_t line, std::size_t column)
        : line_{ line }
        , column_{ column }
        , source_{ source }
    {
        HashCombine(cached_hash_, line_);
        HashCombine(cached_hash_, column_);
        HashCombine(cached_hash_, source_->cached_hash_);
    }
}
