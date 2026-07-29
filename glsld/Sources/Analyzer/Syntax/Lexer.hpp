#pragma once

#include <cstddef>
#include <atomic>
#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Base/FileSystem/IncludeLoader.hpp"
#include "Base/FileSystem/Source.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    class Lexer {
    public:
        Lexer(const SourceFile* source_file,
              std::string_view source,
              IncludeLoader& include_loader,
              IncludeDirectoryHandle include_dirs);

        std::vector<Token> Tokenize(int version_replica = 0, VersionPointer version_pointer = nullptr);

    private:
        friend class Parser;
        friend class Preprocessor;

        Token AcquireNextToken();
        Token ProduceToken();
        void TryPrefetchInclude();
        std::optional<std::pair<std::size_t, std::size_t>> FindIncludeExprAfterSharp(std::size_t sharp_index) const;
        std::optional<std::size_t> TryFindIncludeAngleEnd() const;
        Token DetectToken(unsigned char current_char);
        unsigned char Peek(std::size_t offset = 1) const;
        Token Capture(TokenType type, std::size_t length = 1);

        void BuildLexicalTable();
        void SkipWhitespaceAndComments();
        void Advance(std::size_t count = 1);

        const SourceFile*      source_file_;
        std::string_view       source_;
        IncludeLoader&         include_loader_;
        IncludeDirectoryHandle include_dirs_;
        std::size_t            position_{};
        std::size_t            line_{ 1 };
        std::size_t            column_{ 1 };
        int                    qualifier_paren_depth_{};
        bool                   preprocessor_line_{ false };
        bool                   qualifier_after_equal_{ false };
        std::size_t            last_token_line_{};
        std::string            last_token_text_{};

        static inline const StringHeteroHashMap<TokenType>* lexical_table_{ nullptr };
    };
}
