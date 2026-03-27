#include "stdafx.h"
#include "Preprocessor.hpp"

#include <cmath>
#include <charconv>
#include <limits>
#include <ranges>
#include <utility>
#include "Analyzer/Syntax/Lexer.hpp"

namespace glsld {
    namespace {
        class ConditionEvaluator {
        public:
            ConditionEvaluator(std::span<const Token> tokens, const StringHeteroHashTable<std::string, MacroDefination>& macros)
                : tokens_{ tokens }
                , macros_{ macros }
            {}

            bool Evaluate() {
                if (tokens_.empty()) {
                    return false;
                }

                auto value = ParseConditional();
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

            std::int64_t ParseNumberLiteral(std::string_view text) {
                auto IsSuffix = [](char ch) -> bool {
                    return ch == 'u' || ch == 'U' || ch == 'l' || ch == 'L' || ch == 'f' || ch == 'F';
                };

                auto end = text.size();
                while (end > 0 && IsSuffix(text[end - 1])) {
                    --end;
                }

                auto core = text.substr(0, end);
                if (core.empty()) {
                    return 0;
                }

                bool maybe_float =
                    core.find('.') != std::string_view::npos ||
                    core.find('e') != std::string_view::npos ||
                    core.find('E') != std::string_view::npos ||
                    core.find('p') != std::string_view::npos ||
                    core.find('P') != std::string_view::npos;

                if (maybe_float) {
                    double float_value = 0.0;
                    auto [ptr, ec] = std::from_chars(core.data(), core.data() + core.size(), float_value);
                    if (ec == std::errc{} && ptr == core.data() + core.size() && std::isfinite(float_value)) {
                        if (float_value >= static_cast<double>(std::numeric_limits<std::int64_t>::max()))
                            return std::numeric_limits<std::int64_t>::max();
                        if (float_value <= static_cast<double>(std::numeric_limits<std::int64_t>::min()))
                            return std::numeric_limits<std::int64_t>::min();
                        return static_cast<std::int64_t>(float_value);
                    } else {
                        return 0;
                    }
                }

                bool negative = false;
                if (!core.empty() && (core.front() == '+' || core.front() == '-')) {
                    // from_chars doesn't support leading +/- in integer
                    negative = (core.front() == '-');
                    core.remove_prefix(1);
                }

                if (core.empty()) {
                    return 0;
                }

                int base = 10;
                if (core.size() >= 2 && core[0] == '0' && (core[1] == 'x' || core[1] == 'X')) {
                    base = 16;
                    core.remove_prefix(2);
                } else if (core.size() > 1 && core[0] == '0') {
                    base = 8;
                    // 八进制保留前导 0，from_chars(base=8) 可正常处理
                }

                if (core.empty()) {
                    return 0;
                }

                std::uint64_t magnitude = 0;
                auto [ptr, ec] = std::from_chars(core.data(), core.data() + core.size(), magnitude, base);
                if (ec != std::errc{} || ptr != core.data() + core.size()) {
                    return 0;
                }

                if (!negative) {
                    if (magnitude > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
                        return std::numeric_limits<std::int64_t>::max();
                    };

                    return static_cast<std::int64_t>(magnitude);
                }

                constexpr std::uint64_t kMinAbsolute = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) + 1ull;
                if (magnitude > kMinAbsolute) {
                    return std::numeric_limits<std::int64_t>::min();
                }

                return -static_cast<std::int64_t>(magnitude);
            }

            std::int64_t ParseConditional() {
                auto condition = ParseLogicalOr();
                if (!MatchAndConsume(TokenType::kQuestion)) {
                    return condition;
                }

                if (static_cast<bool>(condition)) {
                    auto true_value = ParseConditional();
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
                    auto false_value = ParseConditional();
                    return IsSuppressed() ? 0 : false_value;
                }
            }

