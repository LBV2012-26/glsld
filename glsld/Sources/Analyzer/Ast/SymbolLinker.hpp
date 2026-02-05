#pragma once

#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/SymbolTable.hpp"

namespace glsld {
    class SymbolLinker : public AstVisitor {
    public:
        SymbolLinker(const DocumentSymbols& symbols);

    private:
        void VisitVariableExpression(VariableExpressionNode* node) override;

        const DocumentSymbols& symbols_;
    };
}
