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

        if (IsPositionInNode(node)) {
            deepest_node_ = node;
            AstVisitor::Traverse(node);
        }
    }

    const AstNode* const NodeLocator::result() const {
        return deepest_node_;
    }

    bool NodeLocator::IsPositionInNode(const AstNode* node) const {
        // [begin.line, begin.col] <= target_ < [end.line, end.col]
        if (target_.line <  node->begin.line || target_.line   > node->end.line)
            return false;
        if (target_.line == node->begin.line && target_.column < node->begin.column)
            return false;
        if (target_.line == node->end.line   && target_.column > node->end.column)
            return false;
        return true;
    }
}
