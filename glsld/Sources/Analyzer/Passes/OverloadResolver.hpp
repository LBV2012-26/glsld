#pragma once

#include <span>
#include <string>
#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Symbol.hpp"

namespace glsld {
    class OverloadResolver : public AstVisitor {
    public:
        OverloadResolver(const DocumentSymbols& symbols, BindingMap& bindings);

    private:
        void VisitCallExpression(CallExpressionNode* node) override;
        const SymbolInfo* ResolveOverload(const SymbolList& candidates, std::span<const TypeInfo> call_arg_types);

        const DocumentSymbols& symbols_;
        BindingMap&            bindings_;
    };
}
