#include "stdafx.h"
#include "Source.hpp"

#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <mutex>
#include <utility>

#include "Utils/Utils.hpp"

namespace glsld {
    std::pair<std::string, std::string> LoadSource(const std::filesystem::path& path) {
        std::ifstream stream(path, std::ios::binary);

        if (!stream.is_open()) {
            return { "", std::format("Failed to open {}: no such file or directory.", path.generic_string()) };
        }

        std::error_code ec;
        auto size = std::filesystem::file_size(path, ec);
        if (ec) {
            return { "", std::format("Failed to get {} size", path.generic_string()) };
        }

        std::vector<std::byte> pubsetbuf(1024 * 1024);
        stream.rdbuf()->pubsetbuf(reinterpret_cast<char*>(pubsetbuf.data()), pubsetbuf.size());

        std::string source;
        source.resize_and_overwrite(size, [&stream](char* data, auto size) -> std::size_t {
            stream.read(data, size);
            return stream.gcount();
        });

        if (!stream) {
            return { "", std::format("Failed to read {}", path.generic_string()) };
        }

        return { std::move(source), "" };
    }

    SourceFile::SourceFile(std::string_view filename, std::string_view uri)
        : filename_{ filename }
        , uri_{ uri }
    {
        HashCombine(cached_hash_, filename_);
        HashCombine(cached_hash_, uri_);
    }

    const SourceFile* SourceTable::Intern(std::string_view filename, std::string_view uri) {
        auto it = sources_.find(filename);
        if (it != sources_.end()) {
            return it->second.get();
        }

        auto source = std::make_unique<SourceFile>(filename, uri);

        std::unique_lock lock(shared_mutex_);
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
        sources_.erase(normalized);
    }

    void SourceTable::RemoveByUri(std::string_view uri) {
        auto filename = utils::UriToPath(uri).generic_string();
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
