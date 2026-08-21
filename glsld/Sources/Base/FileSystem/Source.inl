#include "Source.hpp"
#include <compare>

namespace glsld {
    inline bool SourceFile::operator==(const SourceFile& other) const {
        return cached_hash_ == other.cached_hash_;
    }

    inline std::string_view SourceFile::filename() const {
        return filename_;
    }

    inline std::string_view SourceFile::uri() const {
        return uri_;
    }

    inline SourceKind SourceFile::kind() const {
        return kind_;
    }

    inline bool SourceFile::indexable() const {
        return kind_ == SourceKind::kWorkspace;
    }

    inline std::size_t SourceFileHash::operator()(const SourceFile* source) const {
        return source->cached_hash_;
    }

    inline bool SourceLocation::operator==(const SourceLocation& other) const {
        return *source_ == *other.source_ && line_ == other.line_ && column_ == other.column_;
    }

    inline auto SourceLocation::operator<=>(const SourceLocation& other) const {
        if (auto compare = source_ <=> other.source_; compare != std::strong_ordering::equal) {
            return compare;
        }

        if (auto compare = line_ <=> other.line_; compare != std::strong_ordering::equal) {
            return compare;
        }

        return column_ <=> other.column_;
    }

    inline const SourceFile* SourceLocation::source_file() const {
        return source_;
    }

    inline std::string_view SourceLocation::filename() const {
        return source_->filename();
    }

    inline std::string_view SourceLocation::uri() const {
        return source_->uri();
    }

    inline std::uint32_t SourceLocation::line() const {
        return line_;
    }

    inline std::uint32_t SourceLocation::column() const {
        return column_;
    }

    inline std::size_t LocationHash::operator()(const SourceLocation& location) const {
        return location.cached_hash_;
    }
}
