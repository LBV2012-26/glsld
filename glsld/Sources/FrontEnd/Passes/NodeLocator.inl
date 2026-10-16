#include "NodeLocator.hpp"

namespace glsld {
    template <typename NodeType>
    LocatorHelper<NodeType>::LocatorHelper(const SourceLocation& target)
        : AstVisitor(0, nullptr)
        , target_{ target }
    {}

    template <typename NodeType>
    bool LocatorHelper<NodeType>::IsPositionInNode(const NodeType* node) const {
        // [begin.line, begin.col] <= target_ <= [end.line, end.col]
        return node->begin <= target_ && target_ <= node->end;
    }

    template <typename NodeType>
    bool LocatorHelper<NodeType>::IsPositionDeeper(const NodeType* node) const {
        if (best_match_ == nullptr) {
            return true;
        }

        if (best_match_->begin != node->begin) {
            return node->begin > best_match_->begin;
        }

        return node->end <= best_match_->end;
    }

    template <typename NodeType>
    const NodeType* const LocatorHelper<NodeType>::result() const {
        return best_match_;
    }
}
