#pragma once

#include <atomic>
#include <memory>
#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    class SymbolLinker final : public AstVisitor {
    public:
        SymbolLinker(Document& document, int version_replica, std::shared_ptr<const std::atomic<int>> vesion_pointer);

    private:
        void VisitPreprocessor(PreprocessorNode* node) override;
        void VisitVariableExpression(VariableExpressionNode* node) override;

        Document&                            document_;
        StringHeteroHashMap<SymbolReference> function_cache_;
    };
}
