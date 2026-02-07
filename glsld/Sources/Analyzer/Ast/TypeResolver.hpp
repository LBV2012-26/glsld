#pragma once

#include <span>
#include <string>
#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Symbol.hpp"

namespace glsld {
    class TypeResolver : public AstVisitor {
    public:
        TypeResolver(const DocumentSymbols& symbols, BindingMap& bindings);

    private:
        void VisitFunctionDeclaration(FunctionDeclarationNode* node) override;
        void VisitVariableDeclaration(VariableDeclarationNode* node) override;
        void VisitInterfaceDeclaration(InterfaceDeclarationNode* node) override;
        void VisitStructDeclaration(StructDeclarationNode* node) override;
        void VisitCallExpression(CallExpressionNode* node) override;
        void VisitIndexExpression(IndexExpressionNode* node) override;
        void VisitVariableExpression(VariableExpressionNode* node) override;
        void VisitMemberAccessExpression(MemberAccessExpressionNode* node) override;

        void ExtractTypeInfo(TypeInfo& target, const TypeSpecifier& type_spec);
        const SymbolInfo* ResolveOverload(const SymbolList& candidates, std::span<const TypeInfo> call_arg_types);

        const DocumentSymbols& symbols_;
        BindingMap&            bindings_;
    };
}
