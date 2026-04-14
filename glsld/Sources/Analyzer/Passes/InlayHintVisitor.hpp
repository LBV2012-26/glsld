#pragma once

#include <vector>
#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"

namespace glsld {
    class InlayHintVisitor final : public AstVisitor {
    public:
        InlayHintVisitor(const Document& document);

        const std::vector<InlayHint>& hints() const;

    private:
        void VisitCallExpression(CallExpressionNode* node) override;

        std::vector<InlayHint> hints_;
    };
}
