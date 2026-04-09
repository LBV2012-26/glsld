#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

namespace glsld {
    enum class TokenType {
        kUnknown,

        // 基础分类
        kPrimitive,              // void, bool, int, float...
        kKeyword,                // if, else, for, while, switch, return, layout...
        kBuiltInFunction,        // dot, normalize, texture...
        kBuiltInVariable,        // gl_Position, gl_FragCoord...
        kBuiltInType,            // vec2, mat4, sampler2D...
        kPreprocessor,           // define, version, extension, if, else (after modified)

        // 基础标点
        kOpenBrace,              // {
        kCloseBrace,             // }
        kOpenBracket,            // [
        kCloseBracket,           // ]
        kOpenParen,              // (
        kCloseParen,             // )
        kComma,                  // ,
        kColon,                  // :
        kSemicolon,              // ;
        kDot,                    // .
        kQuestion,               // ? (三元运算符)
        kSharp,                  // #
        kBackslash,              // \

        // 算术运算符
        kPlus,                   // +
        kMinus,                  // -
        kStar,                   // *
        kSlash,                  // /
        kPercent,                // %
        kPlusPlus,               // ++
        kMinusMinus,             // --

        // 逻辑运算符
        kExclamation,            // !
        kAmpersandAmpersand,     // &&
        kCaretCaret,             // ^^ (GLSL unique)
        kVerticalBarVerticalBar, // ||

        // 比较运算符
        kEqualEqual,             // ==
        kNotEqual,               // !=
        kLessThan,               // <
        kGreaterThan,            // >
        kLessEqual,              // <=
        kGreaterEqual,           // >=

        // 位运算符
        kAmpersand,              // &
        kVerticalBar,            // |
        kCaret,                  // ^
        kTilde,                  // ~
        kLeftShift,              // <<
        kRightShift,             // >>

        // 赋值运算符
        kEqual,                  // =
        kPlusEqual,              // +=
        kMinusEqual,             // -=
        kStarEqual,              // *=
        kSlashEqual,             // /=
        kPercentEqual,           // %=
        kLeftShiftEqual,         // <<=
        kRightShiftEqual,        // >>=
        kAmpersandEqual,         // &=
        kCaretEqual,             // ^=
        kVerticalBarEqual,       // |=

        // 字面量与标识符
        kIdentifier,
        kNumberLiteral,
        kStringLiteral,
        kEndOfFile,

        // 高级
        kSharpSharp,             // ##
        kColonColon              // ::
    };

    class SourceFile {
    public:
        SourceFile(std::string_view filename, std::string_view uri);

        bool operator==(const SourceFile& other) const;

        std::string_view filename() const;
        std::string_view uri() const;

    private:
        friend class SourceLocation;

        std::string filename_;
        std::string uri_;
        std::size_t cached_hash_{};
    };

    using SourceReference = std::shared_ptr<SourceFile>;

    class SourceLocation {
    public:
        SourceLocation() = default;
        SourceLocation(SourceReference source_ref, std::size_t line, std::size_t column);

        bool operator==(const SourceLocation& other) const;
        auto operator<=>(const SourceLocation& other) const;

        SourceReference source_ref() const;
        std::string_view filename() const;
        std::string_view uri() const;
        std::size_t line() const;
        std::size_t column() const;

    private:
        friend struct LocationHash;

        SourceReference source_ref_{ nullptr };
        std::size_t     line_{};
        std::size_t     column_{};
        std::size_t     cached_hash_{};
    };

    struct LocationHash {
        std::size_t operator()(const SourceLocation& location) const;
    };

    struct Token {
        std::string    text{ "unknown" };
        SourceLocation location;
        TokenType      type{ TokenType::kUnknown };
    };
}

#include "Token.inl"
