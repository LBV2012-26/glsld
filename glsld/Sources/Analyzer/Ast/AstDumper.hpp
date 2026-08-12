#pragma once

#include <string>
#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Ast/AstVisitor.hpp"

namespace glsld {
    class AstDumper final : public AstVisitor {
    public:
        using Base = AstVisitor;
        using Base::Base;

    private:
        void VisitTranslationUnit(TranslationUnitNode* node) override;
        void VisitDeclarationGroup(DeclarationGroupNode* node) override;
        void VisitPreprocessor(PreprocessorNode* node) override;
        void VisitAttribute(AttributeNode* node) override;
        void VisitQualifierArgument(QualifierArgumentNode* node) override;
        void VisitLayoutQualifier(LayoutQualifierNode* node) override;
        void VisitSpirvIntrinsic(SpirvIntrinsicNode* node) override;
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
        void VisitInitializerListExpression(InitializerListExpressionNode* node) override;
        void VisitBinaryExpression(BinaryExpressionNode* node) override;
        void VisitUnaryExpression(UnaryExpressionNode* node) override;
        void VisitTernaryExpression(TernaryExpressionNode* node) override;
        void VisitCallExpression(CallExpressionNode* node) override;
        void VisitIndexExpression(IndexExpressionNode* node) override;
        void VisitVariableExpression(VariableExpressionNode* node) override;
        void VisitRawExpression(RawExpressionNode* node) override;
        void VisitMemberAccessExpression(MemberAccessExpressionNode* node) override;

        std::string FormatRange(AstNode* node) const;
        std::string TypeToString(TypeSpec& type_spec) const;
        void PrintIndent();
        void TraverseWithoutIndent(auto* node);

        int  indent_level_{};
        bool suspend_indent_{ false };
    };
}
