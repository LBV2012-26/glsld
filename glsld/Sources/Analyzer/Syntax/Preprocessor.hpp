#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <stack>
#include <string>
#include <vector>

#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Base/FileSystem/IncludeLoader.hpp"
#include "Base/FileSystem/Source.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    struct ConditionalFrame {
        bool          parent_active{ true };
        bool          branch_taken{ false };
        bool          current_active{ true };
        bool          in_else{ false };
        std::uint32_t if_line{};
    };

    class Preprocessor {
    public:
        Preprocessor(SourceTable& source_table,
                     const SourceFile* source_file,
                     IncludeLoader& include_loader,
                     IncludeDirectoryHandle include_dirs,
                     std::span<const Token> raw_tokens,
                     Document& document,
                     std::vector<std::string> parent_stack = {});

        std::vector<Token> Process();

    private:
        MacroDefinition CollectMacroReplacement(std::size_t current_physical_line);
        std::vector<Token> CaptureDirectiveBodyTokens(std::size_t directive_physical_line);
        bool ExpandMacro(StringHeteroHashSet& active_macros, std::vector<Token>& output);

        std::vector<Token> ExpandTokenSequence(
            std::span<const Token> input,
            StringHeteroHashSet& active_macros,
            const SourceLocation& call_site);

        std::vector<Token> SubstituteFunctionMacro(
            const MacroDefinition& definition,
            const std::vector<std::vector<Token>>& arguments,
            StringHeteroHashSet& active_macros,
            const SourceLocation& call_site);

        bool ParseFunctionMacroInvocationFromStream(std::vector<std::vector<Token>>& arguments);

        bool ParseFunctionMacroInvocationInSequence(
            std::span<const Token> input,
            std::size_t open_paren_index,
            std::size_t& close_paren_index,
            std::vector<std::vector<Token>>& arguments);

        std::vector<Token> ApplyTokenPasting(std::span<const Token> tokens);
        Token PasteTokens(const Token& left, const Token& right);

        bool IsCurrentBranchActive() const;
        void HandleDirectiveAtSharp(std::vector<Token>& output);
        void ParseDefineFromBody(std::span<const Token> body_tokens);
        bool HandleConditionalDirective(std::string_view directive, std::span<const Token> body_tokens, std::uint32_t sharp_line);
        bool EvaluateIfCondition(std::span<const Token> expr_tokens);
        std::vector<Token> ExpandIfExpression(std::span<const Token> input, StringHeteroHashSet& active_macros);
        void AppendInactiveRegion(std::uint32_t begin_line, std::uint32_t end_line);
        void UpdateInactiveRegions(bool was_active, bool now_active, std::uint32_t directive_line);
        void FinalizeInactiveRegions(std::uint32_t eof_line);
        std::vector<Token> ExpandIncludeDirective(std::span<const Token> body_tokens);

        const Token& current_token() const;
        const Token& PeekToken(std::int64_t offset = 1) const;
        void ConsumeToken(std::ptrdiff_t count = 1);
        bool MatchAndConsume(TokenType type);

        SourceTable&                 source_table_;
        const SourceFile*            source_file_;
        IncludeLoader&               include_loader_;
        IncludeDirectoryHandle       include_dirs_;
        std::vector<std::string>     include_stack_;
        std::stack<ConditionalFrame> condition_stack_;
        std::optional<std::uint32_t> open_inactive_begin_line_;
        std::size_t                  token_index_{};
        std::span<const Token>       raw_tokens_;
        Document&                    document_;
    };
}

#include "Preprocessor.inl"
