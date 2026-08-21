#include "pch.hpp"
#include "Preprocessor.hpp"

#include <cmath>
#include <algorithm>
#include <ranges>
#include <utility>

#include "Analyzer/Syntax/Lexer.hpp"
#include "Base/Hash.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    namespace {
        class ConditionEvaluator {
        public:
            ConditionEvaluator(std::span<const Token> tokens, const StringHeteroHashMap<MacroDefinition>& macros)
                : tokens_{ tokens }
                , macros_{ macros }
            {}

            bool Evaluate() {
                if (tokens_.empty()) {
                    return false;
                }

                const auto value = ParseConditional();
                return value != 0;
            }

        private:
            class SuppressGuard {
            public:
                explicit SuppressGuard(ConditionEvaluator& evaluator)
                    : evaluator_{ evaluator }
                {
                    ++evaluator_.suppress_depth_;
                }

                ~SuppressGuard() {
                    --evaluator_.suppress_depth_;
                }

            private:
                ConditionEvaluator& evaluator_;
            };

            bool IsSuppressed() const {
                return suppress_depth_ > 0;
            }

            std::int64_t ParseConditional() {
                const auto condition = ParseLogicalOr();
                if (!MatchAndConsume(TokenType::kQuestion)) {
                    return condition;
                }

                if (static_cast<bool>(condition)) {
                    const auto true_value = ParseConditional();
                    MatchAndConsume(TokenType::kColon);

                    {
                        SuppressGuard guard(*this);
                        ParseConditional(); // false branch
                    }

                    return IsSuppressed() ? 0 : true_value;
                } else {
                    {
                        SuppressGuard guard(*this);
                        ParseConditional(); // true branch
                    }

                    MatchAndConsume(TokenType::kColon);
                    const auto false_value = ParseConditional();
                    return IsSuppressed() ? 0 : false_value;
                }
            }

            std::int64_t ParsePrimary() {
                if (current_token().type == TokenType::kEndOfFile) {
                    return 0;
                }

                if (MatchAndConsume("defined")) {
                    const bool has_paren = MatchAndConsume(TokenType::kOpenParen);

                    std::string name;
                    const auto& token = current_token();
                    if (token.type == TokenType::kIdentifier) {
                        name = token.text;
                        ConsumeToken();
                    }

                    if (has_paren) {
                        MatchAndConsume(TokenType::kCloseParen);
                    }

                    if (IsSuppressed()) {
                        return 0;
                    }

                    return macros_.contains(name) ? 1 : 0;
                }

                if (MatchAndConsume(TokenType::kOpenParen)) {
                    const auto value = ParseConditional();
                    MatchAndConsume(TokenType::kCloseParen);
                    return IsSuppressed() ? 0 : value;
                }

                const auto& token = current_token();
                if (token.type == TokenType::kNumberLiteral) {
                    ConsumeToken();
                    if (IsSuppressed()) {
                        return 0;
                    }

                    return Utils::ParseNumberLiteralToInteger(token.text);
                }

                if (token.type == TokenType::kSpirvIntrinsic  ||
                    token.type == TokenType::kIdentifier      ||
                    token.type == TokenType::kPrimitive       ||
                    token.type == TokenType::kKeyword         ||
                    token.type == TokenType::kBuiltInFunction ||
                    token.type == TokenType::kBuiltInType     ||
                    token.type == TokenType::kBuiltInVariable)
                {
                    ConsumeToken();
                    return 0;
                }

                ConsumeToken();
                return 0;
            }

            std::int64_t ParseUnary() {
                if (MatchAndConsume(TokenType::kPlus)) {
                    const auto value = ParseUnary();
                    return IsSuppressed() ? 0 : +value;
                }

                if (MatchAndConsume(TokenType::kMinus)) {
                    const auto value = ParseUnary();
                    return IsSuppressed() ? 0 : -value;
                }

                if (MatchAndConsume(TokenType::kExclamation)) {
                    const auto value = ParseUnary();
                    return IsSuppressed() ? 0 : !value;
                }

                if (MatchAndConsume(TokenType::kTilde)) {
                    const auto value = ParseUnary();
                    return IsSuppressed() ? 0 : ~value;
                }

                return ParsePrimary();
            }

            std::int64_t ParseMultiplication() {
                auto lhs = ParseUnary();

                while (true) {
                    if (MatchAndConsume(TokenType::kStar)) {
                        const auto rhs = ParseUnary();
                        if (!IsSuppressed()) {
                            lhs = lhs * rhs;
                        }
                    } else if (MatchAndConsume(TokenType::kSlash)) {
                        const auto rhs = ParseUnary();
                        if (!IsSuppressed()) {
                            lhs = rhs != 0 ? lhs / rhs : 0;
                        }
                    } else if (MatchAndConsume(TokenType::kPercent)) {
                        const auto rhs = ParseUnary();
                        if (!IsSuppressed()) {
                            lhs = rhs != 0 ? lhs % rhs : 0;
                        }
                    } else {
                        break;
                    }
                }

                return IsSuppressed() ? 0 : lhs;
            }

            std::int64_t ParseAddition() {
                auto lhs = ParseMultiplication();

                while (true) {
                    if (MatchAndConsume(TokenType::kPlus)) {
                        const auto rhs = ParseMultiplication();
                        if (!IsSuppressed()) {
                            lhs = lhs + rhs;
                        }
                    } else if (MatchAndConsume(TokenType::kMinus)) {
                        const auto rhs = ParseMultiplication();
                        if (!IsSuppressed()) {
                            lhs = lhs - rhs;
                        }
                    } else {
                        break;
                    }
                }

                return IsSuppressed() ? 0 : lhs;
            }

            std::int64_t ParseShift() {
                auto lhs = ParseAddition();

                while (true) {
                    if (MatchAndConsume(TokenType::kLeftShift)) {
                        const auto rhs = ParseAddition();
                        if (!IsSuppressed()) {
                            lhs = SafeLeftShift(lhs, rhs);
                        }
                    } else if (MatchAndConsume(TokenType::kRightShift)) {
                        const auto rhs = ParseAddition();
                        if (!IsSuppressed()) {
                            lhs = SafeRightShift(lhs, rhs);
                        }
                    } else {
                        break;
                    }
                }

                return IsSuppressed() ? 0 : lhs;
            }

            std::int64_t ParseRelational() {
                auto lhs = ParseShift();

                while (true) {
                    if (MatchAndConsume(TokenType::kLessThan)) {
                        const auto rhs = ParseShift();
                        if (!IsSuppressed()) {
                            lhs = (lhs < rhs) ? 1 : 0;
                        }
                    } else if (MatchAndConsume(TokenType::kLessEqual)) {
                        const auto rhs = ParseShift();
                        if (!IsSuppressed()) {
                            lhs = (lhs <= rhs) ? 1 : 0;
                        }
                    } else if (MatchAndConsume(TokenType::kGreaterThan)) {
                        const auto rhs = ParseShift();
                        if (!IsSuppressed()) {
                            lhs = (lhs > rhs) ? 1 : 0;
                        }
                    } else if (MatchAndConsume(TokenType::kGreaterEqual)) {
                        const auto rhs = ParseShift();
                        if (!IsSuppressed()) {
                            lhs = (lhs >= rhs) ? 1 : 0;
                        }
                    } else {
                        break;
                    }
                }

                return IsSuppressed() ? 0 : lhs;
            }

            std::int64_t ParseEquality() {
                auto lhs = ParseRelational();

                while (true) {
                    if (MatchAndConsume(TokenType::kEqualEqual)) {
                        const auto rhs = ParseRelational();
                        if (!IsSuppressed()) {
                            lhs = (lhs == rhs) ? 1 : 0;
                        }
                    } else if (MatchAndConsume(TokenType::kNotEqual)) {
                        const auto rhs = ParseRelational();
                        if (!IsSuppressed()) {
                            lhs = (lhs != rhs) ? 1 : 0;
                        }
                    } else {
                        break;
                    }
                }

                return IsSuppressed() ? 0 : lhs;
            }

            std::int64_t ParseBitwiseAnd() {
                auto lhs = ParseEquality();

                while (MatchAndConsume(TokenType::kAmpersand)) {
                    const auto rhs = ParseEquality();
                    if (!IsSuppressed()) {
                        lhs = lhs & rhs;
                    }
                }

                return IsSuppressed() ? 0 : lhs;
            }

            std::int64_t ParseBitwiseXor() {
                auto lhs = ParseBitwiseAnd();

                while (MatchAndConsume(TokenType::kCaret)) {
                    const auto rhs = ParseBitwiseAnd();
                    if (!IsSuppressed()) {
                        lhs = lhs ^ rhs;
                    }
                }

                return IsSuppressed() ? 0 : lhs;
            }

            std::int64_t ParseBitwiseOr() {
                auto lhs = ParseBitwiseXor();

                while (MatchAndConsume(TokenType::kVerticalBar)) {
                    const auto rhs = ParseBitwiseXor();
                    if (!IsSuppressed()) {
                        lhs = lhs | rhs;
                    }
                }

                return IsSuppressed() ? 0 : lhs;
            }

            std::int64_t ParseLogicalAnd() {
                auto lhs = ParseBitwiseOr();

                while (MatchAndConsume(TokenType::kAmpersandAmpersand)) {
                    if (!static_cast<bool>(lhs)) {
                        SuppressGuard guard(*this);
                        ParseBitwiseOr();
                        lhs = 0;
                    } else {
                        const auto rhs = ParseBitwiseOr();
                        lhs = IsSuppressed() ? 0 : static_cast<bool>(rhs);
                    }
                }

                return IsSuppressed() ? 0 : lhs;
            }

            std::int64_t ParseLogicalXor() {
                auto lhs = ParseLogicalAnd();

                while (MatchAndConsume(TokenType::kCaretCaret)) {
                    const auto rhs = ParseLogicalAnd();

                    if (!IsSuppressed()) {
                        const auto lhs_bool = static_cast<bool>(lhs);
                        const auto rhs_bool = static_cast<bool>(rhs);
                        lhs = (lhs_bool != rhs_bool) ? 1 : 0;
                    }
                }

                return IsSuppressed() ? 0 : lhs;
            }

            std::int64_t ParseLogicalOr() {
                auto lhs = ParseLogicalXor();

                while (MatchAndConsume(TokenType::kVerticalBarVerticalBar)) {
                    if (static_cast<bool>(lhs)) {
                        SuppressGuard guard(*this);
                        ParseLogicalXor();
                        lhs = 1;
                    } else {
                        const auto rhs = ParseLogicalXor();
                        lhs = IsSuppressed() ? 0 : static_cast<bool>(rhs);
                    }
                }

                return IsSuppressed() ? 0 : lhs;
            }

            std::int64_t SafeLeftShift(std::int64_t lhs, std::int64_t rhs) {
                if (rhs < 0 || rhs >= 63) {
                    return 0;
                }

                return lhs << rhs;
            }

            std::int64_t SafeRightShift(std::int64_t lhs, std::int64_t rhs) {
                if (rhs < 0 || rhs >= 63) {
                    return 0;
                }

                return lhs >> rhs;
            }

            const Token& current_token() const {
                if (token_index_ >= tokens_.size()) {
                    static const Token kEofToken{
                        .text     = {},
                        .location = {},
                        .type     = TokenType::kEndOfFile
                    };

                    return kEofToken;
                }

                return tokens_[token_index_];
            }

            void ConsumeToken(std::ptrdiff_t count = 1) {
                if (tokens_.empty()) {
                    return;
                }

                token_index_ = std::min(token_index_ + count, tokens_.size());
            }

            bool MatchAndConsume(TokenType type) {
                if (token_index_ < tokens_.size() && tokens_[token_index_].type == type) {
                    ++token_index_;
                    return true;
                }

                return false;
            }

            bool MatchAndConsume(std::string_view text) {
                if (token_index_ < tokens_.size() && tokens_[token_index_].text == text) {
                    ++token_index_;
                    return true;
                }

                return false;
            }

            std::span<const Token>                      tokens_;
            const StringHeteroHashMap<MacroDefinition>& macros_;
            std::size_t                                 token_index_{};
            int                                         suppress_depth_{};
        };
    }

    Preprocessor::Preprocessor(Document& document,
                               SourceTable& source_table,
                               const SourceFile* source_file,
                               IncludeLoader& include_loader,
                               IncludeDirectoryHandle include_dirs,
                               std::span<const Token> raw_tokens,
                               std::vector<std::string> parent_stack)

        : source_table_{ source_table }
        , source_file_{ source_file }
        , include_loader_{ include_loader }
        , include_dirs_{ include_dirs }
        , include_stack_{ std::move(parent_stack) }
        , raw_tokens_{ raw_tokens }
        , document_{ document }
    {
        if (include_stack_.empty()) {
            include_stack_.push_back(std::string(source_file_->filename()));
        }
    }

    std::vector<Token> Preprocessor::Process() {
        std::vector<Token> expanded;
        expanded.reserve(static_cast<std::size_t>(raw_tokens_.size() * 1.3));

        while (current_token().type != TokenType::kEndOfFile) {
            if (current_token().type == TokenType::kSharp) {
                HandleDirectiveAtSharp(expanded);
                continue;
            }

            if (!IsCurrentBranchActive()) {
                ConsumeToken();
                continue;
            }

            if (document_.macro_table.contains(current_token().text)) {
                StringHeteroHashSet active_macros;
                if (ExpandMacro(active_macros, expanded)) {
                    continue;
                }
            }

            expanded.push_back(current_token());
            ConsumeToken();
        }

        const auto& eof_token = current_token();
        FinalizeInactiveRegions(eof_token.location.line());
        expanded.push_back(eof_token); // push_back EOF token
        return expanded;
    }

    MacroDefinition Preprocessor::CollectMacroReplacement(std::size_t current_physical_line) {
        // current token is macro name
        const auto& name_token  = current_token();
        const auto& next_token = PeekToken(1);

        MacroDefinition definition;
        definition.original_token = name_token;
        ConsumeToken();

        if (next_token.type == TokenType::kOpenParen &&
            name_token.location.column() + name_token.text.length() == next_token.location.column())
        {
            definition.is_function = true;
            ConsumeToken();
        }

        if (definition.is_function) {
            do {
                if (current_token().type == TokenType::kCloseParen) {
                    break;
                }

                MatchAndConsume(TokenType::kComma);
                const auto& param_token = current_token();
                definition.params.push_back(param_token);
                ConsumeToken();
            } while (current_token().type == TokenType::kComma);

            MatchAndConsume(TokenType::kCloseParen);
        }

        definition.replacement_list = CaptureDirectiveBodyTokens(current_physical_line);
        return definition;
    }

    std::vector<Token> Preprocessor::CaptureDirectiveBodyTokens(std::size_t directive_physical_line) {
        std::vector<Token> body;
        while (current_token().type != TokenType::kEndOfFile) {
            const auto& token = current_token();
            if (token.location.line() > directive_physical_line) {
                if (!body.empty() && body.back().type == TokenType::kBackslash) {
                    if (token.location.line() - body.back().location.line() > 1) {
                        body.pop_back(); // remove backslash
                        break;
                    }

                    body.pop_back();
                    directive_physical_line = token.location.line();
                } else {
                    break;
                }
            }

            body.push_back(token);
            ConsumeToken();
        }

        return body;
    }

    bool Preprocessor::ExpandMacro(StringHeteroHashSet& active_macros, std::vector<Token>& output) {
        // current token is macro name
        const auto& macro_token = current_token();

        auto it = document_.macro_table.find(macro_token.text);
        if (it == document_.macro_table.end()) {
            output.push_back(macro_token);
            ConsumeToken();
            return true;
        }

        if (active_macros.contains(macro_token.text)) {
            return false;
        }

        document_.macro_traces.try_emplace(macro_token.location, it->second.original_token);

        active_macros.insert(macro_token.text);
        const auto& definition = it->second;

        if (!definition.is_function) {
            ConsumeToken(); // consume macro name token
            auto replaced = ExpandTokenSequence(definition.replacement_list, active_macros, macro_token.location);
            document_.macro_expansions.try_emplace(macro_token.location, replaced);
            output.append_range(replaced | std::views::as_rvalue);
            active_macros.erase(macro_token.text);
            return true;
        }

        if (PeekToken().type != TokenType::kOpenParen) {
            active_macros.erase(macro_token.text);
            return false;
        }

        std::vector<std::vector<Token>> arguments;
        if (!ParseFunctionMacroInvocationFromStream(arguments)) {
            active_macros.erase(macro_token.text);
            return false;
        }

        auto replaced = SubstituteFunctionMacro(definition, arguments, active_macros, macro_token.location);
        document_.macro_expansions.try_emplace(macro_token.location, replaced);
        output.append_range(replaced | std::views::as_rvalue);
        active_macros.erase(macro_token.text);
        return true;
    }

    std::vector<Token> Preprocessor::ExpandTokenSequence(
        std::span<const Token> input,
        StringHeteroHashSet& active_macros,
        const SourceLocation& call_site)
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

            auto it = document_.macro_table.find(token.text);
            if (it == document_.macro_table.end() || active_macros.contains(token.text)) {
                PushAtCallSite(token);
                continue;
            }

            const auto& definition = it->second;
            const auto& macro_name = token.text;
            active_macros.insert(macro_name);

            if (!definition.is_function) {
                auto nested = ExpandTokenSequence(definition.replacement_list, active_macros, call_site);
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

            auto replaced = SubstituteFunctionMacro(definition, arguments, active_macros, call_site);
            AppendRangeAtCallSite(replaced);
            i = close_paren_index;
            active_macros.erase(macro_name);
        }

        return result;
    }

    std::vector<Token> Preprocessor::SubstituteFunctionMacro(
        const MacroDefinition& definition,
        const std::vector<std::vector<Token>>& arguments,
        StringHeteroHashSet& active_macros,
        const SourceLocation& call_site)
    {
        StringHeteroHashMap<std::size_t> param_index;
        for (auto i = 0uz; i != definition.params.size(); ++i) {
            param_index.try_emplace(definition.params[i].text, i);
        }

        auto arguments_active_macros = active_macros;
        arguments_active_macros.erase(definition.original_token.text);

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
            const auto& replacement_list = definition.replacement_list;
            if (replace_index > 0 && replacement_list[replace_index - 1].type == TokenType::kSharpSharp) {
                return true;
            }

            if (replace_index + 1 < replacement_list.size() &&
                replacement_list[replace_index + 1].type == TokenType::kSharpSharp)
            {
                return true;
            }

            return false;
        };

        std::vector<Token> replaced;
        for (auto i = 0uz; i != definition.replacement_list.size(); ++i) {
            const auto& token = definition.replacement_list[i];
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
            replaced.push_back(std::move(copied));
        }

        const auto pasted = ApplyTokenPasting(replaced);
        auto rescanned = ExpandTokenSequence(pasted, active_macros, call_site);
        for (auto& token : rescanned) {
            token.location = call_site;
        }

        return rescanned;
    }

    bool Preprocessor::ParseFunctionMacroInvocationFromStream(std::vector<std::vector<Token>>& arguments) {
        auto open_paren_index  = token_index_ + 1;
        auto close_paren_index = 0uz;

        if (!ParseFunctionMacroInvocationInSequence(raw_tokens_, open_paren_index, close_paren_index, arguments)) {
            return false;
        }

        token_index_ = close_paren_index + 1;
        return true;
    }

    bool Preprocessor::ParseFunctionMacroInvocationInSequence(
        std::span<const Token> input,
        std::size_t open_paren_index,
        std::size_t& close_paren_index,
        std::vector<std::vector<Token>>& arguments)
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

            MacroArgumentTrace trace{ token };

            auto macro_it = document_.macro_table.find(token.text);
            if (macro_it != document_.macro_table.end()) {
                trace.definition = macro_it->second.original_token;
            }

            document_.macro_args_traces.try_emplace(token.location, std::move(trace));
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

            auto left = std::move(result.back());
            result.pop_back();
            const auto& right = tokens[++i];
            auto pasted = PasteTokens(left, right);
            result.push_back(std::move(pasted));
        }

        return result;
    }

    Token Preprocessor::PasteTokens(const Token& left, const Token& right) {
        std::string new_text;
        new_text.reserve(left.text.size() + right.text.size());
        new_text.append(left.text);
        new_text.append(right.text);

        Lexer lexer(source_file_, new_text, include_loader_, include_dirs_);
        Token token = lexer.AcquireNextToken();

        token.text     = document_.StoreTokenText(token.text);
        token.location = left.location;

        return token;
    }

    void Preprocessor::HandleDirectiveAtSharp(std::vector<Token>& output) {
        // current token is #
        const auto& sharp_token = current_token();
        ConsumeToken();

        if (current_token().type == TokenType::kEndOfFile) {
            output.push_back(sharp_token);
            return;
        }

        const auto& directive_token = current_token();
        const auto& directive_text  = directive_token.text;
        const auto  directive_line  = directive_token.location.line();
        ConsumeToken();

        auto ProcessLineSplicing = [this](auto& body) -> void {
            for (auto i = 0uz; i < body.size(); ++i) {
                if (i + 1 < body.size() && body[i].location.line() < body[i + 1].location.line()) {
                    body.insert(body.begin() + i + 1, Token{
                        .text     = "\\",
                        .location = SourceLocation(
                            source_file_,
                            body[i].location.line(),
                            body[i].location.column() + static_cast<std::uint32_t>(body[i].text.length()) + 1
                        ),
                        .type     = TokenType::kBackslash
                    });

                    ++i;
                }
            }
        };

        auto body = CaptureDirectiveBodyTokens(directive_line);

        if (directive_text == "if"   || directive_text == "ifdef" || directive_text == "ifndef" ||
            directive_text == "elif" || directive_text == "else"  || directive_text == "endif")
        {
            const bool was_active = IsCurrentBranchActive();
            const bool handled    = HandleConditionalDirective(directive_text, body, sharp_token.location.line());
            const bool now_active = IsCurrentBranchActive();

            if (handled) {
                UpdateInactiveRegions(was_active, now_active, sharp_token.location.line());
            }

            output.push_back(sharp_token);
            output.push_back(directive_token);
            ProcessLineSplicing(body);
            output.append_range(body | std::views::as_rvalue);
            return;
        }

        if (!IsCurrentBranchActive()) {
            return;
        }

        if (directive_text == "include") {
            output.push_back(sharp_token);
            output.push_back(directive_token);

            ProcessLineSplicing(body);

            auto included = ExpandIncludeDirective(body);
            output.append_range(body     | std::views::as_rvalue);
            output.append_range(included | std::views::as_rvalue);
            return;
        }

        if (directive_text == "define") {
            if (!body.empty() && body.front().type == TokenType::kIdentifier) {
                ParseDefineFromBody(body);
            }
        } else if (directive_text == "undef") {
            if (!body.empty() && body.front().type == TokenType::kIdentifier) {
                document_.macro_table.erase(body.front().text);
            }
        }

        output.push_back(sharp_token);
        output.push_back(directive_token);
        ProcessLineSplicing(body);
        output.append_range(body | std::views::as_rvalue);
    }

    void Preprocessor::ParseDefineFromBody(std::span<const Token> body_tokens) {
        MacroDefinition definition;
        auto index = 0uz;

        definition.original_token = body_tokens[index++];

        auto IsAdjacent = [](const Token& lhs, const Token& rhs) -> bool {
            return lhs.location.line()  == rhs.location.line() &&
                   lhs.location.column() + lhs.text.length() == rhs.location.column();
        };

        if (index < body_tokens.size() &&
            body_tokens[index].type == TokenType::kOpenParen &&
            IsAdjacent(definition.original_token, body_tokens[index])) {
            definition.is_function = true;
            ++index; // consume '('

            bool paren_closed = false;
            while (index < body_tokens.size()) {
                const auto& token = body_tokens[index];

                if (token.type == TokenType::kCloseParen) {
                    ++index; // consume ')'
                    paren_closed = true;
                    break;
                }

                if (token.type == TokenType::kComma) {
                    ++index;
                    continue;
                }

                if (token.type == TokenType::kIdentifier) {
                    definition.params.push_back(token);
                    ++index;
                    continue;
                }

                definition.is_function = false;
                definition.params.clear();
                index = 1; // 替换从 macro name 后开始
                break;
            }

            if (definition.is_function && !paren_closed) {
                definition.is_function = false;
                definition.params.clear();
                index = 1;
            }
        }

        if (index < body_tokens.size()) {
            definition.replacement_list.assign_range(body_tokens | std::views::drop(index));
        }

        document_.macro_table.insert_or_assign(definition.original_token.text, std::move(definition));
    }

    bool Preprocessor::HandleConditionalDirective(
        std::string_view directive,
        std::span<const Token> body_tokens,
        std::uint32_t sharp_line)
    {
        if (directive == "if") {
            const bool parent_active = IsCurrentBranchActive();
            const bool condition     = parent_active && EvaluateIfCondition(body_tokens);
            condition_stack_.push({
                .parent_active  = parent_active,
                .branch_taken   = condition,
                .current_active = parent_active && condition,
                .in_else        = false,
                .if_line        = sharp_line
            });

            return true;
        }

        if (directive == "ifdef" || directive == "ifndef") {
            const bool parent_active = IsCurrentBranchActive();
            bool       macro_defined = false;
            if (!body_tokens.empty() && body_tokens.front().type == TokenType::kIdentifier) {
                macro_defined = document_.macro_table.contains(body_tokens.front().text);
            } // #ifdef 和 #ifndef 不看宏本身的数值，只要被定义就算

            const bool condition = (directive == "ifdef") ? macro_defined : !macro_defined;
            condition_stack_.push({
                .parent_active  = parent_active,
                .branch_taken   = condition,
                .current_active = parent_active && condition,
                .in_else        = false,
                .if_line        = sharp_line
            });

            return true;
        }

        if (directive == "elif") {
            if (condition_stack_.empty()) {
                return false;
            }

            auto& frame = condition_stack_.top();
            if (frame.in_else) {
                return false;
            }

            if (!frame.parent_active || frame.branch_taken) {
                frame.current_active = false;
                return true;
            }

            const bool condition = EvaluateIfCondition(body_tokens);
            frame.branch_taken   = condition;
            frame.current_active = frame.parent_active && condition;
            return true;
        }

        if (directive == "else") {
            if (condition_stack_.empty()) {
                return false;
            }

            auto& frame = condition_stack_.top();
            if (frame.in_else) {
                return false;
            }

            frame.current_active = frame.parent_active && !frame.branch_taken;
            frame.branch_taken   = true;
            frame.in_else        = true;
            return true;
        }

        if (directive == "endif") {
            if (condition_stack_.empty()) {
                return false;
            }

            condition_stack_.pop();
            return true;
        }

        return false;
    }

    bool Preprocessor::EvaluateIfCondition(std::span<const Token> expr_tokens) {
        if (expr_tokens.empty()) {
            return false;
        }

        StringHeteroHashSet active_macros;
        const auto expanded = ExpandIfExpression(expr_tokens, active_macros);

        if (expanded.empty()) {
            return false;
        }

        ConditionEvaluator evaluator(expanded, document_.macro_table);
        return evaluator.Evaluate();
    }

    std::vector<Token> Preprocessor::ExpandIfExpression(std::span<const Token> input, StringHeteroHashSet& active_macros) {
        std::vector<Token> normalized;
        normalized.reserve(input.size());

        auto MakeNumber = [this](std::string_view text, const SourceLocation& location) -> Token {
            return Token{
                .text     = document_.StoreTokenText(text),
                .location = location,
                .type     = TokenType::kNumberLiteral
            };
        };

        auto IsIdentifierLike = [](const Token& token) -> bool {
            return token.type == TokenType::kIdentifier
                || token.type == TokenType::kPrimitive
                || token.type == TokenType::kKeyword
                || token.type == TokenType::kPreprocessor
                || token.type == TokenType::kBuiltInFunction
                || token.type == TokenType::kBuiltInVariable
                || token.type == TokenType::kBuiltInType;
        };

        for (auto i = 0uz; i != input.size();) {
            const auto& token = input[i];

            if (token.text == "defined") {
                auto j         = i + 1;
                bool has_paren = false;
                bool valid     = false;
                bool exists    = false;
                const auto& location  = token.location;

                if (j < input.size() && input[j].type == TokenType::kOpenParen) {
                    has_paren = true;
                    ++j;
                }

                if (j < input.size() && input[j].type == TokenType::kIdentifier) {
                    exists = document_.macro_table.contains(input[j].text);
                    valid  = true;
                    ++j;
                }

                if (has_paren) {
                    if (j < input.size() && input[j].type == TokenType::kCloseParen) {
                        ++j;
                    } else {
                        valid = false;
                    }
                }

                normalized.push_back(MakeNumber(valid && exists ? "1" : "0", location));
                i = (j > i ? j : i + 1);
                continue;
            }

            normalized.push_back(token);
            ++i;
        }

        auto expanded = ExpandTokenSequence(
            normalized, active_macros, normalized.empty() ? SourceLocation{} : normalized.front().location);

        std::vector<Token> final_tokens;
        final_tokens.reserve(expanded.size());

        for (auto& token : expanded) {
            if (IsIdentifierLike(token)) {
                token.text = "0";
                token.type = TokenType::kNumberLiteral;
                final_tokens.push_back(std::move(token));
                continue;
            }

            final_tokens.push_back(std::move(token));
        }

        return final_tokens;
    }

    void Preprocessor::AppendInactiveRegion(std::uint32_t begin_line, std::uint32_t end_line) {
        if (begin_line == 0 || end_line == 0 || begin_line > end_line) {
            return;
        }

        auto& inactive_region = document_.inactive_regions[source_file_];

        if (!inactive_region.empty()) {
            auto& last = inactive_region.back();
            if (begin_line <= last.end_line + 1) {
                last.end_line = std::max(last.end_line, end_line);
                return;
            }
        }

        inactive_region.push_back({
            .begin_line = begin_line,
            .end_line   = end_line
        });
    }

    void Preprocessor::UpdateInactiveRegions(bool was_active, bool now_active, std::uint32_t directive_line) {
        if (was_active == now_active) {
            return;
        }

        if (was_active && !now_active) {
            // 从激活进入失活：从该条件指令下一行开始
            open_inactive_begin_line_ = directive_line + 1;
            return;
        }

        // 从失活进入激活：到该条件指令上一行结束
        if (open_inactive_begin_line_.has_value()) {
            const auto begin_line = *open_inactive_begin_line_;
            const auto end_line   = (directive_line > 0) ? (directive_line - 1) : 0;
            AppendInactiveRegion(begin_line, end_line);
            open_inactive_begin_line_.reset();
        }
    }

    void Preprocessor::FinalizeInactiveRegions(std::uint32_t eof_line) {
        if (!open_inactive_begin_line_.has_value()) {
            return;
        }

        AppendInactiveRegion(*open_inactive_begin_line_, eof_line);
        open_inactive_begin_line_.reset();
    }

    std::vector<Token> Preprocessor::ExpandIncludeDirective(std::span<const Token> body_tokens) {
        StringHeteroHashSet active_macros;
        SourceLocation call_site;
        if (!body_tokens.empty()) {
            call_site = body_tokens.front().location;
        }

        auto expanded_body    = ExpandTokenSequence(body_tokens, active_macros, call_site);
        auto future           = include_loader_.Include(source_file_->uri(), expanded_body, include_dirs_);
        auto include_snapshot = future.get();

        if (include_snapshot == nullptr || !include_snapshot->valid() ||
            std::ranges::find(include_stack_, include_snapshot->filename) != include_stack_.end())
        {
            return {};
        }

        document_.dependencies.push_back(include_snapshot->uri);

        const auto* include_file = source_table_.Intern(include_snapshot->filename, include_snapshot->uri);
        Preprocessor subprocessor(document_, source_table_, include_file, include_loader_, include_dirs_, include_snapshot->tokens, include_stack_);
        auto expanded = subprocessor.Process();

        document_.StoreIncludeSource(std::move(include_snapshot));

        if (!expanded.empty() && expanded.back().type == TokenType::kEndOfFile) {
            expanded.pop_back();
        }

        return expanded;
    }
}
