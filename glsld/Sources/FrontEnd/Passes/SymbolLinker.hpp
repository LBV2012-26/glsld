#pragma once

#include <FrontEnd/Ast/Ast.hpp>
#include <FrontEnd/Ast/AstVisitor.hpp>
#include <FrontEnd/Syntax/Document.hpp>
#include <FrontEnd/Syntax/Symbol.hpp>

namespace glsld {
    class SymbolLinker final : public AstVisitor {
    public:
        SymbolLinker(Document& document, int version_replica, VersionPointer vesion_pointer);

    private:
        void BindResolvedNames(std::span<const ResolvedNamePart> names);
        void TraverseTypeSpec(TypeSpec& type_spec) override;
        void VisitPreprocessor(PreprocessorNode* node) override;
        void VisitAttribute(AttributeNode* node) override;
        void VisitNamespaceDeclaration(NamespaceDeclarationNode* node) override;
        void VisitEnumeratorDeclaration(EnumeratorDeclarationNode* node) override;
        void VisitEnumDeclaration(EnumDeclarationNode* node) override;
        void VisitVariableExpression(VariableExpressionNode* node) override;

        Document& document_;
    };
}
