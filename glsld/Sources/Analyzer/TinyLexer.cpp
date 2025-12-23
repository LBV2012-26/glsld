#include "stdafx.h"
#include "TinyLexer.hpp"

#include <cctype>
#include <format>
#include <fstream>
#include <ios>
#include <iosfwd>
#include <ranges>
#include <span>
#include <stdexcept>
#include <vector>

namespace glsld {
    utils::StringHeteroHashTable<std::string, TokenType> TinyLexer::lexical_table_;

    TinyLexer::TinyLexer(std::string_view source)
        : source_{ source }
    {
        BuildLexicalTable();
    }

    Token TinyLexer::AcquireNextToken() {
        SkipWhitespaceAndComments();

        const SourceLocation location = { line_, column_ };

        if (position_ >= source_.length()) {
            return { {}, location, TokenType::kEndOfFile };
        }

        auto ConstructToken = [this, &location](TokenType type) -> Token {
            return { std::string(source_.substr(position_ - 1, 1)), location, type };
        };

        unsigned char current_char = static_cast<unsigned char>(source_[position_]);
        switch (current_char) {
        case '{':
            Advance();
            return ConstructToken(TokenType::kOpenBrace);
        case '}':
            Advance();
            return ConstructToken(TokenType::kCloseBrace);
        case '[':
            Advance();
            return ConstructToken(TokenType::kOpenBracket);
        case ']':
            Advance();
            return ConstructToken(TokenType::kCloseBracket);
        case '(':
            Advance();
            return ConstructToken(TokenType::kOpenParen);
        case ')':
            Advance();
            return ConstructToken(TokenType::kCloseParen);
        case ',':
            Advance();
            return ConstructToken(TokenType::kComma);
        case ':':
            Advance();
            return ConstructToken(TokenType::kColon);
        case ';':
            Advance();
            return ConstructToken(TokenType::kSemicolon);
        case '+':
            Advance();
            return ConstructToken(TokenType::kPlus);
        case '-':
            Advance();
            return ConstructToken(TokenType::kMinus);
        case '*':
            Advance();
            return ConstructToken(TokenType::kStar);
        case '=':
            Advance();
            return ConstructToken(TokenType::kEqual);
        case '#':
            Advance();
            return ConstructToken(TokenType::kSharp);
        case '/':
            Advance();
            return ConstructToken(TokenType::kSlash);
        case '\\':
            Advance();
            return ConstructToken(TokenType::kBackslash);
        default:
            break;
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
            while (position_ < source_.length() && IsIdentifierChar(source_[position_])) {
                Advance();
            }
            std::string_view word = source_.substr(begin, position_ - begin);

            auto it = lexical_table_.find(word);
            if (it != lexical_table_.end()) {
                return { std::string(word), location, it->second };
            }

            return { std::string(word), location, TokenType::kIdentifier };
        }

        if (std::isdigit(current_char)) {
            std::size_t begin = position_;
            Advance();
            while (position_ < source_.length() && std::isalnum(source_[position_]) || source_[position_] == '.') {
                Advance();
            }

            return { std::string(source_.substr(begin, position_ - begin)), location, TokenType::kNumberLiteral };
        }

        if (current_char == '"') {
            std::size_t begin = position_;
            Advance(); // Consume opening quote
            while (position_ < source_.length() && source_[position_] != '"') {
                if (source_[position_] == '\\') { // Handle escaped quotes
                    Advance();
                }
                Advance();
            }

            if (position_ < source_.length()) {
                Advance(); // Consume closing quote
            }

            return { std::string(source_.substr(begin, position_ - begin)), location, TokenType::kStringLiteral };
        }

        Advance();
        return ConstructToken(TokenType::kUnknown);
    }

    void TinyLexer::BuildLexicalTable() {
        if (!lexical_table_.empty()) {
            return;
        }

        LoadLexicalFile(utils::GetFilePath("Assets/glslControlKeywords.txt"), TokenType::kKeyword_Control);
        LoadLexicalFile(utils::GetFilePath("Assets/glslFunctions.txt"), TokenType::kBuiltInFunction);
        LoadLexicalFile(utils::GetFilePath("Assets/glslKeywords.txt"), TokenType::kKeyword);
        LoadLexicalFile(utils::GetFilePath("Assets/glslPreprocessorDirectives.txt"), TokenType::kPreprocessor);
        LoadLexicalFile(utils::GetFilePath("Assets/glslTypes.txt"), TokenType::kKeyword_Typed);
        LoadLexicalFile(utils::GetFilePath("Assets/glslVariables.txt"), TokenType::kBuiltInVariable);
    }

    void TinyLexer::LoadLexicalFile(std::string_view filename, TokenType type) {
        std::ifstream stream(filename.data());
        if (!stream.is_open()) {
            throw std::runtime_error(std::format("Failed to open {}: No such file or directory.", filename));
        }

        stream.seekg(0, std::ios::end);
        std::streampos size = stream.tellg();
        stream.seekg(0);

        std::vector<char> buffer(size);
        stream.read(buffer.data(), size);

        auto ExtractWords = [](std::span<const char> text) -> std::vector<std::string> {
            auto words_range = text | std::views::chunk_by([](auto cha, auto chb) -> bool {
                return (std::isspace(static_cast<unsigned char>(cha)) ==
                        std::isspace(static_cast<unsigned char>(chb)));
            }) | std::views::filter([](auto chunk) -> bool {
                return !std::isspace(static_cast<unsigned char>(chunk.front()));
            }) | std::views::transform([](auto word_view) -> std::string {
                return std::string(word_view.begin(), word_view.end());
            });

            return std::vector<std::string>(words_range.begin(), words_range.end());
        };

        auto words = ExtractWords(buffer);
        InsertTable(words, type);
    }

    void TinyLexer::InsertTable(std::span<const std::string> words, TokenType type) {
        for (auto& word : words) {
            lexical_table_.try_emplace(word, type);
        }
    }

    void TinyLexer::SkipWhitespaceAndComments() {
        while (position_ < source_.length()) {
            if (std::isspace(static_cast<unsigned char>(source_[position_]))) {
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

    void TinyLexer::Advance(std::size_t count) {
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
