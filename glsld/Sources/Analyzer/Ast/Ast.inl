#include "Ast.hpp"
#include <type_traits>

namespace glsld {
    template <typename Self>
    auto AstNode::DefaultClone(this Self&& self) {
        return std::make_unique<std::remove_cvref_t<Self>>(self);
    }

    inline AstNodeKind QualifierArgumentNode::kind() const {
        return AstNodeKind::kQualifierArgument;
    }

    inline std::unique_ptr<AstNode> QualifierArgumentNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind LayoutQualifierNode::kind() const {
        return AstNodeKind::kLayoutQualifier;
    }

    inline std::unique_ptr<AstNode> LayoutQualifierNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind SpirvIntrinsicNode::kind() const {
        return AstNodeKind::kSpirvIntrinsic;
    }

    inline std::unique_ptr<AstNode> SpirvIntrinsicNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind AttributeNode::kind() const {
        return AstNodeKind::kAttribute;
    }

    inline std::unique_ptr<AstNode> AttributeNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind PreprocessorNode::kind() const {
        return AstNodeKind::kPreprocessor;
    }

    inline std::unique_ptr<AstNode> PreprocessorNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind CompoundStatementNode::kind() const {
        return AstNodeKind::kCompoundStatement;
    }

    inline std::unique_ptr<AstNode> CompoundStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind IfStatementNode::kind() const {
        return AstNodeKind::kIfStatement;
    }

    inline std::unique_ptr<AstNode> IfStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind ForStatementNode::kind() const {
        return AstNodeKind::kForStatement;
    }

    inline std::unique_ptr<AstNode> ForStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind WhileStatementNode::kind() const {
        return AstNodeKind::kWhileStatement;
    }

    inline std::unique_ptr<AstNode> WhileStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind DoStatementNode::kind() const {
        return AstNodeKind::kDoStatement;
    }

    inline std::unique_ptr<AstNode> DoStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind SwitchStatementNode::kind() const {
        return AstNodeKind::kSwitchStatement;
    }

    inline std::unique_ptr<AstNode> SwitchStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind CaseStatementNode::kind() const {
        return AstNodeKind::kCaseStatement;
    }

    inline std::unique_ptr<AstNode> CaseStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind ReturnStatementNode::kind() const {
        return AstNodeKind::kReturnStatement;
    }

    inline std::unique_ptr<AstNode> ReturnStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind BreakStatementNode::kind() const {
        return AstNodeKind::kBreakStatement;
    }

    inline std::unique_ptr<AstNode> BreakStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind ContinueStatementNode::kind() const {
        return AstNodeKind::kContinueStatement;
    }

    inline std::unique_ptr<AstNode> ContinueStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind DiscardStatementNode::kind() const {
        return AstNodeKind::kDiscardStatement;
    }

    inline std::unique_ptr<AstNode> DiscardStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind ExpressionStatementNode::kind() const {
        return AstNodeKind::kExpressionStatement;
    }

    inline std::unique_ptr<AstNode> ExpressionStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind NullStatementNode::kind() const {
        return AstNodeKind::kNullStatement;
    }

    inline std::unique_ptr<AstNode> NullStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind InitializerListExpressionNode::kind() const {
        return AstNodeKind::kInitializerListExpression;
    }

    inline std::unique_ptr<AstNode> InitializerListExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind BinaryExpressionNode::kind() const {
        return AstNodeKind::kBinaryExpression;
    }

    inline std::unique_ptr<AstNode> BinaryExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind UnaryExpressionNode::kind() const {
        return AstNodeKind::kUnaryExpression;
    }

    inline std::unique_ptr<AstNode> UnaryExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind TernaryExpressionNode::kind() const {
        return AstNodeKind::kTernaryExpression;
    }

    inline std::unique_ptr<AstNode> TernaryExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind CallExpressionNode::kind() const {
        return AstNodeKind::kCallExpression;
    }

    inline std::unique_ptr<AstNode> CallExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind IndexExpressionNode::kind() const {
        return AstNodeKind::kIndexExpression;
    }

    inline std::unique_ptr<AstNode> IndexExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind VariableExpressionNode::kind() const {
        return AstNodeKind::kVariableExpression;
    }

    inline std::unique_ptr<AstNode> VariableExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind RawExpressionNode::kind() const {
        return AstNodeKind::kLiteralExpression;
    }

    inline std::unique_ptr<AstNode> RawExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind MemberAccessExpressionNode::kind() const {
        return AstNodeKind::kMemberAccessExpression;
    }

    inline std::unique_ptr<AstNode> MemberAccessExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind VariableDeclarationNode::kind() const {
        return AstNodeKind::kVariableDeclaration;
    }

    inline std::unique_ptr<AstNode> VariableDeclarationNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind DeclarationGroupNode::kind() const {
        return AstNodeKind::kDeclarationGroup;
    }

    inline std::unique_ptr<AstNode> DeclarationGroupNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind FunctionDeclarationNode::kind() const {
        return AstNodeKind::kFunctionDeclaration;
    }

    inline std::unique_ptr<AstNode> FunctionDeclarationNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind InterfaceDeclarationNode::kind() const {
        return AstNodeKind::kInterfaceDeclaration;
    }

    inline std::unique_ptr<AstNode> InterfaceDeclarationNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind StructDeclarationNode::kind() const {
        return AstNodeKind::kStructDeclaration;
    }

    inline std::unique_ptr<AstNode> StructDeclarationNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind TranslationUnitNode::kind() const {
        return AstNodeKind::kTranslationUnit;
    }

    inline std::unique_ptr<AstNode> TranslationUnitNode::Clone() const {
        return DefaultClone();
    }
} // namespace glsld
