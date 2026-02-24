#include "Preprocessor.hpp"
#include <algorithm>

namespace glsld {
    inline const Token& Preprocessor::current_token() const {
        return raw_tokens_[token_index_];
    }

    inline const Token& Preprocessor::PeekToken(std::int64_t offset) const {
        if (token_index_ + offset >= raw_tokens_.size()) {
            return raw_tokens_.back();
        }

        return raw_tokens_[token_index_ + offset];
    }

    inline void Preprocessor::ConsumeToken(std::ptrdiff_t count) {
        token_index_ = std::min(token_index_ + count, raw_tokens_.size() - 1);
    }

    inline bool Preprocessor::MatchAndConsume(TokenType type) {
        if (current_token().type == type) {
            ConsumeToken();
            return true;
        }

        return false;
    }
}
