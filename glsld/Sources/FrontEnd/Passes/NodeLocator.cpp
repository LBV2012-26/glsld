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
        if (node->object != nullptr && node->object->begin <= target_ && target_ < node->object->end) {
            Traverse(node->object);
            return;
        }

        best_match_ = node->object;
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

        const bool valid_callee = node->callee->kind() == AstNodeKind::kVariableExpression;
        if (valid_callee && target_ > node->callee->end && target_ <= node->end) {
            best_match_ = node;
        }

        AstVisitor::VisitCallExpression(node);
    }
}
