#pragma once

#include <atomic>
#include <memory>
#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Symbol.hpp"

namespace glsld {
    class AstVisitor {
    public:
        AstVisitor(int version_replica, VersionPointer version_pointer);
        virtual ~AstVisitor() = default;

        virtual void Traverse(AstNode* node);

    protected:
        void DispatchCommonVisit(AstNode* node);
        void TraverseTypeSpec(TypeSpec& type_spec);

        virtual void VisitTranslationUnit(TranslationUnitNode* node);
        virtual void VisitDeclarationGroup(DeclarationGroupNode* node);
        virtual void VisitPreprocessor(PreprocessorNode* node);
        virtual void VisitAttribute(AttributeNode* node);
        virtual void VisitQualifierArgument(QualifierArgumentNode* node);
        virtual void VisitLayoutQualifier(LayoutQualifierNode* node);
        virtual void VisitSpirvIntrinsic(SpirvIntrinsicNode* node);
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
        virtual void VisitInitializerListExpression(InitializerListExpressionNode* node);
        virtual void VisitCastExpression(CastExpressionNode* node);
        virtual void VisitBinaryExpression(BinaryExpressionNode* node);
        virtual void VisitUnaryExpression(UnaryExpressionNode* node);
        virtual void VisitTernaryExpression(TernaryExpressionNode* node);
        virtual void VisitCallExpression(CallExpressionNode* node);
        virtual void VisitIndexExpression(IndexExpressionNode* node);
        virtual void VisitVariableExpression(VariableExpressionNode* node);
        virtual void VisitRawExpression(RawExpressionNode* node);
        virtual void VisitMemberAccessExpression(MemberAccessExpressionNode* node);

        Scope*         current_scope_{ nullptr };
        int            version_replica_{};
        VersionPointer version_pointer_;
    };
}
