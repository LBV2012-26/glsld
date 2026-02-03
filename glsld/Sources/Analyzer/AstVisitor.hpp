#pragma once

#include "Analyzer/Ast.hpp"

namespace glsld {
    class AstVisitor {
    public:
        virtual ~AstVisitor() = default;

        virtual void Traverse(AstNode* node);

    protected:
        virtual void VisitTranslationUnit(TranslationUnitNode* node);
        virtual void VisitDeclarationGroup(DeclarationGroupNode* node);
        virtual void VisitPreprocessor(PreprocessorNode* node);
        virtual void VisitFunctionDeclaration(FunctionDeclarationNode* node);
        virtual void VisitVariableDeclaration(VariableDeclarationNode* node);
        virtual void VisitInterfaceDeclaration(InterfaceDeclarationNode* node);
        virtual void VisitStructDeclaration(StructDeclarationNode* node);
        virtual void VisitCompoundStatement(CompoundStatementNode* node);
        virtual void VisitIfStatement(IfStatementNode* node);
        virtual void VisitForStatement(ForStatementNode* node);
        virtual void VisitWhileStatement(WhileStatementNode* node);
        virtual void VisitDoStatement(DoStatementNode* node);
        virtual void VisitSwitchStatement(SwitchStatementNode* node);
        virtual void VisitCaseStatement(CaseStatementNode* node);
        virtual void VisitReturnStatement(ReturnStatementNode* node);
        virtual void VisitBreakStatement(BreakStatementNode* node);
        virtual void VisitContinueStatement(ContinueStatementNode* node);
        virtual void VisitDiscardStatement(DiscardStatementNode* node);
        virtual void VisitExpressionStatement(ExpressionStatementNode* node);
        virtual void VisitNullStatement(NullStatementNode* node);
        virtual void VisitBinaryExpression(BinaryExpressionNode* node);
        virtual void VisitUnaryExpression(UnaryExpressionNode* node);
        virtual void VisitCallExpression(CallExpressionNode* node);
        virtual void VisitIndexExpression(IndexExpressionNode* node);
        virtual void VisitVariableExpression(VariableExpressionNode* node);
        virtual void VisitRawExpression(RawExpressionNode* node);
        virtual void VisitMemberAccessExpression(MemberAccessExpressionNode* node);
    };
}
