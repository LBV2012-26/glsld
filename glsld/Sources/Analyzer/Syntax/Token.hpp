#pragma once

#include <cstddef>
#include <compare>
#include <memory>
#include <string>

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

    struct SourceFile {
        std::string normalized_path;
        std::string uri;

        bool operator==(const SourceFile& other) const {
            return normalized_path == other.normalized_path && uri == other.uri;
        }
    };

    using SourceReference = std::shared_ptr<SourceFile>;

    struct SourceLocation {
        SourceReference source{ nullptr };
        std::size_t     line{};
        std::size_t     column{};

        bool operator==(const SourceLocation& other) const {
            return source == other.source && line == other.line && column == other.column;
        }

        auto operator<=>(const SourceLocation& other) const {
            if (line < other.line) {
                return std::strong_ordering::less;
            } else if (line > other.line) {
                return std::strong_ordering::greater;
            } else {
                // line == other.line
                if (column < other.column) {
                    return std::strong_ordering::less;
                } else if (column > other.column) {
                    return std::strong_ordering::greater;
                } else {
                    return std::strong_ordering::equal;
                }
            }
        }
    };

    struct Token {
        std::string    text{ "unknown" };
        SourceLocation location;
        TokenType      type{ TokenType::kUnknown };
    };
}
