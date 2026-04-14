#include "stdafx.h"
#include "NodeLocator.hpp"

namespace glsld {
    LeafLocator::LeafLocator(const Document& document, const SourceLocation& target)
        : AstVisitor(0, nullptr)
        , target_{ target }
    {
        Traverse(document.ast.get());
    }

    const AstNode* const LeafLocator::result() const {
        return deepest_node_;
    }

    void LeafLocator::Traverse(AstNode * node) {
        if (node == nullptr) {
            return;
        }

        if (IsPositionInNode(node) && IsPositionDeeper(node)) {
            deepest_node_ = node;
            AstVisitor::Traverse(node);
        }

        if (node->begin.line() > target_.line() ||
            (node->begin.line() == target_.line() && node->begin.column() > target_.column()))
        {
            // No need to continue traversing siblings, as they will be after the target position.
            return;
        }
    }

    bool LeafLocator::IsPositionInNode(const AstNode* node) const {
        // [begin.line, begin.col] <= target_ <= [end.line, end.col]
        return node->begin <= target_ && target_ <= node->end;
    }

    bool LeafLocator::IsPositionDeeper(const AstNode* node) const {
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

    void ContextLocator::Traverse(AstNode* node) {
        if (node == nullptr) {
            return;
        }

        if (IsPositionInNode(node) && IsPositionDeeper(node)) {
            AstVisitor::Traverse(node);
        }

        if (deepest_node_ != nullptr || node->begin.line() > target_.line() ||
            (node->begin.line() == target_.line() && node->begin.column() > target_.column()))
        {
            return;
        }
    }

    void ContextLocator::VisitMemberAccessExpression(MemberAccessExpressionNode* node) {
        deepest_node_ = node->object.get();
        // AstVisitor::VisitMemberAccessExpression(node);
    }

    SignatureLocator::SignatureLocator(const Document& document, const SourceLocation& cursor)
        : AstVisitor(0, nullptr)
        , cursor_{ cursor }
    {
        Traverse(document.ast.get());
    }

    const CallExpressionNode* const SignatureLocator::result() const {
        return best_match_;
    }

    void SignatureLocator::VisitCallExpression(CallExpressionNode* node) {
        if (node == nullptr) {
            return;
        }

        if (cursor_ > node->callee->end && cursor_ <= node->end) {
            best_match_ = node;
        }

        AstVisitor::VisitCallExpression(node);

        if (best_match_ != nullptr || node->begin.line() > cursor_.line() ||
            (node->begin.line() == cursor_.line() && node->begin.column() > cursor_.column()))
        {
            return;
        }
    }
}
