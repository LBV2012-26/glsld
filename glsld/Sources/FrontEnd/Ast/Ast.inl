#include "Ast.hpp"

namespace glsld {
    inline Token TypeSpec::typename_token() const {
        return specifiers.empty() ? Token{} : specifiers.back();
    }

    inline SourceLocation TypeSpec::begin_location() const {
        if (specifiers.empty())
            return {};
        if (specifiers.size() == 1 && typename_begin.source_file() != nullptr)
            return typename_begin;
        return specifiers.front().location;
    }

    inline bool TypeSpec::empty() const {
        return specifiers.empty();
    }

    inline AstNodeKind QualifierArgumentNode::kind() const {
        return AstNodeKind::kQualifierArgument;
    }

    inline AstNodeKind LayoutQualifierNode::kind() const {
        return AstNodeKind::kLayoutQualifier;
    }

    inline AstNodeKind SpirvIntrinsicNode::kind() const {
        return AstNodeKind::kSpirvIntrinsic;
    }

    inline AstNodeKind ExpressionNode::kind() const {
        return AstNodeKind::kExpression;
    }

    inline AstNodeKind AttributeNode::kind() const {
        return AstNodeKind::kAttribute;
    }

    inline AstNodeKind StatementNode::kind() const {
        return AstNodeKind::kStatement;
    }

    inline AstNodeKind PreprocessorNode::kind() const {
        return AstNodeKind::kPreprocessor;
    }

    inline AstNodeKind CompoundStatementNode::kind() const {
        return AstNodeKind::kCompoundStatement;
    }

    inline AstNodeKind IfStatementNode::kind() const {
        return AstNodeKind::kIfStatement;
    }

    inline AstNodeKind ForStatementNode::kind() const {
        return AstNodeKind::kForStatement;
    }

    inline AstNodeKind WhileStatementNode::kind() const {
        return AstNodeKind::kWhileStatement;
    }

    inline AstNodeKind DoStatementNode::kind() const {
        return AstNodeKind::kDoStatement;
    }

    inline AstNodeKind SwitchStatementNode::kind() const {
        return AstNodeKind::kSwitchStatement;
    }

    inline AstNodeKind CaseStatementNode::kind() const {
        return AstNodeKind::kCaseStatement;
    }

    inline AstNodeKind ReturnStatementNode::kind() const {
        return AstNodeKind::kReturnStatement;
    }

    inline AstNodeKind BreakStatementNode::kind() const {
        return AstNodeKind::kBreakStatement;
    }

    inline AstNodeKind ContinueStatementNode::kind() const {
        return AstNodeKind::kContinueStatement;
    }

    inline AstNodeKind DiscardStatementNode::kind() const {
        return AstNodeKind::kDiscardStatement;
    }

    inline AstNodeKind ExpressionStatementNode::kind() const {
        return AstNodeKind::kExpressionStatement;
    }

    inline AstNodeKind NullStatementNode::kind() const {
        return AstNodeKind::kNullStatement;
    }

    inline AstNodeKind InitializerListExpressionNode::kind() const {
        return AstNodeKind::kInitializerListExpression;
    }

    inline AstNodeKind CastExpressionNode::kind() const {
        return AstNodeKind::kCastExpression;
    }

    inline AstNodeKind BinaryExpressionNode::kind() const {
        return AstNodeKind::kBinaryExpression;
    }

    inline AstNodeKind UnaryExpressionNode::kind() const {
        return AstNodeKind::kUnaryExpression;
    }

    inline AstNodeKind TernaryExpressionNode::kind() const {
        return AstNodeKind::kTernaryExpression;
    }

    inline AstNodeKind CallExpressionNode::kind() const {
        return AstNodeKind::kCallExpression;
    }

    inline AstNodeKind IndexExpressionNode::kind() const {
        return AstNodeKind::kIndexExpression;
    }

    inline AstNodeKind VariableExpressionNode::kind() const {
        return AstNodeKind::kVariableExpression;
    }

    inline AstNodeKind RawExpressionNode::kind() const {
        return AstNodeKind::kRawExpression;
    }

    inline AstNodeKind MemberAccessExpressionNode::kind() const {
        return AstNodeKind::kMemberAccessExpression;
    }

    inline AstNodeKind DeclarationNode::kind() const {
        return AstNodeKind::kDeclaration;
    }

    inline AstNodeKind TypeAliasDeclarationNode::kind() const {
        return AstNodeKind::kTypeAliasDeclaration;
    }

    inline AstNodeKind NamespaceDeclarationNode::kind() const {
        return AstNodeKind::kNamespaceDeclaration;
    }

    inline AstNodeKind VariableDeclarationNode::kind() const {
        return AstNodeKind::kVariableDeclaration;
    }

    inline AstNodeKind DeclarationGroupNode::kind() const {
        return AstNodeKind::kDeclarationGroup;
    }

    inline AstNodeKind FunctionDeclarationNode::kind() const {
        return AstNodeKind::kFunctionDeclaration;
    }

    inline AstNodeKind InterfaceDeclarationNode::kind() const {
        return AstNodeKind::kInterfaceDeclaration;
    }

    inline AstNodeKind StructDeclarationNode::kind() const {
        return AstNodeKind::kStructDeclaration;
    }

    inline AstNodeKind EnumeratorDeclarationNode::kind() const {
        return AstNodeKind::kEnumeratorDeclaration;
    }

    inline AstNodeKind EnumDeclarationNode::kind() const {
        return AstNodeKind::kEnumDeclaration;
    }

    inline AstNodeKind TranslationUnitNode::kind() const {
        return AstNodeKind::kTranslationUnit;
    }
} // namespace glsld
