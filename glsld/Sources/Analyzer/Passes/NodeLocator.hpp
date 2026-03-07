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
        AstNode* deepest_node_;
    };

    class ContextLocator final : public LeafLocator {
    public:
        using LeafLocator::LeafLocator;
        void Traverse(AstNode* node) override;

    private:
        void VisitMemberAccessExpression(MemberAccessExpressionNode* node) override;
    };
}
