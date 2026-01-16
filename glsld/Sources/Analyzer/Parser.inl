#include "Parser.hpp"

#include <algorithm>
#include <concepts>
#include <utility>

namespace glsld {
    inline const auto& Parser::tokens() const {
        return tokens_;
    }

    inline const Token& Parser::CurrentToken() const {
        return tokens_[token_index_];
    }

    inline const Token& Parser::PeekToken(std::int64_t offset) const {
        if (token_index_ + offset >= tokens_.size()) {
            return tokens_.back();
        }

        return tokens_[token_index_ + offset];
    }

    inline void Parser::ConsumeToken(std::size_t count) {
        token_index_ = std::min(token_index_ + count, tokens_.size() - 1);
    }

    inline SourceLocation Parser::GetCurrentTokenEnd() const {
        const auto& token = CurrentToken();
        return {
            .line   = token.location.line,
            .column = token.location.column + static_cast<std::size_t>(token.text.length())
        };
    }

    inline SourceLocation Parser::GetPreviousTokenEnd() const {
        const auto& token = PeekToken(-1);
        return {
            .line   = token.location.line,
            .column = token.location.column + static_cast<std::size_t>(token.text.length())
        };
    }

    template <typename Ty>
    concept IsVector = requires {
        typename Ty::value_type;
    } && std::same_as<Ty, std::vector<typename Ty::value_type>>;

    template <typename Ty>
    std::vector<std::unique_ptr<Ty>> Parser::ParseSequence(TokenType terminator, auto parse_func, bool consume_terminator) {
        std::vector<std::unique_ptr<Ty>> nodes;

        while (CurrentToken().type != TokenType::kEndOfFile && CurrentToken().type != terminator) {
            auto result = parse_func();

            if constexpr (IsVector<decltype(result)>) {
                for (auto& node : result) {
                    if (node != nullptr) {
                        nodes.push_back(std::move(node));
                    }
                }
            } else {
                if (result != nullptr) {
                    nodes.push_back(std::move(result));
                }
            }
        }

        if (consume_terminator) {
            MatchAndConsume(terminator);
        }

        return nodes;
    }

    inline Scope* Parser::current_scope() {
        return scope_stack_.top();
    }
}