            std::int64_t ParsePrimary() {
                if (current_token().type == TokenType::kEndOfFile) {
                    return 0;
                }

                if (MatchAndConsume("defined")) {
                    bool has_paren = MatchAndConsume(TokenType::kOpenParen);

                    std::string name;
                    if (current_token().type == TokenType::kIdentifier) {
                        name = current_token().text;
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
                    auto value = ParseConditional();
                    MatchAndConsume(TokenType::kCloseParen);
                    return IsSuppressed() ? 0 : value;
                }

                const auto& token = current_token();
                if (token.type == TokenType::kNumberLiteral) {
                    ConsumeToken();
                    if (IsSuppressed()) {
                        return 0;
                    }

                    return ParseNumberLiteral(token.text);
                }

                if (token.type == TokenType::kIdentifier      ||
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
                    auto value = ParseUnary();
                    return IsSuppressed() ? 0 : +value;
                }

                if (MatchAndConsume(TokenType::kMinus)) {
                    auto value = ParseUnary();
                    return IsSuppressed() ? 0 : -value;
                }

                if (MatchAndConsume(TokenType::kExclamation)) {
                    auto value = ParseUnary();
                    return IsSuppressed() ? 0 : !value;
                }

                if (MatchAndConsume(TokenType::kTilde)) {
                    auto value = ParseUnary();
                    return IsSuppressed() ? 0 : ~value;
                }

                return ParsePrimary();
            }

            std::int64_t ParseMultiplication() {
                auto lhs = ParseUnary();

                while (true) {
                    if (MatchAndConsume(TokenType::kStar)) {
                        auto rhs = ParseUnary();
                        if (!IsSuppressed()) {
                            lhs = lhs * rhs;
                        }
                    } else if (MatchAndConsume(TokenType::kSlash)) {
                        auto rhs = ParseUnary();
                        if (!IsSuppressed()) {
                            lhs = rhs != 0 ? lhs / rhs : 0;
                        }
                    } else if (MatchAndConsume(TokenType::kPercent)) {
                        auto rhs = ParseUnary();
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
                        auto rhs = ParseMultiplication();
                        if (!IsSuppressed()) {
                            lhs = lhs + rhs;
                        }
                    } else if (MatchAndConsume(TokenType::kMinus)) {
                        auto rhs = ParseMultiplication();
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
                        auto rhs = ParseAddition();
                        if (!IsSuppressed()) {
                            lhs = SafeLeftShift(lhs, rhs);
                        }
                    } else if (MatchAndConsume(TokenType::kRightShift)) {
                        auto rhs = ParseAddition();
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
                        auto rhs = ParseShift();
                        if (!IsSuppressed()) {
                            lhs = (lhs < rhs) ? 1 : 0;
                        }
                    } else if (MatchAndConsume(TokenType::kLessEqual)) {
                        auto rhs = ParseShift();
                        if (!IsSuppressed()) {
                            lhs = (lhs <= rhs) ? 1 : 0;
                        }
                    } else if (MatchAndConsume(TokenType::kGreaterThan)) {
                        auto rhs = ParseShift();
                        if (!IsSuppressed()) {
                            lhs = (lhs > rhs) ? 1 : 0;
                        }
                    } else if (MatchAndConsume(TokenType::kGreaterEqual)) {
                        auto rhs = ParseShift();
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
                        auto rhs = ParseRelational();
                        if (!IsSuppressed()) {
                            lhs = (lhs == rhs) ? 1 : 0;
                        }
                    } else if (MatchAndConsume(TokenType::kNotEqual)) {
                        auto rhs = ParseRelational();
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
                    auto rhs = ParseEquality();
                    if (!IsSuppressed()) {
                        lhs = lhs & rhs;
                    }
                }

                return IsSuppressed() ? 0 : lhs;
            }

            std::int64_t ParseBitwiseXor() {
                auto lhs = ParseBitwiseAnd();

                while (MatchAndConsume(TokenType::kCaret)) {
                    auto rhs = ParseBitwiseAnd();
                    if (!IsSuppressed()) {
                        lhs = lhs ^ rhs;
                    }
                }

                return IsSuppressed() ? 0 : lhs;
            }

            std::int64_t ParseBitwiseOr() {
                auto lhs = ParseBitwiseXor();

                while (MatchAndConsume(TokenType::kVerticalBar)) {
                    auto rhs = ParseBitwiseXor();
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
                        auto rhs = ParseBitwiseOr();
                        lhs = IsSuppressed() ? 0 : static_cast<bool>(rhs);
                    }
                }

                return IsSuppressed() ? 0 : lhs;
            }

            std::int64_t ParseLogicalXor() {
                auto lhs = ParseLogicalAnd();

                while (MatchAndConsume(TokenType::kCaretCaret)) {
                    auto rhs = ParseLogicalAnd();

                    if (!IsSuppressed()) {
                        auto lhs_bool = static_cast<bool>(lhs);
                        auto rhs_bool = static_cast<bool>(rhs);
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
                        auto rhs = ParseLogicalXor();
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
                    return Token{
                        .text     = {},
                        .location = {},
                        .type     = TokenType::kEndOfFile
                    };
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

            std::span<const Token>                                     tokens_;
            const StringHeteroHashTable<std::string, MacroDefination>& macros_;
            std::size_t                                                token_index_{};
            int                                                        suppress_depth_{};
        };
    }

    Preprocessor::Preprocessor(MacroTraceMap& trace_map, MacroArgsTraceMap& args_trace_map,
                               std::vector<InactiveRegion>& inactive_regions, std::span<const Token> raw_tokens)
        : trace_map_{ trace_map }
        , args_trace_map_{ args_trace_map }
        , inactive_regions_{ inactive_regions }
        , raw_tokens_{ raw_tokens }
    {}

    std::vector<Token> Preprocessor::Process() {
        std::vector<Token> expanded;

        while (current_token().type != TokenType::kEndOfFile) {
            if (current_token().type == TokenType::kSharp) {
                HandleDirectiveAtSharp(expanded);
                continue;
            }

            if (!IsCurrentBranchActive()) {
                ConsumeToken();
                continue;
            }

            if (macros_.contains(current_token().text)) {
                std::unordered_set<std::string> active_macros;
                if (ExpandMacro(active_macros, expanded)) {
                    continue;
                }
            }

            expanded.push_back(current_token());
            ConsumeToken();
        }

        expanded.push_back(current_token()); // push_back EOF token
        return expanded;
    }

    MacroDefination Preprocessor::CollectMacroReplacement(std::size_t current_physical_line) {
        // current token is macro name
        const auto& name_token  = current_token();
        const auto& next_token = PeekToken(1);

        MacroDefination defination;
        defination.original_token = name_token;
        ConsumeToken();

        if (next_token.type == TokenType::kOpenParen &&
            name_token.location.column + name_token.text.length() == next_token.location.column)
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

        defination.replacement_list = CaptureDirectiveBodyTokens(current_physical_line);
        return defination;
    }

    std::vector<Token> Preprocessor::CaptureDirectiveBodyTokens(std::size_t directive_physical_line) {
        std::vector<Token> body;
        while (current_token().type != TokenType::kEndOfFile) {
            const auto& token = current_token();
            if (token.location.line > directive_physical_line) {
                if (!body.empty() && body.back().type == TokenType::kBackslash) {
                    body.pop_back();
                    directive_physical_line = token.location.line;
                } else {
                    break;
                }
            }

            body.push_back(token);
            ConsumeToken();
        }

        return body;
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

    bool Preprocessor::HandleDirectiveAtSharp(std::vector<Token>& output) {
        // current token is #
        const auto& sharp_token = current_token();
        ConsumeToken();

        if (current_token().type == TokenType::kEndOfFile) {
            output.push_back(sharp_token);
            return true;
        }

        const auto& directive_token = current_token();
        const auto& directive_text  = directive_token.text;
        const auto  directive_line  = directive_token.location.line;
        ConsumeToken();

        auto body = CaptureDirectiveBodyTokens(directive_line);

        if (directive_text == "if"   || directive_text == "ifdef" || directive_text == "ifndef" ||
            directive_text == "elif" || directive_text == "else"  || directive_text == "endif")
        {
            HandleConditionalDirective(directive_text, body, sharp_token.location.line);
            output.push_back(sharp_token);
            output.push_back(directive_token);
            output.append_range(body | std::views::as_rvalue);
            return true;
        }

        if (!IsCurrentBranchActive()) {
            return true;
        }

        if (directive_text == "define") {
            if (!body.empty() && body.front().type == TokenType::kIdentifier) {
                ParseDefineFromBody(body);
            }
        } else if (directive_text == "undef") {
            if (!body.empty() && body.front().type == TokenType::kIdentifier) {
                macros_.erase(body.front().text);
            }
        }

        output.push_back(sharp_token);
        output.push_back(directive_token);
        output.append_range(body | std::views::as_rvalue);
        return true;
    }

    void Preprocessor::ParseDefineFromBody(std::span<const Token> body_tokens) {
        MacroDefination defination;
        auto index = 0uz;

        defination.original_token = body_tokens[index++];

        auto IsAdjacent = [](const Token& lhs, const Token& rhs) -> bool {
            return lhs.location.line  == rhs.location.line &&
                   lhs.location.column + lhs.text.length() == rhs.location.column;
        };

        if (index < body_tokens.size() &&
            body_tokens[index].type == TokenType::kOpenParen &&
            IsAdjacent(defination.original_token, body_tokens[index])) {
            defination.is_function = true;
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
                    defination.params.push_back(token);
                    ++index;
                    continue;
                }

                defination.is_function = false;
                defination.params.clear();
                index = 1; // 替换从 macro name 后开始
                break;
            }

            if (defination.is_function && !paren_closed) {
                defination.is_function = false;
                defination.params.clear();
                index = 1;
            }
        }

        if (index < body_tokens.size()) {
            defination.replacement_list.assign_range(body_tokens | std::views::drop(index));
        }

        macros_.insert_or_assign(defination.original_token.text, std::move(defination));
    }

    bool Preprocessor::HandleConditionalDirective(std::string_view directive, std::span<const Token> body_tokens, std::size_t sharp_line) {
        if (directive == "if") {
            bool parent_active = IsCurrentBranchActive();
            bool condition     = parent_active && EvaluateIfCondition(body_tokens);
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
            bool parent_active = IsCurrentBranchActive();
            bool macro_defined = false;
            if (!body_tokens.empty() && body_tokens.front().type == TokenType::kIdentifier) {
                macro_defined = macros_.contains(body_tokens.front().text);
            } // #ifdef 和 #ifndef 不看宏本身的数值，只要被定义就算

            bool condition = (directive == "ifdef") ? macro_defined : !macro_defined;
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

            bool condition = EvaluateIfCondition(body_tokens);
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

        std::unordered_set<std::string> active_macros;
        auto expanded = ExpandIfExpression(expr_tokens, active_macros);

        if (expanded.empty()) {
            return false;
        }

        ConditionEvaluator evaluator(expanded, macros_);
        return evaluator.Evaluate();
    }

    std::vector<Token> Preprocessor::ExpandIfExpression(std::span<const Token> input, std::unordered_set<std::string>& active_macros) {
        std::vector<Token> normalized;
        normalized.reserve(input.size());

        auto MakeNumber = [](std::string text, SourceLocation location) -> Token {
            return Token{
                .text     = std::move(text),
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
                auto location  = token.location;

                if (j < input.size() && input[j].type == TokenType::kOpenParen) {
                    has_paren = true;
                    ++j;
                }

                if (j < input.size() && input[j].type == TokenType::kIdentifier) {
                    exists = macros_.contains(input[j].text);
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

        auto expanded = ExpandTokenSequence(normalized, active_macros, normalized.empty() ? SourceLocation{} : normalized.front().location);

        std::vector<Token> final_tokens;
        final_tokens.reserve(expanded.size());

        for (auto token : expanded) {
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
}
