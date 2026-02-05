#pragma once

#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Token.hpp"

namespace glsld {
    class NodeLocator : public AstVisitor {
    public:
        NodeLocator(SourceLocation target);

        virtual void Traverse(AstNode* node) override;
        const AstNode* const result() const;

    private:
        bool IsPositionInNode(AstNode* node) const;

        SourceLocation target_;
        AstNode* deepest_node_;
    };
}
