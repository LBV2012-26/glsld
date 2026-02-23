#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <unordered_set>
#include <vector>

#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    struct MacroDefination {
        bool                     is_macro_func{};
        Token                    original_token;
        std::vector<Token>       replacement_list;
        std::vector<std::string> params;
    };

    class Preprocessor {
    public:
        Preprocessor(MacroTraceMap& trace_map);
        std::vector<Token> Process(std::span<const Token> raw_tokens);

    private:
        void CollectMacroReplacement(std::size_t index, std::span<const Token> tokens, MacroDefination& defination);
        void ExpandMacro(const Token& macro_token, std::vector<Token>& output, std::unordered_set<std::string>& active_macros);
        Token PasteTokens(const Token& left, const Token& right);

        StringHeteroHashTable<std::string, MacroDefination> macros_;
        MacroTraceMap&                                      trace_map_;
    };
}
