#include "stdafx.h"
#include "Preprocessor.hpp"

#include <ranges>
#include <utility>
#include "Analyzer/Syntax/Lexer.hpp"

namespace glsld {
    Preprocessor::Preprocessor(MacroTraceMap& trace_map, MacroArgsTraceMap& args_trace_map, std::span<const Token> raw_tokens)
        : trace_map_{ trace_map }
        , args_trace_map_{ args_trace_map }
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

                if (defination.is_function) {
                    expanded.push_back(Token{
                        .text     = "(",
                        .location = { name_token.location.line, name_token.location.column + name_token.text.length() },
                        .type     = TokenType::kOpenParen
                    });

                    for (auto i = 0uz; i != defination.params.size(); ++i) {
                        expanded.push_back(defination.params[i]);
                        if (i + 1 != defination.params.size()) {
                            expanded.push_back(Token{
                                .text     = ",",
                                .location = { defination.params[i].location.line, defination.params[i].location.column + defination.params[i].text.length() },
                                .type     = TokenType::kComma
                            });
                        }
                    }

                    const auto& last_token = expanded.back();
                    expanded.push_back(Token{
                        .text     = ")",
                        .location = { last_token.location.line, last_token.location.column + last_token.text.length() },
                        .type     = TokenType::kCloseParen
                    });
                }

