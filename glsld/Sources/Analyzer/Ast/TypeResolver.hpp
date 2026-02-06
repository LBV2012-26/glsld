#pragma once

#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/SymbolTable.hpp"

namespace glsld {
    class TypeResolver : public AstVisitor {
    public:
        TypeResolver(const DocumentSymbols& symbols, BindingMap& bindings);

    private:
        void VisitVariableDeclaration(VariableDeclarationNode* node) override;
        void VisitVariableExpression(VariableExpressionNode* node) override;
        void VisitIndexExpression(IndexExpressionNode* node) override;
        void VisitMemberAccessExpression(MemberAccessExpressionNode* node) override;

        const DocumentSymbols& symbols_;
        BindingMap& bindings_;
    };
}
