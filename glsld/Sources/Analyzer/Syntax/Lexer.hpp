#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "Analyzer/Syntax/Token.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    class Lexer {
    public:
        Lexer(std::string_view source);

        Token AcquireNextToken();

    private:
        Token DetectToken(unsigned char current_char);
        unsigned char Peek(std::size_t offset = 1) const;
        Token Capture(TokenType type, std::size_t length = 1);
        void BuildLexicalTable();
        void LoadLexicalFile(std::string_view filename, TokenType type);
        void InsertTable(std::span<const std::string_view> words, TokenType type);
        void SkipWhitespaceAndComments();
        void Advance(std::size_t count = 1);

        std::string_view source_;
        std::size_t      position_{};
        std::size_t      line_{ 1 };
        std::size_t      column_{ 1 };

        static StringHeteroHashTable<std::string, TokenType> lexical_table_;
    };
}
