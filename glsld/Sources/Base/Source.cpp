#include "stdafx.h"
#include "Source.hpp"

#include <filesystem>
#include <utility>

#include "Utils/Utils.hpp"

namespace glsld {
    SourceFile::SourceFile(std::string_view filename, std::string_view uri)
        : filename_{ filename }
        , uri_{ uri }
    {
        HashCombine(cached_hash_, filename_);
        HashCombine(cached_hash_, uri_);
    }

    const SourceFile* SourceTable::Intern(std::string_view filename, std::string_view uri) {
        SourceFile source(filename, uri);
        auto [inserted_it, _] = sources_.try_emplace(std::string(filename), std::move(source));
        return &inserted_it->second;
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

        auto it = sources_.find(normalized);
        if (it != sources_.end()) {
            return &it->second;
        }

        return nullptr;
    }

    const SourceFile* SourceTable::GetByUri(std::string_view uri) const {
        auto filename = utils::UriToPath(uri).generic_string();

        auto it = sources_.find(filename);
        if (it != sources_.end()) {
            return &it->second;
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
