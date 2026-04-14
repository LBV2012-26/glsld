#pragma once

#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"

namespace glsld {
    class LeafLocator : public AstVisitor {
    public:
        LeafLocator(const Document& document, const SourceLocation& target);
        ~LeafLocator() override = default;

        const AstNode* const result() const;

    protected:
        void Traverse(AstNode* node) override;
        bool IsPositionInNode(const AstNode* node) const;
        bool IsPositionDeeper(const AstNode* node) const;

        SourceLocation target_;
        AstNode*       deepest_node_{ nullptr };
    };

    class ContextLocator final : public LeafLocator {
    public:
        using LeafLocator::LeafLocator;

    private:
        void Traverse(AstNode* node) override;
        void VisitMemberAccessExpression(MemberAccessExpressionNode* node) override;
    };

    class SignatureLocator final : public AstVisitor {
    public:
        SignatureLocator(const Document& document, const SourceLocation& cursor);

        const CallExpressionNode* const result() const;

    private:
        void VisitCallExpression(CallExpressionNode* node) override;

        SourceLocation      cursor_;
        CallExpressionNode* best_match_{ nullptr };
    };
}
