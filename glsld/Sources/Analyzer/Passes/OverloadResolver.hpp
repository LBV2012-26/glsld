#pragma once

#include <atomic>
#include <memory>
#include <span>
#include <string>

#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Symbol.hpp"

namespace glsld {
    class OverloadResolver : public AstVisitor {
    public:
        OverloadResolver(const DocumentSymbols& symbols, BindingMap& bindings, int version_replica,
                         std::shared_ptr<const std::atomic<int>> version_pointer);

    private:
        void VisitCallExpression(CallExpressionNode* node) override;
        SymbolReference ResolveOverload(const SymbolList& candidates, std::span<const TypeInfo> call_arg_types);

        const DocumentSymbols& symbols_;
        BindingMap&            bindings_;
    };
}
