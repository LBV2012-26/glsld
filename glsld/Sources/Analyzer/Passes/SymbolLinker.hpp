#pragma once

#include <Analyzer/Ast/Ast.hpp>
#include <Analyzer/Ast/AstVisitor.hpp>
#include <Analyzer/Syntax/Document.hpp>
#include <Analyzer/Syntax/Symbol.hpp>

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
        void VisitVariableExpression(VariableExpressionNode* node) override;

        Document& document_;
    };
}
