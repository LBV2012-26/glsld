#include "stdafx.h"
#include "AstVisitor.hpp"

namespace glsld {
    void AstVisitor::Traverse(AstNode* node) {
        if (node == nullptr) {
            return;
        }

        switch (node->kind()) {
        case AstNodeKind::kTranslationUnit:
            VisitTranslationUnit(static_cast<TranslationUnitNode*>(node));
            break;
        case AstNodeKind::kDeclarationGroup:
            VisitDeclarationGroup(static_cast<DeclarationGroupNode*>(node));
            break;
        case AstNodeKind::kPreprocessor:
            VisitPreprocessor(static_cast<PreprocessorNode*>(node));
            break;
        case AstNodeKind::kFunctionDecl:
            VisitFunctionDeclaration(static_cast<FunctionDeclarationNode*>(node));
            break;
        case AstNodeKind::kVariableDecl:
            VisitVariableDeclaration(static_cast<VariableDeclarationNode*>(node));
            break;
        case AstNodeKind::kInterfaceDecl:
            VisitInterfaceDeclaration(static_cast<InterfaceDeclarationNode*>(node));
            break;
        case AstNodeKind::kStructDecl:
            VisitStructDeclaration(static_cast<StructDeclarationNode*>(node));
            break;
        case AstNodeKind::kCompoundStmt:
            VisitCompoundStatement(static_cast<CompoundStatementNode*>(node));
            break;
        case AstNodeKind::kIfStmt:
            VisitIfStatement(static_cast<IfStatementNode*>(node));
            break;
        case AstNodeKind::kForStmt:
            VisitForStatement(static_cast<ForStatementNode*>(node));
            break;
        case AstNodeKind::kWhileStmt:
            VisitWhileStatement(static_cast<WhileStatementNode*>(node));
            break;
        case AstNodeKind::kDoStmt:
            VisitDoStatement(static_cast<DoStatementNode*>(node));
            break;
        case AstNodeKind::kSwitchStmt:
            VisitSwitchStatement(static_cast<SwitchStatementNode*>(node));
            break;
        case AstNodeKind::kCaseStmt:
            VisitCaseStatement(static_cast<CaseStatementNode*>(node));
            break;
        case AstNodeKind::kReturnStmt:
            VisitReturnStatement(static_cast<ReturnStatementNode*>(node));
            break;
        case AstNodeKind::kBreakStmt:
            VisitBreakStatement(static_cast<BreakStatementNode*>(node));
            break;
        case AstNodeKind::kContinueStmt:
            VisitContinueStatement(static_cast<ContinueStatementNode*>(node));
            break;
        case AstNodeKind::kDiscardStmt:
            VisitDiscardStatement(static_cast<DiscardStatementNode*>(node));
            break;
        case AstNodeKind::kExprStmt:
            VisitExpressionStatement(static_cast<ExpressionStatementNode*>(node));
            break;
        case AstNodeKind::kNullStmt:
            VisitNullStatement(static_cast<NullStatementNode*>(node));
            break;
        case AstNodeKind::kInitListExpr:
            VisitInitializerListExpression(static_cast<InitializerListExpressionNode*>(node));
            break;
        case AstNodeKind::kBinaryExpr:
            VisitBinaryExpression(static_cast<BinaryExpressionNode*>(node));
            break;
        case AstNodeKind::kUnaryExpr:
            VisitUnaryExpression(static_cast<UnaryExpressionNode*>(node));
            break;
        case AstNodeKind::kCallExpr:
            VisitCallExpression(static_cast<CallExpressionNode*>(node));
            break;
        case AstNodeKind::kIndexExpr:
            VisitIndexExpression(static_cast<IndexExpressionNode*>(node));
            break;
        case AstNodeKind::kVariableExpr:
            VisitVariableExpression(static_cast<VariableExpressionNode*>(node));
            break;
        case AstNodeKind::kLiteralExpr:
            VisitRawExpression(static_cast<RawExpressionNode*>(node));
            break;
        case AstNodeKind::kMemberAccessExpr:
            VisitMemberAccessExpression(static_cast<MemberAccessExpressionNode*>(node));
            break;
        }
    }

    void AstVisitor::VisitTranslationUnit(TranslationUnitNode* node) {
        for (auto& stmt : node->stmts) {
            Traverse(stmt.get());
        }
    }

    void AstVisitor::VisitDeclarationGroup(DeclarationGroupNode* node) {
        for (auto& decl : node->decls) {
            Traverse(decl.get());
        }
    }

    void AstVisitor::VisitPreprocessor(PreprocessorNode* node) {}

    void AstVisitor::VisitFunctionDeclaration(FunctionDeclarationNode* node) {
        for (auto& param : node->params) {
            Traverse(param.get());
        }

        if (node->body != nullptr) {
            Traverse(node->body.get());
        }
    }

