#include "stdafx.h"
#include "Preprocessor.hpp"

#include "Analyzer/Syntax/Lexer.hpp"

namespace glsld {
    Preprocessor::Preprocessor(MacroTraceMap& trace_map, std::span<const Token> raw_tokens)
        : trace_map_{ trace_map }
        , raw_tokens_{ raw_tokens }
    {}

    std::vector<Token> Preprocessor::Process() {
        std::vector<Token> expanded;

        while (current_token().type != TokenType::kEndOfFile) {
            if (current_token().type == TokenType::kSharp && PeekToken().text == "define") {
                expanded.push_back(current_token()); // #
                ConsumeToken();
                expanded.push_back(current_token()); // define
                ConsumeToken();

                const auto& name_token = current_token();
                expanded.push_back(name_token); // macro name

                MacroDefination defination;
                defination.original_token = name_token;
                ConsumeToken();

                CollectMacroReplacement(defination);
                macros_.try_emplace(name_token.text, defination);

                for (auto i = 0uz; i != defination.params.size(); ++i) {
                    if (i == 0) {
                        expanded.push_back(Token{
                            .text     = "(",
                            .location = { name_token.location.line, name_token.location.column + name_token.text.length() },
                            .type     = TokenType::kOpenParen
                        });
                    }

                    expanded.push_back(defination.params[i]);

                    if (i + 1 == defination.params.size()) {
                        const auto& last_token = expanded.back();
                        expanded.push_back(Token{
                            .text     = ")",
                            .location = { last_token.location.line, last_token.location.column + last_token.text.length() },
                            .type     = TokenType::kCloseParen
                        });
                    } else {
                        expanded.push_back(Token{
                            .text     = ",",
                            .location = { defination.params[i].location.line, defination.params[i].location.column + defination.params[i].text.length() },
                            .type     = TokenType::kComma
                        });
                    }
                }

                for (const auto& replaced : defination.replacement_list) {
                    expanded.push_back(replaced);
                }
            }

            if (macros_.contains(current_token().text)) {
                const auto& macro_token = current_token();
                auto it = macros_.find(macro_token.text);
                trace_map_.try_emplace(macro_token.location, it->second.original_token);

                std::unordered_set<std::string> active_macros;
                ExpandMacro(macro_token, active_macros, expanded);
                ConsumeToken();
                continue;
            }

            expanded.push_back(current_token());
            ConsumeToken();
        }

        expanded.push_back(current_token());
        return expanded;
    }

    void Preprocessor::CollectMacroReplacement(MacroDefination& defination) {
        const auto& prev_token = PeekToken(-1);
        const auto& this_token = current_token();
        if (this_token.type == TokenType::kOpenParen &&
            prev_token.location.column + prev_token.text.length() == this_token.location.column)
        {
            defination.is_function = true;
            ConsumeToken();
        }

        if (defination.is_function) {
            do {
                MatchAndConsume(TokenType::kComma);
                const auto& param_token = current_token();
                defination.params.push_back(param_token);
                ConsumeToken();
            } while (current_token().type == TokenType::kComma);

            MatchAndConsume(TokenType::kCloseParen);
        }

        std::size_t current_physical_line = current_token().location.line;

        while (current_token().type != TokenType::kEndOfFile) {
            const auto& token = current_token();
            if (token.location.line > current_physical_line) {
                auto& replacement_list = defination.replacement_list;
                if (!replacement_list.empty() && replacement_list.back().type == TokenType::kBackslash) {
                    replacement_list.pop_back();
                    current_physical_line = token.location.line;
                } else {
                    break;
                }
            }

            defination.replacement_list.push_back(token);
            ConsumeToken();
        }
    }

    void Preprocessor::ExpandMacro(const Token& macro_token, std::unordered_set<std::string>& active_macros, std::vector<Token>& output) {
        // current token is macro name
        const auto& macro_name = macro_token.text;

        if (active_macros.contains(macro_name)) {
            output.push_back(macro_token);
            return;
        }

        auto it = macros_.find(macro_name);
        if (macros_.find(macro_name) == macros_.end()) {
            output.push_back(macro_token);
            return;
        }

        if (it->second.is_function) {
            ConsumeToken();
            if (current_token().type != TokenType::kOpenParen) {
                output.push_back(macro_token);
                return;
            }

            // TODO: expand macro function
            while (!MatchAndConsume(TokenType::kCloseParen)) {
                ConsumeToken();
            }
        }

        const auto& replacement_list = it->second.replacement_list;
        std::vector<Token> pasted_list;
        for (auto i = 0uz; i != replacement_list.size(); ++i) {
            if (i + 1 < replacement_list.size() && replacement_list[i + 1].type == TokenType::kSharpSharp) {
                Token current = replacement_list[i];
                while (i + 1 < replacement_list.size() && replacement_list[i + 1].type == TokenType::kSharpSharp) {
                    if (i + 2 < replacement_list.size()) {
                        Token next = replacement_list[i + 2];
                        current = PasteTokens(current, next);
                        i += 2;
                    } else {
                        break;
                    }
                }

                pasted_list.push_back(current);
            } else {
                pasted_list.push_back(replacement_list[i]);
            }
        }

        active_macros.insert(macro_name);

        for (const auto& replaced_token : pasted_list) {
            auto new_token = replaced_token;
            new_token.location = macro_token.location;

            if (new_token.type == TokenType::kIdentifier) {
                ExpandMacro(new_token, active_macros, output);
            } else {
                output.push_back(new_token);
            }
        }

        active_macros.erase(macro_name);
    }

    Token Preprocessor::PasteTokens(const Token& left, const Token& right) {
        std::string new_text = left.text + right.text;

        Lexer lexer(new_text);
        Token token = lexer.AcquireNextToken();
        token.location = left.location;

        return token;
    }
}
