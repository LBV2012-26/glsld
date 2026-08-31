#pragma once

#include <vector>
#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"

namespace glsld {
    class InlayHintCollector final : public AstVisitor {
    public:
        InlayHintCollector(const Document& document);

        const std::vector<InlayHint>& hints() const;

    private:
        void VisitFunctionDeclaration(FunctionDeclarationNode* node) override;
        void VisitInitializerListExpression(InitializerListExpressionNode* node) override;
        void VisitCallExpression(CallExpressionNode* node) override;

        void CollectMacroArgumentHints(const Document& document);

        std::vector<InlayHint> hints_;
    };
}
