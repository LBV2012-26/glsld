#pragma once

#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/SymbolTable.hpp"

namespace glsld {
    class TypeResolver : public AstVisitor {
    public:
        TypeResolver(const DocumentSymbols& symbols);

    private:
        void VisitVariableDeclaration(VariableDeclarationNode* node) override;
        void VisitVariableExpression(VariableExpressionNode* node) override;
        void VisitMemberAccessExpression(MemberAccessExpressionNode* node) override;

        const DocumentSymbols& symbols_;
    };
}