    void AstVisitor::VisitVariableDeclaration(VariableDeclarationNode* node) {
        if (node->array_size != nullptr) {
            Traverse(node->array_size.get());
        }

        if (node->init != nullptr) {
            Traverse(node->init.get());
        }
    }

    void AstVisitor::VisitInterfaceDeclaration(InterfaceDeclarationNode* node) {
        if (node->body != nullptr) {
            Traverse(node->body.get());
        }

        if (node->instances != nullptr) {
            Traverse(node->instances.get());
        }
    }

    void AstVisitor::VisitStructDeclaration(StructDeclarationNode* node) {
        if (node->body != nullptr) {
            Traverse(node->body.get());
        }

        if (node->instances != nullptr) {
            Traverse(node->instances.get());
        }
    }

    void AstVisitor::VisitCompoundStatement(CompoundStatementNode* node) {
        for (auto& child : node->children) {
            Traverse(child.get());
        }
    }

    void AstVisitor::VisitIfStatement(IfStatementNode* node) {
        if (node->condition != nullptr) {
            Traverse(node->condition.get());
        }

        if (node->then_branch != nullptr) {
            Traverse(node->then_branch.get());
        }

        if (node->else_branch != nullptr) {
            Traverse(node->else_branch.get());
        }
    }

    void AstVisitor::VisitForStatement(ForStatementNode* node) {
        if (node->init != nullptr) {
            Traverse(node->init.get());
        }

        if (node->condition != nullptr) {
            Traverse(node->condition.get());
        }

        if (node->iteration != nullptr) {
            Traverse(node->iteration.get());
        }

        if (node->body != nullptr) {
            Traverse(node->body.get());
        }
    }

    void AstVisitor::VisitWhileStatement(WhileStatementNode* node) {
        if (node->condition != nullptr) {
            Traverse(node->condition.get());
        }

        if (node->body != nullptr) {
            Traverse(node->body.get());
        }
    }

    void AstVisitor::VisitDoStatement(DoStatementNode* node) {
        if (node->body != nullptr) {
            Traverse(node->body.get());
        }

        if (node->condition != nullptr) {
            Traverse(node->condition.get());
        }
    }

    void AstVisitor::VisitSwitchStatement(SwitchStatementNode* node) {
        if (node->condition != nullptr) {
            Traverse(node->condition.get());
        }

        for (auto& case_node : node->cases) {
            Traverse(case_node.get());
        }
    }

    void AstVisitor::VisitCaseStatement(CaseStatementNode* node) {
        if (node->condition != nullptr) {
            Traverse(node->condition.get());
        }

        for (auto& stmt : node->body) {
            Traverse(stmt.get());
        }
    }

    void AstVisitor::VisitReturnStatement(ReturnStatementNode* node) {
        if (node->return_value != nullptr) {
            Traverse(node->return_value.get());
        }
    }

    void AstVisitor::VisitBreakStatement(BreakStatementNode* node) {}
    void AstVisitor::VisitContinueStatement(ContinueStatementNode* node) {}
    void AstVisitor::VisitDiscardStatement(DiscardStatementNode* node) {}

    void AstVisitor::VisitExpressionStatement(ExpressionStatementNode* node) {
        if (node->expr != nullptr) {
            Traverse(node->expr.get());
        }
    }

    void AstVisitor::VisitNullStatement(NullStatementNode* node) {}

    void AstVisitor::VisitInitializerListExpression(InitializerListExpressionNode* node) {
        for (auto& element : node->elements) {
            Traverse(element.get());
        }
    }

    void AstVisitor::VisitBinaryExpression(BinaryExpressionNode* node) {
        if (node->left != nullptr) {
            Traverse(node->left.get());
        }

        if (node->right != nullptr) {
            Traverse(node->right.get());
        }
    }

    void AstVisitor::VisitUnaryExpression(UnaryExpressionNode* node) {
        if (node->operand != nullptr) {
            Traverse(node->operand.get());
        }
    }

    void AstVisitor::VisitCallExpression(CallExpressionNode* node) {
        if (node->callee != nullptr) {
            Traverse(node->callee.get());
        }

        for (auto& arg : node->args) {
            Traverse(arg.get());
        }
    }

    void AstVisitor::VisitIndexExpression(IndexExpressionNode* node) {
        if (node->base != nullptr) {
            Traverse(node->base.get());
        }

        if (node->index != nullptr) {
            Traverse(node->index.get());
        }
    }

    void AstVisitor::VisitVariableExpression(VariableExpressionNode* node) {}
    void AstVisitor::VisitRawExpression(RawExpressionNode* node) {}

    void AstVisitor::VisitMemberAccessExpression(MemberAccessExpressionNode* node) {
        if (node->object != nullptr) {
            Traverse(node->object.get());
        }
    }
}
