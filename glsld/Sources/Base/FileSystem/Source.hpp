#pragma once

#include <cstddef>
#include <expected>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>

#include "Base/Hash.hpp"

namespace glsld {
    std::expected<std::vector<std::byte>, std::string> LoadBinary(const std::filesystem::path& filename);
    std::expected<std::string, std::string> LoadSource(const std::filesystem::path& filename);

    class SourceFile {
    public:
        SourceFile(std::string_view filename, std::string_view uri);

        bool operator==(const SourceFile& other) const;

        std::string_view filename() const;
        std::string_view uri() const;

    private:
        friend struct SourceFileHash;
        friend class  SourceLocation;

        std::string filename_;
        std::string uri_;
        std::size_t cached_hash_{};
    };

    struct SourceFileHash {
        std::size_t operator()(const SourceFile* source) const;
    };

    class SourceTable {
    public:
        const SourceFile* Intern(std::string_view filename, std::string_view uri);
        const SourceFile* InternByFilename(std::string_view filename);
        const SourceFile* InternByUri(std::string_view uri);
        const SourceFile* GetByFilename(std::string_view filename) const;
        const SourceFile* GetByUri(std::string_view uri) const;
        void RemoveByFilename(std::string_view filename);
        void RemoveByUri(std::string_view uri);

    private:
        StringHeteroHashMap<std::unique_ptr<SourceFile>> sources_; // [GenericFilename, SourceFile]
        mutable std::shared_mutex                        shared_mutex_;
    };

    class SourceLocation {
    public:
        SourceLocation() = default;
        SourceLocation(const SourceFile* source, std::size_t line, std::size_t column);

        bool operator==(const SourceLocation& other) const;
        auto operator<=>(const SourceLocation& other) const;

        const SourceFile* source_file() const;
        std::string_view filename() const;
        std::string_view uri() const;
        std::size_t line() const;
        std::size_t column() const;

    private:
        friend struct LocationHash;

        const SourceFile* source_{ nullptr };
        std::size_t       line_{};
        std::size_t       column_{};
        std::size_t       cached_hash_{};
    };

    struct LocationHash {
        std::size_t operator()(const SourceLocation& location) const;
    };
}

#include "Source.inl"
