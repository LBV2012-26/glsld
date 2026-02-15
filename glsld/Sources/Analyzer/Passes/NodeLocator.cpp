#include "stdafx.h"
#include "NodeLocator.hpp"

namespace glsld {
    NodeLocator::NodeLocator(SourceLocation target)
        : target_{ target }
        , deepest_node_{ nullptr }
    {}

    void NodeLocator::Traverse(AstNode * node) {
        if (node == nullptr) {
            return;
        }

        if (IsPositionInNode(node) && IsPositionDeeper(node)) {
            deepest_node_ = node;
            AstVisitor::Traverse(node);
        }

        if (node->begin.line > target_.line || (node->begin.line == target_.line && node->begin.column > target_.column)) {
            // No need to continue traversing siblings, as they will be after the target position.
            return;
        }
    }

    const AstNode* const NodeLocator::result() const {
        return deepest_node_;
    }

    bool NodeLocator::IsPositionInNode(const AstNode* node) const {
        // [begin.line, begin.col] <= target_ <= [end.line, end.col]
        return node->begin <= target_ && target_ <= node->end;
    }

    bool NodeLocator::IsPositionDeeper(const AstNode* node) const {
        if (deepest_node_ == nullptr) {
            return true;
        }

        if (deepest_node_->begin > node->begin) {
            return false;
        } else if (deepest_node_->begin < node->begin) {
            return true;
        } else {
            // deepest_node_->begin == node->begin
            if (deepest_node_->end < node->end) {
                return false;
            } else if (deepest_node_->end > node->end) {
                return true;
            } else {
                // deepest_node_->end == node->end
                return true;
            }
        }

        return true;
    }
}
