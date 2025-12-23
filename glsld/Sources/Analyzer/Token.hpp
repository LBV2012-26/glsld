#pragma once

#include <cstddef>
#include <string>

namespace glsld {
    enum class TokenType {
        kKeyword,
        kKeyword_Control,
        kKeyword_Typed,
        kBuiltInFunction,
        kBuiltInVariable,
        kPreprocessor,

        kOpenBrace,    // {
        kCloseBrace,   // }
        kOpenBracket,  // [
        kCloseBracket, // ]
        kOpenParen,    // (
        kCloseParen,   // )
        kComma,        // ,
        kColon,        // :
        kSemicolon,    // ;
        kPlus,         // +
        kMinus,        // -
        kStar,         // *
        kEqual,        // =
        kSharp,        // #
        kSlash,        // /
        kBackslash,    // \

        kIdentifier,
        kNumberLiteral,
        kStringLiteral,
        kUnknown,
        kEndOfFile
    };

    struct SourceLocation {
        std::size_t line{};
        std::size_t column{};
    };

    struct Token {
        std::string    text;
        SourceLocation location;
        TokenType      type;
    };
}
