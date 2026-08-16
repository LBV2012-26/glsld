#include "pch.hpp"
#include "AstVisitor.hpp"

namespace glsld {
    AstVisitor::AstVisitor(int version_replica, VersionPointer version_pointer)
        : version_replica_{ version_replica }
        , version_pointer_{ version_pointer }
    {}

    namespace {
        bool IsStatementKind(AstNodeKind kind) {
            switch (kind) {
            case AstNodeKind::kCompoundStatement:
            case AstNodeKind::kIfStatement:
            case AstNodeKind::kForStatement:
            case AstNodeKind::kWhileStatement:
            case AstNodeKind::kDoStatement:
            case AstNodeKind::kSwitchStatement:
            case AstNodeKind::kCaseStatement:
            case AstNodeKind::kReturnStatement:
            case AstNodeKind::kBreakStatement:
            case AstNodeKind::kContinueStatement:
            case AstNodeKind::kDiscardStatement:
            case AstNodeKind::kExpressionStatement:
            case AstNodeKind::kNullStatement:
            case AstNodeKind::kDeclarationGroup:
            case AstNodeKind::kPreprocessor:
            case AstNodeKind::kFunctionDeclaration:
            case AstNodeKind::kVariableDeclaration:
            case AstNodeKind::kInterfaceDeclaration:
            case AstNodeKind::kStructDeclaration:
                return true;
            default:
                return false;
            }
        }
    }

    void AstVisitor::Traverse(AstNode* node) {
        if (node == nullptr) {
            return;
        }

        if (version_pointer_ != nullptr && version_replica_ != version_pointer_->load(std::memory_order::relaxed)) {
            return;
        }

        auto* previous_scope = current_scope_;
        if (node->internal_scope != nullptr) {
            current_scope_ = node->internal_scope;
        } else {
            current_scope_ = node->located_scope;
        }

        if (IsStatementKind(node->kind())) {
            for (auto& attribute : static_cast<StatementNode*>(node)->attributes) {
                VisitAttribute(attribute);
            }
        }

        DispatchCommonVisit(node);

        current_scope_ = previous_scope;
    }

