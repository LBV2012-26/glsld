#pragma once

#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Symbol.hpp"

namespace glsld {
    class SymbolLinker : public AstVisitor {
    public:
        SymbolLinker(const DocumentSymbols& symbols, BindingMap& bindings);

    private:
        void VisitPreprocessor(PreprocessorNode* node) override;
        void VisitVariableExpression(VariableExpressionNode* node) override;

        const DocumentSymbols& symbols_;
        BindingMap&            bindings_;
    };
}
