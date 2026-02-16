#pragma once

#include <string>
#include <vector>
#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Token.hpp"

namespace glsld {
    struct InlayHint {
        SourceLocation location;
        std::string label;
    };

    class InlayHintVisitor : public AstVisitor {
    public:
        void VisitCallExpression(CallExpressionNode* node) override;

        const std::vector<InlayHint>& hints() const;

    private:
        std::vector<InlayHint> hints_;
    };
}
