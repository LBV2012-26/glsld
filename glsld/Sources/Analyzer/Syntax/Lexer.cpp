#include "stdafx.h"
#include "Lexer.hpp"

#include <cctype>
#include <format>
#include <fstream>
#include <ios>
#include <ranges>
#include <stdexcept>
#include <vector>

#include "Utils/Utils.hpp"

namespace glsld {
    StringHeteroHashTable<std::string, TokenType> Lexer::lexical_table_;

    Lexer::Lexer(std::string_view source)
        : source_{ source }
    {
        BuildLexicalTable();
    }

    Token Lexer::AcquireNextToken() {
        SkipWhitespaceAndComments();

        const SourceLocation location{ line_, column_ };

        if (position_ >= source_.length()) {
            return { {}, location, TokenType::kEndOfFile };
        }

        unsigned char current_char = static_cast<unsigned char>(source_[position_]);
        auto token = DetectToken(current_char);
        if (token.type != TokenType::kUnknown) {
            return token;
        }

        auto IsIdentifierStart = [](unsigned char ch) -> bool {
            return std::isalpha(ch) || ch == '_';
        };

        auto IsIdentifierChar = [](unsigned char ch) -> bool {
            return std::isalnum(ch) || ch == '_';
        };

        if (IsIdentifierStart(current_char)) {
            std::size_t begin = position_;
            Advance();
            while (position_ < source_.length() && IsIdentifierChar(static_cast<unsigned char>(source_[position_]))) {
                Advance();
            }
            std::string_view word = source_.substr(begin, position_ - begin);

            auto it = lexical_table_.find(word);
            if (it != lexical_table_.end()) {
                return { std::string(word), location, it->second };
            }

            return { std::string(word), location, TokenType::kIdentifier };
        }

        auto IsIdentifierAlnum = [](unsigned char ch) -> bool {
            return std::isalnum(ch) || ch == '.' || ch == '_';
        };

        if (std::isdigit(current_char)) {
            std::size_t begin = position_;
            Advance();
            while (position_ < source_.length() && IsIdentifierAlnum(static_cast<unsigned char>(source_[position_]))) {
                Advance();
            }

            return { std::string(source_.substr(begin, position_ - begin)), location, TokenType::kNumberLiteral };
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

            return { std::string(source_.substr(begin, position_ - begin)), location, TokenType::kStringLiteral };
        }

        Advance();
        return { std::string(source_.substr(position_ - 1, 1)), location, TokenType::kUnknown };
    }

    Token Lexer::DetectToken(unsigned char current_char) {
        switch (current_char) {
        case '{':  return Capture(TokenType::kOpenBrace);
        case '}':  return Capture(TokenType::kCloseBrace);
        case '[':  return Capture(TokenType::kOpenBracket);
        case ']':  return Capture(TokenType::kCloseBracket);
        case '(':  return Capture(TokenType::kOpenParen);
        case ')':  return Capture(TokenType::kCloseParen);
        case ',':  return Capture(TokenType::kComma);
        case ':':  return Capture(TokenType::kColon);
        case ';':  return Capture(TokenType::kSemicolon);
        case '.':  return Capture(TokenType::kDot);
        case '#':  return Capture(TokenType::kSharp);
        case '~':  return Capture(TokenType::kTilde);
        case '?':  return Capture(TokenType::kQuestion);
        case '\\': return Capture(TokenType::kBackslash);
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
        const SourceLocation location{ line_, column_ };
        std::string text(source_.substr(position_, length));
        Advance(length);
        return { text, location, type };
    }

    void Lexer::BuildLexicalTable() {
        if (!lexical_table_.empty()) {
            return;
        }

        LoadLexicalFile(utils::GetFilePath("Assets/glslFunctions.txt"), TokenType::kBuiltInFunction);
        LoadLexicalFile(utils::GetFilePath("Assets/glslKeywords.txt"), TokenType::kKeyword);
        LoadLexicalFile(utils::GetFilePath("Assets/glslPreprocessors.txt"), TokenType::kPreprocessor);
        LoadLexicalFile(utils::GetFilePath("Assets/glslPrimitives.txt"), TokenType::kPrimitive);
        LoadLexicalFile(utils::GetFilePath("Assets/glslTypes.txt"), TokenType::kBuiltInType);
        LoadLexicalFile(utils::GetFilePath("Assets/glslVariables.txt"), TokenType::kBuiltInVariable);
    }

    void Lexer::LoadLexicalFile(std::string_view filename, TokenType type) {
        std::ifstream stream(filename.data());
        if (!stream.is_open()) {
            throw std::runtime_error(std::format("Failed to open {}: No such file or directory.", filename));
        }

        stream.seekg(0, std::ios::end);
        auto size = stream.tellg();
        stream.seekg(0);

        std::vector<char> buffer(size);
        stream.read(buffer.data(), size);

        auto ExtractWords = [](std::span<const char> text) -> std::vector<std::string_view> {
            auto words_range = text | std::views::chunk_by([](auto cha, auto chb) -> bool {
                return (std::isspace(static_cast<unsigned char>(cha)) ==
                        std::isspace(static_cast<unsigned char>(chb)));
            }) | std::views::filter([](auto chunk) -> bool {
                return !std::isspace(static_cast<unsigned char>(chunk.front()));
            }) | std::views::transform([](auto word_view) -> std::string_view {
                return std::string_view(word_view);
            });

            return std::ranges::to<std::vector<std::string_view>>(words_range);
        };

        auto words = ExtractWords(buffer);
        InsertTable(words, type);
    }

    void Lexer::InsertTable(std::span<const std::string_view> words, TokenType type) {
        for (auto& word : words) {
            lexical_table_.try_emplace(std::string(word), type);
        }
    }

    void Lexer::SkipWhitespaceAndComments() {
        while (position_ < source_.length()) {
            unsigned char ch = static_cast<unsigned char>(source_[position_]);
            if (std::isspace(ch) || ch == '\0') {
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
        for (std::size_t i = 0; i != count; ++i) {
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
