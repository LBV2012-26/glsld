#include "pch.hpp"
#include "Source.hpp"

#include <algorithm>
#include <format>
#include <mutex>
#include <ranges>
#include <span>
#include <utility>

#include "Base/Unicode.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
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
        const auto normalized = Utils::NormalizePath(std::filesystem::path(filename));
        const auto uri        = Utils::PathToUri(normalized);
        return Intern(normalized.generic_string(), uri);
    }

    const SourceFile* SourceTable::InternByUri(std::string_view uri) {
        const auto filename = Utils::UriToPath(uri).generic_string();
        return Intern(filename, uri);
    }

    const SourceFile* SourceTable::GetByFilename(std::string_view filename) const {
        const auto normalized = Utils::NormalizePath(std::filesystem::path(filename)).generic_string();

        std::shared_lock lock(shared_mutex_);
        auto it = sources_.find(normalized);
        if (it != sources_.end()) {
            return it->second.get();
        }

        return nullptr;
    }

    const SourceFile* SourceTable::GetByUri(std::string_view uri) const {
        const auto filename = Utils::UriToPath(uri).generic_string();

        std::shared_lock lock(shared_mutex_);
        auto it = sources_.find(filename);
        if (it != sources_.end()) {
            return it->second.get();
        }

        return nullptr;
    }

    void SourceTable::RemoveByFilename(std::string_view filename) {
        const auto normalized = Utils::NormalizePath(std::filesystem::path(filename)).generic_string();
        std::lock_guard lock(shared_mutex_);
        sources_.erase(normalized);
    }

    void SourceTable::RemoveByUri(std::string_view uri) {
        const auto filename = Utils::UriToPath(uri).generic_string();
        std::lock_guard lock(shared_mutex_);
        sources_.erase(filename);
    }

    SourceLocation::SourceLocation(const SourceFile* source, std::uint32_t line, std::uint32_t column)
        : line_{ line }
        , column_{ column }
        , source_{ source }
    {
        HashCombine(cached_hash_, line_);
        HashCombine(cached_hash_, column_);
        HashCombine(cached_hash_, source_->cached_hash_);
    }
}
