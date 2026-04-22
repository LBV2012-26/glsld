#include "NodeLocator.hpp"

namespace glsld {
    template <typename NodeType>
    LocatorHelper<NodeType>::LocatorHelper(const SourceLocation& target)
        : AstVisitor(0, nullptr)
        , target_{ target }
    {}

    template <typename NodeType>
    inline bool LocatorHelper<NodeType>::IsPositionInNode(const NodeType* node) const {
        // [begin.line, begin.col] <= target_ <= [end.line, end.col]
        return node->begin <= target_ && target_ <= node->end;
    }

    template <typename NodeType>
    bool LocatorHelper<NodeType>::IsPositionDeeper(const NodeType* node) const {
        if (best_match_ == nullptr) {
            return true;
        }

        if (best_match_->begin > node->begin) {
            return false;
        } else if (best_match_->begin < node->begin) {
            return true;
        } else {
            // best_match_->begin == node->begin
            if (best_match_->end < node->end) {
                return false;
            } else if (best_match_->end > node->end) {
                return true;
            } else {
                // best_match_->end == node->end
                return true;
            }
        }

        return true;
    }

    template <typename NodeType>
    inline const NodeType* const LocatorHelper<NodeType>::result() const {
        return best_match_;
    }
}
