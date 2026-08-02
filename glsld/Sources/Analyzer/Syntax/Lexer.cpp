#include "pch.hpp"
#include "Lexer.hpp"

#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <ranges>
#include <string>
#include <stdexcept>
#include <vector>

#include "Analyzer/Syntax/MetadataManager.hpp"
#include "Base/Unicode.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    Lexer::Lexer(const SourceFile* source_file,
                 std::string_view source,
                 IncludeLoader& include_loader,
                 IncludeDirectoryHandle include_dirs)

        : source_file_{ source_file }
        , source_{ source }
        , include_loader_{ include_loader }
        , include_dirs_{ include_dirs }
    {
        BuildLexicalTable();
    }

    std::vector<Token> Lexer::Tokenize(int version_replica, VersionPointer version_pointer) {
        std::vector<Token> tokens;
        tokens.reserve(source_.length() / 5);

        do {
            if (version_pointer != nullptr && version_replica != version_pointer->load(std::memory_order::relaxed)) {
                return {};
            }

            tokens.push_back(AcquireNextToken());
        } while (tokens.back().type != TokenType::kEndOfFile);

        return tokens;
    }

    Token Lexer::AcquireNextToken() {
        auto token = ProduceToken();
        last_token_line_ = token.location.line();

        bool need_deep_paren = (last_token_text_ == "layout" || (last_token_text_.starts_with("spirv")) && (token.text != "spirv_id" && token.text != "spirv_by_reference"));

        if (token.type == TokenType::kSharp)
            preprocessor_line_ = true;
        if (token.type == TokenType::kOpenParen && need_deep_paren)
            ++qualifier_paren_depth_;
        if (token.type == TokenType::kCloseParen && qualifier_paren_depth_ > 0)
            --qualifier_paren_depth_;
        if (token.type == TokenType::kIdentifier  ||
            token.type == TokenType::kPrimitive   ||
            token.type == TokenType::kBuiltInType ||
            token.type == TokenType::kKeyword     ||
            token.type == TokenType::kSpirvIntrinsic)
            last_token_text_ = token.text;

        if (qualifier_paren_depth_ > 0 && token.type == TokenType::kEqual) {
            qualifier_after_equal_ = true;
        } else {
            qualifier_after_equal_ = false;
        }

        return token;
    }

    Token Lexer::ProduceToken() {
        SkipWhitespaceAndComments();

        if (line_ > last_token_line_) {
            preprocessor_line_ = false;
        }

        if (position_ < source_.length() && source_[position_] == '#') {
            TryPrefetchInclude();
        }

        const SourceLocation location(source_file_, line_, column_);

        if (position_ >= source_.length()) {
            return {
                .text     = {},
                .location = location,
                .type     = TokenType::kEndOfFile
            };
        }

        unsigned char current_char = static_cast<unsigned char>(source_[position_]);

        auto IsIdentifierAlnum = [](unsigned char ch) -> bool {
            return IsAsciiAlnum(ch) || ch == '.' || ch == '_';
        };

        if (IsAsciiDigit(current_char) || (current_char == '.' && IsAsciiDigit(Peek()))) {
            auto begin = position_;
            Advance();
            while (position_ < source_.length() && IsIdentifierAlnum(static_cast<unsigned char>(source_[position_]))) {
                Advance();
            }

            return {
                .text     = std::string(source_.substr(begin, position_ - begin)),
                .location = location,
                .type     = TokenType::kNumberLiteral
            };
        }

        if (current_char == '<') {
            auto angle_end = TryFindIncludeAngleEnd();
            if (angle_end.has_value()) {
                auto begin  = position_;
                auto length = *angle_end - begin;
                Advance(length);

                return {
                    .text     = std::string(source_.substr(begin, length)),
                    .location = location,
                    .type     = TokenType::kStringLiteral
                };
            }
        }

        auto token = DetectToken(current_char);
        if (token.type != TokenType::kUnknown) {
            return token;
        }

        auto IsIdentifierStart = [](unsigned char ch) -> bool {
            return IsAsciiAlpha(ch) || ch == '_';
        };

        auto IsIdentifierChar = [](unsigned char ch) -> bool {
            return IsAsciiAlnum(ch) || ch == '_';
        };

        if (IsIdentifierStart(current_char)) {
            std::size_t begin = position_;
            Advance();
            while (position_ < source_.length() && IsIdentifierChar(static_cast<unsigned char>(source_[position_]))) {
                Advance();
            }
            std::string_view word = source_.substr(begin, position_ - begin);

            auto it = lexical_table_->find(word);
            if (it != lexical_table_->end()) {
                if (it->second == TokenType::kPreprocessor) {
                    if (preprocessor_line_) {
                        return {
                            .text     = std::string(word),
                            .location = location,
                            .type     = it->second
                        };
                    } else {
                        return {
                            .text     = std::string(word),
                            .location = location,
                            .type     = TokenType::kIdentifier
                        };
                    }
                }

                // layout(...)/spirv_xxx(...)
                if (it->second == TokenType::kPrimitive) {
                    auto subtype = MetadataManager::GetInstance().GetLexicalSubtype(word);
                    if (subtype.has_value() && (subtype->starts_with("Primitives.Layout") || subtype->starts_with("Primitives.Spirv"))) {
                        if (qualifier_paren_depth_ == 0 || qualifier_after_equal_) {
                            return {
                                .text     = std::string(word),
                                .location = location,
                                .type     = TokenType::kIdentifier
                            };
                        }
                    }
                }

                return {
                    .text     = std::string(word),
                    .location = location,
                    .type     = it->second
                };
            }

            return {
                .text     = std::string(word),
                .location = location,
                .type     = TokenType::kIdentifier
            };
        }

        if (current_char == '"') {
            std::size_t begin = position_;
            Advance(); // consume opening quote
            while (position_ < source_.length() && source_[position_] != '"') {
                if (source_[position_] == '\\') { // handle escaped quotes
                    Advance();
                }
                Advance();
            }

            if (position_ < source_.length()) {
                Advance(); // consume closing quote
            }

            return {
                .text     = std::string(source_.substr(begin, position_ - begin)),
                .location = location,
                .type     = TokenType::kStringLiteral
            };
        }

        auto begin = position_;
        auto point = DecodeUtf8(source_.substr(position_));
        Advance(point.byte_count);

        return {
            .text     = point.valid
                      ? std::string(source_.substr(begin, point.byte_count))
                      : std::string("\xEF\xBF\xBD"),
            .location = location,
            .type     = TokenType::kUnknown
        };
    }

    void Lexer::TryPrefetchInclude() {
        if (position_ >= source_.length() || source_[position_] != '#') {
            return;
        }

        auto include_expr_range = FindIncludeExprAfterSharp(position_);
        if (!include_expr_range.has_value()) {
            return;
        }

        auto [include_begin, include_end] = *include_expr_range;
        std::string_view include_expr = source_.substr(include_begin, include_end - include_begin);
        include_loader_.Prefetch(source_file_->uri(), include_expr, include_dirs_);
    }

    std::optional<std::pair<std::size_t, std::size_t>> Lexer::FindIncludeExprAfterSharp(std::size_t sharp_index) const {
        if (sharp_index > source_.length() || source_[sharp_index] != '#') {
            return std::nullopt;
        }

        auto index = sharp_index + 1;

        auto SkipWhitespace = [&index, this]() {
            while (index < source_.length() &&
                   (source_[index] == ' ' || source_[index] == '\t' || source_[index] == '\r'))
            {
                ++index;
            }
        };

        SkipWhitespace();

        static constexpr std::string_view kInclude = "include";
        if (index + kInclude.length() > source_.length()) {
            return std::nullopt;
        }

        for (auto i = 0; i != kInclude.length(); ++i) {
            if (source_[index + i] != kInclude[i]) {
                return std::nullopt;
            }
        }

        index += kInclude.length();
        SkipWhitespace();

        if (index >= source_.length()) {
            return std::nullopt;
        }

        if (source_[index] == '"' || source_[index] == '<') {
            char delimiter = source_[index] == '"' ? '"' : '>';

            auto end = index + 1;
            while (end < source_.length() && source_[end] != delimiter && source_[end] != '\n') {
                ++end;
            }

            if (end < source_.length() && source_[end] == delimiter) {
                return std::make_pair(index, end + 1);
            }
        }

        return std::nullopt;
    }

    std::optional<std::size_t> Lexer::TryFindIncludeAngleEnd() const {
        if (position_ >= source_.length() || source_[position_] != '<') {
            return std::nullopt;
        }

        auto line_begin = position_;
        while (line_begin > 0 && source_[line_begin - 1] != '\n') {
            --line_begin;
        }

        auto first_non_space = line_begin;
        while (first_non_space < source_.length() &&
               (source_[first_non_space] == ' ' || source_[first_non_space] == '\t' || source_[first_non_space] == '\r'))
        {
            ++first_non_space;
        }

        if (first_non_space >= source_.length() || source_[first_non_space] != '#') {
            return std::nullopt;
        }

        auto include_expr = FindIncludeExprAfterSharp(first_non_space);
        if (!include_expr.has_value()) {
            return std::nullopt;
        }

        auto [include_begin, include_end] = *include_expr;
        if (include_begin == position_ && source_[include_begin] == '<') {
            return include_end;
        }

        return std::nullopt;
    }

    Token Lexer::DetectToken(unsigned char current_char) {
        switch (current_char) {
        // X
        case '{':  return Capture(TokenType::kOpenBrace);
        case '}':  return Capture(TokenType::kCloseBrace);
        case '[':  return Capture(TokenType::kOpenBracket);
        case ']':  return Capture(TokenType::kCloseBracket);
        case '(':  return Capture(TokenType::kOpenParen);
        case ')':  return Capture(TokenType::kCloseParen);
        case ',':  return Capture(TokenType::kComma);
        case ';':  return Capture(TokenType::kSemicolon);
        case '~':  return Capture(TokenType::kTilde);
        case '?':  return Capture(TokenType::kQuestion);
        case '\\': return Capture(TokenType::kBackslash);
        // XX
        case '#': return Peek() == '#' ? Capture(TokenType::kSharpSharp, 2) : Capture(TokenType::kSharp);
        case ':': return Peek() == ':' ? Capture(TokenType::kColonColon, 2) : Capture(TokenType::kColon);
        // X, XE
        case '*': return Peek() == '=' ? Capture(TokenType::kStarEqual,    2) : Capture(TokenType::kStar);
        case '/': return Peek() == '=' ? Capture(TokenType::kSlashEqual,   2) : Capture(TokenType::kSlash);
        case '%': return Peek() == '=' ? Capture(TokenType::kPercentEqual, 2) : Capture(TokenType::kPercent);
        case '!': return Peek() == '=' ? Capture(TokenType::kNotEqual,     2) : Capture(TokenType::kExclamation);
        case '=': return Peek() == '=' ? Capture(TokenType::kEqualEqual,   2) : Capture(TokenType::kEqual);
        // X, XX, XE
        case '+':
            if (Peek() == '+') return Capture(TokenType::kPlusPlus,  2);
            if (Peek() == '=') return Capture(TokenType::kPlusEqual, 2);
            return Capture(TokenType::kPlus);
        case '-':
            if (Peek() == '-') return Capture(TokenType::kMinusMinus, 2);
            if (Peek() == '=') return Capture(TokenType::kMinusEqual, 2);
            return Capture(TokenType::kMinus);
        case '&':
            if (Peek() == '&') return Capture(TokenType::kAmpersandAmpersand, 2);
            if (Peek() == '=') return Capture(TokenType::kAmpersandEqual,     2);
            return Capture(TokenType::kAmpersand);
        case '^':
            if (Peek() == '^') return Capture(TokenType::kCaretCaret, 2);
            if (Peek() == '=') return Capture(TokenType::kCaretEqual, 2);
            return Capture(TokenType::kCaret);
        case '|':
            if (Peek() == '|') return Capture(TokenType::kVerticalBarVerticalBar, 2);
            if (Peek() == '=') return Capture(TokenType::kVerticalBarEqual,       2);
            return Capture(TokenType::kVerticalBar);
        // X, XE, XX, XXE
        case '<':
            if (Peek() == '<')
                return Peek(2) == '=' ? Capture(TokenType::kLeftShiftEqual, 3) : Capture(TokenType::kLeftShift, 2);
            return Peek() == '=' ? Capture(TokenType::kLessEqual, 2) : Capture(TokenType::kLessThan);
        case '>':
            if (Peek() == '>')
                return Peek(2) == '=' ? Capture(TokenType::kRightShiftEqual, 3) : Capture(TokenType::kRightShift, 2);
            return Peek() == '=' ? Capture(TokenType::kGreaterEqual, 2) : Capture(TokenType::kGreaterThan);
        // XXX
        case '.':
            if (Peek() == '.' && Peek(2) == '.')
                return Capture(TokenType::kEllipsis, 3);
            return Capture(TokenType::kDot);
        default:
            return {};
        }
    }

    unsigned char Lexer::Peek(std::size_t offset) const {
        if (position_ + offset >= source_.length()) {
            return '\0';
        }
        return source_[position_ + offset];
    }

    Token Lexer::Capture(TokenType type, std::size_t length) {
        const SourceLocation location(source_file_, line_, column_);

        std::string text(source_.substr(position_, length));
        Advance(length);

        return {
            .text     = text,
            .location = location,
            .type     = type
        };
    }

    void Lexer::BuildLexicalTable() {
        if (lexical_table_ != nullptr && !lexical_table_->empty()) {
            return;
        }

        lexical_table_ = MetadataManager::GetInstance().GetLexicalTable();
    }

    void Lexer::SkipWhitespaceAndComments() {
        while (position_ < source_.length()) {
            unsigned char ch = static_cast<unsigned char>(source_[position_]);
            if (IsAsciiSpace(ch) || ch == '\0') {
                Advance();
                continue;
            }

            if (position_ + 1 < source_.length() && source_[position_] == '/') {
                if (source_[position_ + 1] == '/') {
                    Advance(2);
                    while (position_ < source_.length() && source_[position_] != '\n') {
                        Advance();
                    }
                    continue;
                }

                if (source_[position_ + 1] == '*') {
                    Advance(2);
                    while (position_ + 1 < source_.length() && !(source_[position_] == '*' && source_[position_ + 1] == '/')) {
                        Advance();
                    }
                    if (position_ + 1 < source_.length()) {
                        Advance(2);
                    }
                    continue;
                }
            }

            break;
        }
    }

    void Lexer::Advance(std::size_t count) {
        for (auto i = 0uz; i != count; ++i) {
            if (position_ >= source_.length()) {
                return;
            }

            if (source_[position_] == '\n') {
                ++line_;
                column_ = 1;
            } else {
                ++column_;
            }
            ++position_;
        }
    }
}
