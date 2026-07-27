#include "pch.hpp"
#include "NodeLocator.hpp"

namespace glsld {
    LeafLocator::LeafLocator(const Document& document, const SourceLocation& target)
        : LocatorHelper(target)
    {
        Traverse(document.ast.get());
    }

    void LeafLocator::Traverse(AstNode* node) {
        if (node == nullptr) {
            return;
        }

        if (IsPositionInNode(node) && IsPositionDeeper(node)) {
            best_match_ = node;
            AstVisitor::Traverse(node);
        }

        if (node->begin.line() > target_.line() ||
            (node->begin.line() == target_.line() && node->begin.column() > target_.column()))
        {
            // No need to continue traversing siblings, as they will be after the target position.
            return;
        }
    }

    ContextLocator::ContextLocator(const Document& document, const SourceLocation& target)
        : LocatorHelper(target)
    {
        Traverse(document.ast.get());
    }

    void ContextLocator::Traverse(AstNode* node) {
        if (node == nullptr) {
            return;
        }

        if (IsPositionInNode(node) && IsPositionDeeper(node)) {
            AstVisitor::Traverse(node);
        }

        if (best_match_ != nullptr || node->begin.line() > target_.line() || // 这里 best_match_ != nullptr 是为了在找到一个包含目标位置的节点后，停止继续遍历其他兄弟节点，因为它们不可能包含目标位置了。
            (node->begin.line() == target_.line() && node->begin.column() > target_.column()))
        {
            return;
        }
    }

    void ContextLocator::VisitMemberAccessExpression(MemberAccessExpressionNode* node) {
        best_match_ = node->object.get();
        // AstVisitor::VisitMemberAccessExpression(node);
    }

    SignatureLocator::SignatureLocator(const Document& document, const SourceLocation& target)
        : LocatorHelper(target)
    {
        Traverse(document.ast.get());
    }

    void SignatureLocator::VisitCallExpression(CallExpressionNode* node) {
        if (node == nullptr) {
            return;
        }

        if (target_ > node->callee->end && target_ <= node->end) {
            best_match_ = node;
        }

        AstVisitor::VisitCallExpression(node);

        if (best_match_ != nullptr || node->begin.line() > target_.line() ||
            (node->begin.line() == target_.line() && node->begin.column() > target_.column()))
        {
            return;
        }
    }
}
