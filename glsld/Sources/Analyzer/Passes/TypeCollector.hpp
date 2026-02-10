#pragma once

#include <span>
#include <string>
#include <string_view>
#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Symbol.hpp"

namespace glsld {
    class TypeCollector : public AstVisitor {
    public:
        TypeCollector(const DocumentSymbols& symbols, BindingMap& bindings);

    private:
        void VisitFunctionDeclaration(FunctionDeclarationNode* node) override;
        void VisitVariableDeclaration(VariableDeclarationNode* node) override;
        void VisitInterfaceDeclaration(InterfaceDeclarationNode* node) override;
        void VisitStructDeclaration(StructDeclarationNode* node) override;
        void VisitIndexExpression(IndexExpressionNode* node) override;
        void VisitVariableExpression(VariableExpressionNode* node) override;
        void VisitRawExpression(RawExpressionNode* node) override;
        void VisitMemberAccessExpression(MemberAccessExpressionNode* node) override;

        TypeInfo ExtractTypeInfo(const TypeSpecifier& type_spec);
        TypeDescriptor ParseTypeDescriptor(std::string_view text);
        TypeInfo SniffLiteralType(const Token& token);

        const DocumentSymbols& symbols_;
        BindingMap&            bindings_;
    };
}
