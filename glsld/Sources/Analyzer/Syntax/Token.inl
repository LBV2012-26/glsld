#include "Token.hpp"

#include <compare>
#include <utility>

#include "Base/Hash.hpp"

namespace glsld {
    inline SourceFile::SourceFile(std::string_view filename, std::string_view uri)
        : filename_{ filename }
        , uri_{ uri }
    {
        cached_hash_ = std::hash<std::string>{}(filename_);
        // filename 是绝对路径，uri 不用再哈希了
    }

    inline bool SourceFile::operator==(const SourceFile& other) const {
        return filename_ == other.filename_ && uri_ == other.uri_;
    }

    inline std::string_view SourceFile::filename() const {
        return filename_;
    }

    inline std::string_view SourceFile::uri() const {
        return uri_;
    }
 
    inline SourceLocation::SourceLocation(SourceReference source_ref, std::size_t line, std::size_t column)
        : source_ref_{ std::move(source_ref) }
        , line_{ line }
        , column_{ column }
    {
        if (source_ref_ == nullptr) {
            return;
        }

        HashCombine(cached_hash_, line_);
        HashCombine(cached_hash_, column_);
        HashCombine(cached_hash_, source_ref_->cached_hash_);
    }

    inline bool SourceLocation::operator==(const SourceLocation& other) const {
        return source_ref_ == other.source_ref_ && line_ == other.line_ && column_ == other.column_;
    }

    inline SourceReference SourceLocation::source_ref() const {
        return source_ref_;
    }

    inline std::string_view SourceLocation::filename() const {
        return source_ref_ != nullptr ? source_ref_->filename() : std::string_view();
    }

    inline std::string_view SourceLocation::uri() const {
        return source_ref_ != nullptr ? source_ref_->uri() : std::string_view();
    }

    inline std::size_t SourceLocation::line() const {
        return line_;
    }

    inline std::size_t SourceLocation::column() const {
        return column_;
    }

    inline auto SourceLocation::operator<=>(const SourceLocation& other) const {
        if (line_ < other.line_) {
            return std::strong_ordering::less;
        } else if (line_ > other.line_) {
            return std::strong_ordering::greater;
        } else {
            // line == other.line
            if (column_ < other.column_) {
                return std::strong_ordering::less;
            } else if (column_ > other.column_) {
                return std::strong_ordering::greater;
            } else {
                return std::strong_ordering::equal;
            }
        }
    }

    inline std::size_t LocationHash::operator()(const SourceLocation& location) const {
        return location.cached_hash_;
    }
}
