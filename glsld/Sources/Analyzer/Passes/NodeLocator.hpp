#pragma once

#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"

namespace glsld {
    template <typename NodeType>
    class LocatorHelper : public AstVisitor {
    public:
        LocatorHelper(const SourceLocation& target);

        bool IsPositionInNode(const NodeType* node) const;
        bool IsPositionDeeper(const NodeType* node) const;

        const NodeType* const result() const;

    protected:
        SourceLocation target_;
        NodeType*      best_match_{ nullptr };
    };

    class LeafLocator final : public LocatorHelper<AstNode> {
    public:
        LeafLocator(const Document& document, const SourceLocation& target);

    private:
        void Traverse(AstNode* node) override;
    };

    class ContextLocator final : public LocatorHelper<AstNode> {
    public:
        ContextLocator(const Document& document, const SourceLocation& target);

    private:
        void Traverse(AstNode* node) override;
        void VisitMemberAccessExpression(MemberAccessExpressionNode* node) override;
    };

    class SignatureLocator final : public LocatorHelper<CallExpressionNode> {
    public:
        SignatureLocator(const Document& document, const SourceLocation& target);

    private:
        void VisitCallExpression(CallExpressionNode* node) override;
    };
}

#include "NodeLocator.inl"
