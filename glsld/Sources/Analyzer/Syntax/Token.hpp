#pragma once

#include <string>
#include "Base/FileSystem/Source.hpp"

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
        kEllipsis,               // ...
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
        kColonColon,             // ::

        // GLSL_EXT_spirv_intrinsics
        kSpirvIntrinsic
    };

    struct Token {
        std::string    text{ "unknown" };
        SourceLocation location;
        TokenType      type{ TokenType::kUnknown };
    };
}