    void AstVisitor::DispatchCommonVisit(AstNode* node) {
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
        case AstNodeKind::kQualifierArgument:
            VisitQualifierArgument(static_cast<QualifierArgumentNode*>(node));
            break;
        case AstNodeKind::kLayoutQualifier:
            VisitLayoutQualifier(static_cast<LayoutQualifierNode*>(node));
            break;
        case AstNodeKind::kSpirvIntrinsic:
            VisitSpirvIntrinsic(static_cast<SpirvIntrinsicNode*>(node));
            break;
        case AstNodeKind::kFunctionDeclaration:
            VisitFunctionDeclaration(static_cast<FunctionDeclarationNode*>(node));
            break;
        case AstNodeKind::kVariableDeclaration:
            VisitVariableDeclaration(static_cast<VariableDeclarationNode*>(node));
            break;
        case AstNodeKind::kInterfaceDeclaration:
            VisitInterfaceDeclaration(static_cast<InterfaceDeclarationNode*>(node));
            break;
        case AstNodeKind::kStructDeclaration:
            VisitStructDeclaration(static_cast<StructDeclarationNode*>(node));
            break;
        case AstNodeKind::kCompoundStatement:
            VisitCompoundStatement(static_cast<CompoundStatementNode*>(node));
            break;
        case AstNodeKind::kIfStatement:
            VisitIfStatement(static_cast<IfStatementNode*>(node));
            break;
        case AstNodeKind::kForStatement:
            VisitForStatement(static_cast<ForStatementNode*>(node));
            break;
        case AstNodeKind::kWhileStatement:
            VisitWhileStatement(static_cast<WhileStatementNode*>(node));
            break;
        case AstNodeKind::kDoStatement:
            VisitDoStatement(static_cast<DoStatementNode*>(node));
            break;
        case AstNodeKind::kSwitchStatement:
            VisitSwitchStatement(static_cast<SwitchStatementNode*>(node));
            break;
        case AstNodeKind::kCaseStatement:
            VisitCaseStatement(static_cast<CaseStatementNode*>(node));
            break;
        case AstNodeKind::kReturnStatement:
            VisitReturnStatement(static_cast<ReturnStatementNode*>(node));
            break;
        case AstNodeKind::kBreakStatement:
            VisitBreakStatement(static_cast<BreakStatementNode*>(node));
            break;
        case AstNodeKind::kContinueStatement:
            VisitContinueStatement(static_cast<ContinueStatementNode*>(node));
            break;
        case AstNodeKind::kDiscardStatement:
            VisitDiscardStatement(static_cast<DiscardStatementNode*>(node));
            break;
        case AstNodeKind::kExpressionStatement:
            VisitExpressionStatement(static_cast<ExpressionStatementNode*>(node));
            break;
        case AstNodeKind::kNullStatement:
            VisitNullStatement(static_cast<NullStatementNode*>(node));
            break;
        case AstNodeKind::kInitializerListExpression:
            VisitInitializerListExpression(static_cast<InitializerListExpressionNode*>(node));
            break;
        case AstNodeKind::kBinaryExpression:
            VisitBinaryExpression(static_cast<BinaryExpressionNode*>(node));
            break;
        case AstNodeKind::kUnaryExpression:
            VisitUnaryExpression(static_cast<UnaryExpressionNode*>(node));
            break;
        case AstNodeKind::kTernaryExpression:
            VisitTernaryExpression(static_cast<TernaryExpressionNode*>(node));
            break;
        case AstNodeKind::kCallExpression:
            VisitCallExpression(static_cast<CallExpressionNode*>(node));
            break;
        case AstNodeKind::kIndexExpression:
            VisitIndexExpression(static_cast<IndexExpressionNode*>(node));
            break;
        case AstNodeKind::kVariableExpression:
            VisitVariableExpression(static_cast<VariableExpressionNode*>(node));
            break;
        case AstNodeKind::kLiteralExpression:
            VisitRawExpression(static_cast<RawExpressionNode*>(node));
            break;
        case AstNodeKind::kMemberAccessExpression:
            VisitMemberAccessExpression(static_cast<MemberAccessExpressionNode*>(node));
            break;
        default:
            break;
        }
    }

    void AstVisitor::VisitTranslationUnit(TranslationUnitNode* node) {
        for (auto& statement : node->statements) {
            Traverse(statement);
        }
    }

    void AstVisitor::VisitDeclarationGroup(DeclarationGroupNode* node) {
        for (auto& declaration : node->declarations) {
            Traverse(declaration);
        }
    }

    void AstVisitor::VisitPreprocessor(PreprocessorNode* node) {
        for (auto& statement : node->body) {
            Traverse(statement);
        }
    }

    void AstVisitor::VisitAttribute(AttributeNode* node) {
        Traverse(node->argument);
    }

    void AstVisitor::VisitQualifierArgument(QualifierArgumentNode* node) {
        for (auto& child : node->children) {
            Traverse(child);
        }

        if (node->rhs_expr != nullptr) {
            Traverse(node->rhs_expr);
        }
    }

    void AstVisitor::VisitLayoutQualifier(LayoutQualifierNode* node) {
        for (auto& param : node->params) {
            Traverse(param);
        }
    }

    void AstVisitor::VisitSpirvIntrinsic(SpirvIntrinsicNode* node) {
        for (auto& param : node->params) {
            Traverse(param);
        }
    }

    void AstVisitor::VisitFunctionDeclaration(FunctionDeclarationNode* node) {
        for (auto& layout : node->type_spec.layouts) {
            Traverse(layout);
        }

        for (auto& spirv_intrinsic : node->type_spec.spirv_intrinsics) {
            Traverse(spirv_intrinsic);
        }

        for (auto& template_arg : node->type_spec.template_args) {
            Traverse(template_arg);
        }

        for (auto& size_expr : node->type_spec.array_sizes) {
            Traverse(size_expr);
        }

        for (auto& param : node->params) {
            Traverse(param);
        }

        if (node->body != nullptr) {
            Traverse(node->body);
        }
    }

    void AstVisitor::VisitVariableDeclaration(VariableDeclarationNode* node) {
        for (auto& layout : node->type_spec.layouts) {
            Traverse(layout);
        }

        for (auto& spirv_intrinsic : node->type_spec.spirv_intrinsics) {
            Traverse(spirv_intrinsic);
        }

        for (auto& template_arg : node->type_spec.template_args) {
            Traverse(template_arg);
        }

        for (auto& size_expr : node->type_spec.array_sizes) {
            Traverse(size_expr);
        }

        if (node->init != nullptr) {
            Traverse(node->init);
        }
    }

    void AstVisitor::VisitInterfaceDeclaration(InterfaceDeclarationNode* node) {
        for (auto& layout : node->type_spec.layouts) {
            Traverse(layout);
        }

        for (auto& spirv_intrinsic : node->type_spec.spirv_intrinsics) {
            Traverse(spirv_intrinsic);
        }

        for (auto& template_arg : node->type_spec.template_args) {
            Traverse(template_arg);
        }

        if (node->body != nullptr) {
            Traverse(node->body);
        }

        if (node->instances != nullptr) {
            Traverse(node->instances);
        }
    }

    void AstVisitor::VisitStructDeclaration(StructDeclarationNode* node) {
        if (node->body != nullptr) {
            Traverse(node->body);
        }

        if (node->instances != nullptr) {
            Traverse(node->instances);
        }
    }

    void AstVisitor::VisitCompoundStatement(CompoundStatementNode* node) {
        for (auto& child : node->children) {
            Traverse(child);
        }
    }

    void AstVisitor::VisitIfStatement(IfStatementNode* node) {
        if (node->condition != nullptr) {
            Traverse(node->condition);
        }

        if (node->then_branch != nullptr) {
            Traverse(node->then_branch);
        }

        if (node->else_branch != nullptr) {
            Traverse(node->else_branch);
        }
    }

    void AstVisitor::VisitForStatement(ForStatementNode* node) {
        if (node->init != nullptr) {
            Traverse(node->init);
        }

        if (node->condition != nullptr) {
            Traverse(node->condition);
        }

        if (node->iteration != nullptr) {
            Traverse(node->iteration);
        }

        if (node->body != nullptr) {
            Traverse(node->body);
        }
    }

    void AstVisitor::VisitWhileStatement(WhileStatementNode* node) {
        if (node->condition != nullptr) {
            Traverse(node->condition);
        }

        if (node->body != nullptr) {
            Traverse(node->body);
        }
    }

    void AstVisitor::VisitDoStatement(DoStatementNode* node) {
        if (node->body != nullptr) {
            Traverse(node->body);
        }

        if (node->condition != nullptr) {
            Traverse(node->condition);
        }
    }

    void AstVisitor::VisitSwitchStatement(SwitchStatementNode* node) {
        if (node->condition != nullptr) {
            Traverse(node->condition);
        }

        for (auto& case_node : node->cases) {
            Traverse(case_node);
        }
    }

    void AstVisitor::VisitCaseStatement(CaseStatementNode* node) {
        if (node->condition != nullptr) {
            Traverse(node->condition);
        }

        for (auto& statement : node->body) {
            Traverse(statement);
        }
    }

    void AstVisitor::VisitReturnStatement(ReturnStatementNode* node) {
        if (node->return_value != nullptr) {
            Traverse(node->return_value);
        }
    }

    void AstVisitor::VisitBreakStatement(BreakStatementNode* node) {
        (void)node; // No additional traversal needed for break statements
    }

    void AstVisitor::VisitContinueStatement(ContinueStatementNode* node) {
        (void)node; // No additional traversal needed for continue statements
    }

    void AstVisitor::VisitDiscardStatement(DiscardStatementNode* node) {
        (void)node; // No additional traversal needed for discard statements
    }

    void AstVisitor::VisitExpressionStatement(ExpressionStatementNode* node) {
        if (node->expr != nullptr) {
            Traverse(node->expr);
        }
    }

    void AstVisitor::VisitNullStatement(NullStatementNode* node) {
        (void)node; // No additional traversal needed for null statements
    }

    void AstVisitor::VisitInitializerListExpression(InitializerListExpressionNode* node) {
        for (auto& element : node->elements) {
            Traverse(element);
        }
    }

    void AstVisitor::VisitBinaryExpression(BinaryExpressionNode* node) {
        if (node->left != nullptr) {
            Traverse(node->left);
        }

        if (node->right != nullptr) {
            Traverse(node->right);
        }
    }

    void AstVisitor::VisitUnaryExpression(UnaryExpressionNode* node) {
        if (node->operand != nullptr) {
            Traverse(node->operand);
        }
    }

    void AstVisitor::VisitTernaryExpression(TernaryExpressionNode* node) {
        if (node->condition != nullptr) {
            Traverse(node->condition);
        }

        if (node->true_expr != nullptr) {
            Traverse(node->true_expr);
        }

        if (node->false_expr != nullptr) {
            Traverse(node->false_expr);
        }
    }

    void AstVisitor::VisitCallExpression(CallExpressionNode* node) {
        if (node->callee != nullptr) {
            Traverse(node->callee);
        }

        for (auto& arg : node->args) {
            Traverse(arg);
        }
    }

    void AstVisitor::VisitIndexExpression(IndexExpressionNode* node) {
        if (node->base != nullptr) {
            Traverse(node->base);
        }

        if (node->index != nullptr) {
            Traverse(node->index);
        }
    }

    void AstVisitor::VisitVariableExpression(VariableExpressionNode* node) {
        (void)node; // No additional traversal needed for variable expressions
    }

    void AstVisitor::VisitRawExpression(RawExpressionNode* node) {
        (void)node; // No additional traversal needed for raw expressions
    }

    void AstVisitor::VisitMemberAccessExpression(MemberAccessExpressionNode* node) {
        if (node->object != nullptr) {
            Traverse(node->object);
        }

        if (node->member != nullptr) {
            Traverse(node->member);
        }
    }
}
