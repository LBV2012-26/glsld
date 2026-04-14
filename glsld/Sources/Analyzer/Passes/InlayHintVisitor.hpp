#pragma once

#include <vector>
#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"

namespace glsld {
    class InlayHintVisitor final : public AstVisitor {
    public:
        InlayHintVisitor();

        void VisitCallExpression(CallExpressionNode* node) override;
        const std::vector<InlayHint>& hints() const;

    private:
        std::vector<InlayHint> hints_;
    };
}
