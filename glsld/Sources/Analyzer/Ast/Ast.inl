#include "Ast.hpp"
#include <type_traits>

namespace glsld {
    inline Token TypeSpec::typename_token() const {
        return specifiers.empty() ? Token{} : specifiers.back();
    }

    inline SourceLocation TypeSpec::begin_location() const {
        return specifiers.empty() ? SourceLocation{} : specifiers.front().location;
    }

    inline bool TypeSpec::empty() const {
        return specifiers.empty();
    }

    template <typename Self>
    auto AstNode::DefaultClone(this Self&& self) {
        using NodeType = std::remove_cvref_t<Self>;
        return self.arena->template Construct<NodeType>(self);
    }

    inline AstNodeKind QualifierArgumentNode::kind() const {
        return AstNodeKind::kQualifierArgument;
    }

    inline AstNode* QualifierArgumentNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind LayoutQualifierNode::kind() const {
        return AstNodeKind::kLayoutQualifier;
    }

    inline AstNode* LayoutQualifierNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind SpirvIntrinsicNode::kind() const {
        return AstNodeKind::kSpirvIntrinsic;
    }

    inline AstNode* SpirvIntrinsicNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind AttributeNode::kind() const {
        return AstNodeKind::kAttribute;
    }

    inline AstNode* AttributeNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind PreprocessorNode::kind() const {
        return AstNodeKind::kPreprocessor;
    }

    inline AstNode* PreprocessorNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind CompoundStatementNode::kind() const {
        return AstNodeKind::kCompoundStatement;
    }

    inline AstNode* CompoundStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind IfStatementNode::kind() const {
        return AstNodeKind::kIfStatement;
    }

    inline AstNode* IfStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind ForStatementNode::kind() const {
        return AstNodeKind::kForStatement;
    }

    inline AstNode* ForStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind WhileStatementNode::kind() const {
        return AstNodeKind::kWhileStatement;
    }

    inline AstNode* WhileStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind DoStatementNode::kind() const {
        return AstNodeKind::kDoStatement;
    }

    inline AstNode* DoStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind SwitchStatementNode::kind() const {
        return AstNodeKind::kSwitchStatement;
    }

    inline AstNode* SwitchStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind CaseStatementNode::kind() const {
        return AstNodeKind::kCaseStatement;
    }

    inline AstNode* CaseStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind ReturnStatementNode::kind() const {
        return AstNodeKind::kReturnStatement;
    }

    inline AstNode* ReturnStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind BreakStatementNode::kind() const {
        return AstNodeKind::kBreakStatement;
    }

    inline AstNode* BreakStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind ContinueStatementNode::kind() const {
        return AstNodeKind::kContinueStatement;
    }

    inline AstNode* ContinueStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind DiscardStatementNode::kind() const {
        return AstNodeKind::kDiscardStatement;
    }

    inline AstNode* DiscardStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind ExpressionStatementNode::kind() const {
        return AstNodeKind::kExpressionStatement;
    }

    inline AstNode* ExpressionStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind NullStatementNode::kind() const {
        return AstNodeKind::kNullStatement;
    }

    inline AstNode* NullStatementNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind InitializerListExpressionNode::kind() const {
        return AstNodeKind::kInitializerListExpression;
    }

    inline AstNode* InitializerListExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind CastExpressionNode::kind() const {
        return AstNodeKind::kCastExpression;
    }

    inline AstNode* CastExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind BinaryExpressionNode::kind() const {
        return AstNodeKind::kBinaryExpression;
    }

    inline AstNode* BinaryExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind UnaryExpressionNode::kind() const {
        return AstNodeKind::kUnaryExpression;
    }

    inline AstNode* UnaryExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind TernaryExpressionNode::kind() const {
        return AstNodeKind::kTernaryExpression;
    }

    inline AstNode* TernaryExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind CallExpressionNode::kind() const {
        return AstNodeKind::kCallExpression;
    }

    inline AstNode* CallExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind IndexExpressionNode::kind() const {
        return AstNodeKind::kIndexExpression;
    }

    inline AstNode* IndexExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind VariableExpressionNode::kind() const {
        return AstNodeKind::kVariableExpression;
    }

    inline AstNode* VariableExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind RawExpressionNode::kind() const {
        return AstNodeKind::kLiteralExpression;
    }

    inline AstNode* RawExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind MemberAccessExpressionNode::kind() const {
        return AstNodeKind::kMemberAccessExpression;
    }

    inline AstNode* MemberAccessExpressionNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind VariableDeclarationNode::kind() const {
        return AstNodeKind::kVariableDeclaration;
    }

    inline AstNode* VariableDeclarationNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind DeclarationGroupNode::kind() const {
        return AstNodeKind::kDeclarationGroup;
    }

    inline AstNode* DeclarationGroupNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind FunctionDeclarationNode::kind() const {
        return AstNodeKind::kFunctionDeclaration;
    }

    inline AstNode* FunctionDeclarationNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind InterfaceDeclarationNode::kind() const {
        return AstNodeKind::kInterfaceDeclaration;
    }

    inline AstNode* InterfaceDeclarationNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind StructDeclarationNode::kind() const {
        return AstNodeKind::kStructDeclaration;
    }

    inline AstNode* StructDeclarationNode::Clone() const {
        return DefaultClone();
    }

    inline AstNodeKind TranslationUnitNode::kind() const {
        return AstNodeKind::kTranslationUnit;
    }

    inline AstNode* TranslationUnitNode::Clone() const {
        return DefaultClone();
    }
} // namespace glsld
