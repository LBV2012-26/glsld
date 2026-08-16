#include "pch.hpp"
#include "NodeLocator.hpp"

namespace glsld {
    LeafLocator::LeafLocator(const Document& document, const SourceLocation& target)
        : LocatorHelper(target)
    {
        Traverse(document.ast);
    }

    void LeafLocator::Traverse(AstNode* node) {
        if (node == nullptr) {
            return;
        }

        if (IsPositionInNode(node) && IsPositionDeeper(node)) {
            best_match_ = node;
            AstVisitor::Traverse(node);
        }
    }

    ContextLocator::ContextLocator(const Document& document, const SourceLocation& target)
        : LocatorHelper(target)
    {
        Traverse(document.ast);
    }

    void ContextLocator::Traverse(AstNode* node) {
        if (node == nullptr) {
            return;
        }

        if (IsPositionInNode(node) && IsPositionDeeper(node)) {
            AstVisitor::Traverse(node);
        }
    }

    void ContextLocator::VisitMemberAccessExpression(MemberAccessExpressionNode* node) {
        best_match_ = node->object;
        // AstVisitor::VisitMemberAccessExpression(node);
    }

    SignatureLocator::SignatureLocator(const Document& document, const SourceLocation& target)
        : LocatorHelper(target)
    {
        Traverse(document.ast);
    }

    void SignatureLocator::VisitCallExpression(CallExpressionNode* node) {
        if (node == nullptr || node->callee == nullptr) {
            return;
        }

        bool valid_callee = node->callee->kind() == AstNodeKind::kVariableExpression;
        if (valid_callee && target_ > node->callee->end && target_ <= node->end) {
            best_match_ = node;
        }

        AstVisitor::VisitCallExpression(node);
    }
}
