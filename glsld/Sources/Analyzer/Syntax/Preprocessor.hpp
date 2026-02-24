#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <unordered_set>
#include <vector>

#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    class PreprocessorEvaluator {

    };

    struct MacroDefination {
        bool               is_function{};
        Token              original_token;
        std::vector<Token> replacement_list;
        std::vector<Token> params;
    };

    class Preprocessor {
    public:
        Preprocessor(MacroTraceMap& trace_map, std::span<const Token> raw_tokens);
        std::vector<Token> Process();

    private:
        void CollectMacroReplacement(MacroDefination& defination);
        bool ExpandMacro(std::unordered_set<std::string>& active_macros, std::vector<Token>& output);

        std::vector<Token> ExpandTokenSequence(std::span<const Token> input,
                                               std::unordered_set<std::string>& active_macros,
                                               SourceLocation call_site);

        std::vector<Token> SubstituteFunctionMacro(const MacroDefination& defination,
                                                   const std::vector<std::vector<Token>>& arguments,
                                                   std::unordered_set<std::string>& active_macros,
                                                   SourceLocation call_site);

        bool ParseFunctionMacroInvocationFromStream(const MacroDefination& defination, std::vector<std::vector<Token>>& arguments);

        bool ParseFunctionMacroInvocationInSequence(std::span<const Token> input, std::size_t open_paren_index,
                                                    std::size_t& close_paren_index, std::vector<std::vector<Token>>& arguments);

        std::vector<Token> ApplyTokenPasting(std::span<const Token> tokens);
        Token PasteTokens(const Token& left, const Token& right);

        const Token& current_token() const;
        const Token& PeekToken(std::int64_t offset = 1) const;
        void ConsumeToken(std::ptrdiff_t count = 1);
        bool MatchAndConsume(TokenType type);

        StringHeteroHashTable<std::string, MacroDefination> macros_;
        MacroTraceMap&                                      trace_map_;
        std::span<const Token>                              raw_tokens_;
        std::size_t                                         token_index_{};
    };
}

#include "Preprocessor.inl"
