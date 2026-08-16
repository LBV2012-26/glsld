#pragma once

#include <atomic>
#include <memory>
#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    class SymbolLinker final : public AstVisitor {
    public:
        SymbolLinker(Document& document, int version_replica, VersionPointer vesion_pointer);

    private:
        void VisitPreprocessor(PreprocessorNode* node) override;
        void VisitVariableExpression(VariableExpressionNode* node) override;

        SymbolReferenceView ReferenceSymbol(const SymbolReference& reference);

        Document&                            document_;
        StringHeteroHashMap<SymbolReference> function_cache_;
    };
}
