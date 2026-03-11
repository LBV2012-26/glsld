#pragma once

#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Token.hpp"

namespace glsld {
    class LeafLocator : public AstVisitor {
    public:
        LeafLocator(SourceLocation target);
        ~LeafLocator() override = default;

        void Traverse(AstNode* node) override;
        const AstNode* const result() const;

    protected:
        bool IsPositionInNode(const AstNode* node) const;
        bool IsPositionDeeper(const AstNode* node) const;

        SourceLocation target_;
        AstNode* deepest_node_{ nullptr };
    };

    class ContextLocator final : public LeafLocator {
    public:
        using LeafLocator::LeafLocator;
        void Traverse(AstNode* node) override;

    private:
        void VisitMemberAccessExpression(MemberAccessExpressionNode* node) override;
    };

    class SignatureLocator final : public AstVisitor {
    public:
        SignatureLocator(SourceLocation cursor);

        void VisitCallExpression(CallExpressionNode* node) override;
        const CallExpressionNode* const result() const;

    private:
        SourceLocation      cursor_;
        CallExpressionNode* best_match_{ nullptr };
    };
}