                for (const auto& replaced : defination.replacement_list) {
                    expanded.push_back(replaced);
                }
            } else {
                expanded.push_back(current_token());
                ConsumeToken();
            }

            if (macros_.contains(current_token().text)) {
                std::unordered_set<std::string> active_macros;
                if (ExpandMacro(active_macros, expanded)) {
                    continue;
                }
            }
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
                if (current_token().type == TokenType::kCloseParen) {
                    break;
                }

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

    bool Preprocessor::ExpandMacro(std::unordered_set<std::string>& active_macros, std::vector<Token>& output) {
        // current token is macro name
        const auto& macro_token = current_token();

        auto it = macros_.find(macro_token.text);
        if (it == macros_.end()) {
            output.push_back(macro_token);
            ConsumeToken();
            return true;
        }

        if (active_macros.contains(macro_token.text)) {
            return false;
        }

        trace_map_.try_emplace(macro_token.location, it->second.original_token);

        active_macros.insert(macro_token.text);
        const auto& defination = it->second;

        if (!defination.is_function) {
            ConsumeToken(); // consume macro name token
            auto replaced = ExpandTokenSequence(defination.replacement_list, active_macros, macro_token.location);
            output.append_range(replaced | std::views::as_rvalue);
            active_macros.erase(macro_token.text);
            return true;
        }

        if (PeekToken().type != TokenType::kOpenParen) {
            active_macros.erase(macro_token.text);
            return false;
        }

        std::vector<std::vector<Token>> arguments;
        if (!ParseFunctionMacroInvocationFromStream(defination, arguments)) {
            active_macros.erase(macro_token.text);
            return false;
        }

        auto replaced = SubstituteFunctionMacro(defination, arguments, active_macros, macro_token.location);
        output.append_range(replaced | std::views::as_rvalue);
        active_macros.erase(macro_token.text);
        return true;
    }

    std::vector<Token> Preprocessor::ExpandTokenSequence(std::span<const Token> input,
                                                         std::unordered_set<std::string>& active_macros,
                                                         SourceLocation call_site)
    {
        std::vector<Token> result;

        auto PushAtCallSite = [&](Token token) -> void {
            token.location = call_site;
            result.push_back(std::move(token));
        };

        auto AppendRangeAtCallSite = [&](std::span<Token> tokens) -> void {
            for (auto& token : tokens) {
                token.location = call_site;
            }

            result.append_range(tokens | std::views::as_rvalue);
        };

        for (auto i = 0uz; i != input.size(); ++i) {
            const auto& token = input[i];

            if (token.type != TokenType::kIdentifier) {
                PushAtCallSite(token);
                continue;
            }

            auto it = macros_.find(token.text);
            if (it == macros_.end() || active_macros.contains(token.text)) {
                PushAtCallSite(token);
                continue;
            }

            const auto& defination = it->second;
            const auto& macro_name = token.text;
            active_macros.insert(macro_name);

            if (!defination.is_function) {
                auto nested = ExpandTokenSequence(defination.replacement_list, active_macros, call_site);
                AppendRangeAtCallSite(nested);
                active_macros.erase(macro_name);
                continue;
            }

            if (i + 1 >= input.size() || input[i + 1].type != TokenType::kOpenParen) {
                PushAtCallSite(token);
                active_macros.erase(macro_name);
                continue;
            }

            auto close_paren_index = 0uz;
            std::vector<std::vector<Token>> arguments;
            if (!ParseFunctionMacroInvocationInSequence(input, i + 1, close_paren_index, arguments)) {
                PushAtCallSite(token);
                active_macros.erase(macro_name);
                continue;
            }

            auto replaced = SubstituteFunctionMacro(defination, arguments, active_macros, call_site);
            AppendRangeAtCallSite(replaced);
            i = close_paren_index;
            active_macros.erase(macro_name);
        }

        return result;
    }

    std::vector<Token> Preprocessor::SubstituteFunctionMacro(const MacroDefination& defination,
                                                             const std::vector<std::vector<Token>>& arguments,
                                                             std::unordered_set<std::string>& active_macros,
                                                             SourceLocation call_site)
    {
        std::unordered_map<std::string, std::size_t> param_index;
        for (auto i = 0uz; i != defination.params.size(); ++i) {
            param_index.try_emplace(defination.params[i].text, i);
        }

        auto arguments_active_macros = active_macros;
        arguments_active_macros.erase(defination.original_token.text);

        std::vector<std::vector<Token>> expanded_args(arguments.size());
        for (auto i = 0uz; i != arguments.size(); ++i) {
            expanded_args[i] = ExpandTokenSequence(arguments[i], arguments_active_macros, call_site);
            for (auto& token : expanded_args[i]) {
                token.location = call_site;
            }
        }

        auto AppendWithCallSiteWithoutMove = [&](std::vector<Token>& target, std::span<const Token> source) -> void {
            for (auto token : source) {
                token.location = call_site;
                target.push_back(std::move(token));
            }
        };

        auto IsAdjacentToTokenPaste = [&](std::size_t replace_index) -> bool {
            const auto& replacement_list = defination.replacement_list;
            if (replace_index > 0 && replacement_list[replace_index - 1].type == TokenType::kSharpSharp) {
                return true;
            }

            if (replace_index + 1 < replacement_list.size() && replacement_list[replace_index + 1].type == TokenType::kSharpSharp) {
                return true;
            }

            return false;
        };

        std::vector<Token> replaced;
        for (auto i = 0uz; i != defination.replacement_list.size(); ++i) {
            const auto& token = defination.replacement_list[i];
            if (token.type == TokenType::kIdentifier) {
                auto it = param_index.find(token.text);
                if (it != param_index.end()) {
                    const auto& arg_index = it->second;
                    if (arg_index < arguments.size()) {
                        if (IsAdjacentToTokenPaste(i)) {
                            AppendWithCallSiteWithoutMove(replaced, arguments[arg_index]);
                        } else {
                            AppendWithCallSiteWithoutMove(replaced, expanded_args[arg_index]);
                        }
                    }

                    continue;
                }
            }

            auto copied = token;
            copied.location = call_site;
            replaced.push_back(copied);
        }

        auto pasted    = ApplyTokenPasting(replaced);
        auto rescanned = ExpandTokenSequence(pasted, active_macros, call_site);
        for (auto& token : rescanned) {
            token.location = call_site;
        }

        return rescanned;
    }

    bool Preprocessor::ParseFunctionMacroInvocationFromStream(const MacroDefination& defination, std::vector<std::vector<Token>>& arguments) {
        const auto name_index        = token_index_;
        const auto open_paren_index  = token_index_ + 1;
        auto       close_paren_index = 0uz;

        if (!ParseFunctionMacroInvocationInSequence(raw_tokens_, open_paren_index, close_paren_index, arguments)) {
            return false;
        }

        token_index_ = close_paren_index + 1;
        return true;
    }

    bool Preprocessor::ParseFunctionMacroInvocationInSequence(std::span<const Token> input, std::size_t open_paren_index,
                                                              std::size_t& close_paren_index, std::vector<std::vector<Token>>& arguments)
    {
        if (open_paren_index >= input.size() || input[open_paren_index].type != TokenType::kOpenParen) {
            return false;
        }

        auto paren_level   = 0uz;
        auto bracket_level = 0uz;
        auto brace_level   = 0uz;
        auto index         = open_paren_index + 1;

        arguments.clear();
        arguments.emplace_back();

        // 空参数列表
        if (index < input.size() && input[index].type == TokenType::kCloseParen) {
            arguments.clear();
            close_paren_index = index;
            return true;
        }

        auto PushArgument = [&](const Token& token) -> void {
            arguments.back().push_back(token);
            args_trace_map_.try_emplace(token.location, token);
        };

        for (; index < input.size(); ++index) {
            const auto& token = input[index];

            if (token.type == TokenType::kEndOfFile) {
                return false;
            }

            switch (token.type) {
            case TokenType::kOpenParen:
                ++paren_level;
                PushArgument(token);
                break;
            case TokenType::kCloseParen:
                if (paren_level == 0 && bracket_level == 0 && brace_level == 0) {
                    close_paren_index = index;
                    return true;
                }

                --paren_level;
                PushArgument(token);
                break;
            case TokenType::kOpenBracket:
                ++bracket_level;
                PushArgument(token);
                break;
            case TokenType::kCloseBracket:
                if (bracket_level == 0) {
                    return false;
                }

                --bracket_level;
                PushArgument(token);
                break;
            case TokenType::kOpenBrace:
                ++brace_level;
                PushArgument(token);
                break;
            case TokenType::kCloseBrace:
                if (brace_level == 0) {
                    return false;
                }

                --brace_level;
                PushArgument(token);
                break;
            case TokenType::kComma:
                if (paren_level == 0 && bracket_level == 0 && brace_level == 0) {
                    arguments.emplace_back();
                } else {
                    PushArgument(token);
                }

                break;
            default:
                PushArgument(token);
                break;
            }
        }

        return false;
    }

    std::vector<Token> Preprocessor::ApplyTokenPasting(std::span<const Token> tokens) {
        std::vector<Token> result;

        for (auto i = 0uz; i < tokens.size(); ++i) {
            const auto& token = tokens[i];

            if (token.type != TokenType::kSharpSharp) {
                result.push_back(token);
                continue;
            }

            if (result.empty() || i + 1 >= tokens.size()) {
                continue;
            }

            auto left = result.back();
            result.pop_back();
            const auto& right = tokens[++i];
            auto pasted = PasteTokens(left, right);
            result.push_back(std::move(pasted));
        }

        return result;
    }

    Token Preprocessor::PasteTokens(const Token& left, const Token& right) {
        std::string new_text = left.text + right.text;

        Lexer lexer(new_text);
        Token token = lexer.AcquireNextToken();
        token.location = left.location;

        return token;
    }
}
