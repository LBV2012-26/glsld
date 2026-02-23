#include "stdafx.h"
#include "Preprocessor.hpp"

namespace glsld {
    Preprocessor::Preprocessor(MacroTraceMap& trace_map)
        : trace_map_{ trace_map }
    {}

    std::vector<Token> Preprocessor::Process(std::span<const Token> raw_tokens) {
        std::vector<Token> expanded;
        std::size_t index = 0;

        while (index < raw_tokens.size()) {
            const auto& token = raw_tokens[index];

            if (token.type == TokenType::kSharp && index + 1 < raw_tokens.size() && raw_tokens[index + 1].text == "define") {
                expanded.push_back(token); // #
                expanded.push_back(raw_tokens[++index]); // define

                if (index + 1 < raw_tokens.size()) {
                    const auto& name_token = raw_tokens[++index];
                    expanded.push_back(name_token); // macro name

                    MacroDefination defination;
                    defination.original_token = name_token;
                    ++index;

                    CollectMacroReplacement(index, raw_tokens, defination);
                    macros_.try_emplace(name_token.text, defination);

                    for (const auto& replaced : defination.replacement_list) {
                        expanded.push_back(replaced);
                        ++index;
                    }

                    continue;
                }
            }

            if (macros_.contains(token.text)) {
                auto it = macros_.find(token.text);
                trace_map_.try_emplace(token.location, it->second.original_token);

                std::unordered_set<std::string> active_macros;
                ExpandMacro(token, expanded, active_macros);
                ++index;
                continue;
            }

            expanded.push_back(token);
            ++index;
        }

        return expanded;
    }

    void Preprocessor::CollectMacroReplacement(std::size_t index, std::span<const Token> tokens, MacroDefination& defination) {
        std::size_t current_physical_line = tokens[index].location.line;

        while (index < tokens.size()) {
            const auto& token = tokens[index];
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
            ++index;
        }
    }

    void Preprocessor::ExpandMacro(const Token& macro_token, std::vector<Token>& output,
                                   std::unordered_set<std::string>& active_macros)
    {
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

        active_macros.insert(macro_name);

        for (const auto& replaced_token : it->second.replacement_list) {
            auto new_token = replaced_token;
            new_token.location = macro_token.location;

            if (new_token.type == TokenType::kIdentifier) {
                ExpandMacro(new_token, output, active_macros);
            } else {
                output.push_back(new_token);
            }
        }

        active_macros.erase(macro_name);
    }
}
