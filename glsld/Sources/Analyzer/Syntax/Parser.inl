#include "Parser.hpp"
#include <algorithm>

namespace glsld {
    inline const Token& Parser::current_token() const {
        return expanded_tokens_[token_index_];
    }

    inline const Token& Parser::PeekToken(std::int64_t offset) const {
        if (token_index_ + offset >= expanded_tokens_.size()) {
            return expanded_tokens_.back();
        }

        return expanded_tokens_[token_index_ + offset];
    }

    inline void Parser::ConsumeToken(std::ptrdiff_t count) {
        token_index_ = std::min(token_index_ + count, expanded_tokens_.size() - 1);
    }

    inline bool Parser::MatchAndConsume(TokenType type) {
        if (current_token().type == type) {
            ConsumeToken();
            return true;
        }

        return false;
    }

    inline SourceLocation Parser::GetCurrentTokenEnd() const {
        const auto& token = current_token();
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

    inline Scope* Parser::current_scope() {
        return scope_stack_.top();
    }

    inline std::size_t Parser::GetNextAnonymousId() {
        return anonymous_block_index_++;
    }
}
