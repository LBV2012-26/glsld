#pragma once

#include <string>
#include "Analyzer/Ast.hpp"
#include "Analyzer/AstVisitor.hpp"

namespace glsld {
    class AstDumper : public AstVisitor {
    public:
        using Base = AstVisitor;
        using Base::Base;

    private:
        void VisitTranslationUnit(TranslationUnitNode* node) override;
        void VisitDeclarationGroup(DeclarationGroupNode* node) override;
        void VisitPreprocessor(PreprocessorNode* node) override;
        void VisitFunctionDeclaration(FunctionDeclarationNode* node) override;
        void VisitVariableDeclaration(VariableDeclarationNode* node) override;
        void VisitInterfaceDeclaration(InterfaceDeclarationNode* node) override;
        void VisitStructDeclaration(StructDeclarationNode* node) override;
        void VisitCompoundStatement(CompoundStatementNode* node) override;
        void VisitIfStatement(IfStatementNode* node) override;
        void VisitForStatement(ForStatementNode* node) override;
        void VisitWhileStatement(WhileStatementNode* node) override;
        void VisitDoStatement(DoStatementNode* node) override;
        void VisitSwitchStatement(SwitchStatementNode* node) override;
        void VisitCaseStatement(CaseStatementNode* node) override;
        void VisitReturnStatement(ReturnStatementNode* node) override;
        void VisitBreakStatement(BreakStatementNode* node) override;
        void VisitContinueStatement(ContinueStatementNode* node) override;
        void VisitDiscardStatement(DiscardStatementNode* node) override;
        void VisitExpressionStatement(ExpressionStatementNode* node) override;
        void VisitNullStatement(NullStatementNode* node) override;
        void VisitBinaryExpression(BinaryExpressionNode* node) override;
        void VisitUnaryExpression(UnaryExpressionNode* node) override;
        void VisitCallExpression(CallExpressionNode* node) override;
        void VisitVariableExpression(VariableExpressionNode* node) override;
        void VisitRawExpression(RawExpressionNode* node) override;
        void VisitMemberAccessExpression(MemberAccessExpressionNode* node) override;

        std::string FormatRange(AstNode* node) const;
        std::string TypeToString(TypeSpecifier& spec) const;
        void PrintIndent() const;
        void TraverseWithoutIndent(auto* node);

        int indent_level_{};
    };
}

namespace glsld {
    void AstDumper::TraverseWithoutIndent(auto* node) {
        int old_indent = indent_level_;
        indent_level_ = 0;
        Traverse(node);
        indent_level_ = old_indent;
    }
}
