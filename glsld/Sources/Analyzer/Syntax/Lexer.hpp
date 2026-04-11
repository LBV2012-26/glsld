#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "Analyzer/Syntax/Token.hpp"
#include "Base/Hash.hpp"
#include "Base/IncludeLoader.hpp"

namespace glsld {
    class Lexer {
    public:
        Lexer(std::string_view source,
              IncludeLoader& include_loader,
              std::string_view current_uri,
              std::span<const std::filesystem::path> include_dirs = {},
              SourceReference source_ref = nullptr);

        Token AcquireNextToken();

    private:
        void TryPrefetchInclude();
        std::optional<std::pair<std::size_t, std::size_t>> FindIncludeExprAfterSharp(std::size_t sharp_index) const;
        std::optional<std::size_t> TryFindIncludeAngleEnd() const;
        Token DetectToken(unsigned char current_char);
        unsigned char Peek(std::size_t offset = 1) const;
        Token Capture(TokenType type, std::size_t length = 1);
        void BuildLexicalTable();
        void LoadLexicalFile(std::string_view filename, TokenType type);
        void SkipWhitespaceAndComments();
        void Advance(std::size_t count = 1);

        std::string_view                               source_;
        IncludeLoader&                                 include_loader_;
        std::string                                    current_uri_;
        std::span<const std::filesystem::path>         include_dirs_;
        std::size_t                                    position_{};
        std::size_t                                    line_{ 1 };
        std::size_t                                    column_{ 1 };
        SourceReference                                source_ref_;

        static inline StringHeteroHashTable<TokenType> lexical_table_;
    };
}
