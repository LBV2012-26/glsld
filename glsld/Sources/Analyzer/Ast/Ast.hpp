#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Token.hpp"

namespace glsld {
    enum class AstNodeKind {
        kTranslationUnit,
        kDeclarationGroup,
        kPreprocessor,
        kAttribute,

        // Declarations
        kFunctionDeclaration,
        kVariableDeclaration,
        kInterfaceDeclaration,
        kStructDeclaration,

        // Statements
        kCompoundStatement,     // { ... }
        kIfStatement,
        kForStatement,
        kWhileStatement,
        kDoStatement,
        kSwitchStatement,
        kCaseStatement,
        kReturnStatement,
        kBreakStatement,
        kContinueStatement,
        kDiscardStatement,
        kExpressionStatement,   // index = 1;
        kNullStatement,         // empty statement ";"

        // Expressions
        kInitializerListExpression,
        kBinaryExpression,
        kUnaryExpression,
        kTernaryExpression,
        kCallExpression,
        kIndexExpression,
        kVariableExpression,    // 变量引用
        kLiteralExpression,
        kMemberAccessExpression // struct.member
    };

    using SymbolList      = std::vector<const SymbolInfo*>;
    using SymbolReference = std::variant<std::monostate, const SymbolInfo*, SymbolList>;

    struct AstNode {
        SourceLocation begin;
        SourceLocation end;
        Scope* located_scope{ nullptr };
        Scope* internal_scope{ nullptr };

        AstNode(Scope* scope)
            : located_scope{ scope }
        {}

        virtual ~AstNode() = default;
        virtual AstNodeKind kind() const = 0;
    };

    struct ExpressionNode;
    struct AttributeNode : public AstNode {
        Token namespace_;
        Token name;
        std::unique_ptr<ExpressionNode> argument;

        using AstNode::AstNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kAttribute;
        }
    };

    struct StatementNode : public AstNode {
        std::vector<std::unique_ptr<AttributeNode>> attributes;
        using AstNode::AstNode;
    };

    struct ExpressionNode : public AstNode {
        TypeInfo evaluated_type;
        using AstNode::AstNode;
    };

    struct PreprocessorNode : public StatementNode {
        std::string              directive;
        std::vector<Token>       tokens;
        std::vector<std::string> params;
        const SymbolInfo*        symbol{ nullptr };

        using StatementNode::StatementNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kPreprocessor;
        }
    };

    struct DeclarationNode : public StatementNode {
        SymbolInfo* declared_symbol{ nullptr };
        using StatementNode::StatementNode;
    };

    struct CompoundStatementNode : public StatementNode {
        std::vector<std::unique_ptr<StatementNode>> children;

        using StatementNode::StatementNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kCompoundStatement;
        }
    };

    struct IfStatementNode : public StatementNode {
        std::unique_ptr<ExpressionNode> condition;
        std::unique_ptr<StatementNode>  then_branch;
        std::unique_ptr<StatementNode>  else_branch;

        using StatementNode::StatementNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kIfStatement;
        }
    };

    struct ForStatementNode : public StatementNode {
        std::unique_ptr<StatementNode>  init;
        std::unique_ptr<ExpressionNode> condition;
        std::unique_ptr<ExpressionNode> iteration;
        std::unique_ptr<StatementNode>  body;

        using StatementNode::StatementNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kForStatement;
        }
    };

    struct WhileStatementNode : public StatementNode {
        std::unique_ptr<ExpressionNode> condition;
        std::unique_ptr<StatementNode>  body;

        using StatementNode::StatementNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kWhileStatement;
        }
    };

    struct DoStatementNode : public StatementNode {
        std::unique_ptr<StatementNode>  body;
        std::unique_ptr<ExpressionNode> condition;

        using StatementNode::StatementNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kDoStatement;
        }
    };

    struct SwitchStatementNode : public StatementNode {
        std::unique_ptr<ExpressionNode> condition;
        std::vector<std::unique_ptr<StatementNode>> cases;

        using StatementNode::StatementNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kSwitchStatement;
        }
    };

    struct CaseStatementNode : public StatementNode {
        std::unique_ptr<ExpressionNode> condition; // nullptr for "default"
        std::vector<std::unique_ptr<StatementNode>> body;

        using StatementNode::StatementNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kCaseStatement;
        }
    };

    struct ReturnStatementNode : public StatementNode {
        std::unique_ptr<ExpressionNode> return_value;

        using StatementNode::StatementNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kReturnStatement;
        }
    };

    struct BreakStatementNode : public StatementNode {
        using StatementNode::StatementNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kBreakStatement;
        }
    };

    struct ContinueStatementNode : public StatementNode {
        using StatementNode::StatementNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kContinueStatement;
        }
    };

    struct DiscardStatementNode : public StatementNode {
        using StatementNode::StatementNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kDiscardStatement;
        }
    };

    struct ExpressionStatementNode : public StatementNode {
        using StatementNode::StatementNode;

        std::unique_ptr<ExpressionNode> expr;

        AstNodeKind kind() const override {
            return AstNodeKind::kExpressionStatement;
        }
    };

    struct NullStatementNode : public StatementNode {
        using StatementNode::StatementNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kNullStatement;
        }
    };

    struct InitializerListExpressionNode : public ExpressionNode {
        std::vector<std::unique_ptr<ExpressionNode>> elements;

        using ExpressionNode::ExpressionNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kInitializerListExpression;
        }
    };

    struct BinaryExpressionNode : public ExpressionNode {
        TokenType                       op{};
        std::unique_ptr<ExpressionNode> left;
        std::unique_ptr<ExpressionNode> right;

        using ExpressionNode::ExpressionNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kBinaryExpression;
        }
    };

    struct UnaryExpressionNode : public ExpressionNode {
        TokenType                       op{};
        bool                            is_postfix{ false };
        std::unique_ptr<ExpressionNode> operand;

        using ExpressionNode::ExpressionNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kUnaryExpression;
        }
    };

    struct TernaryExpressionNode : public ExpressionNode {
        std::unique_ptr<ExpressionNode> condition;
        std::unique_ptr<ExpressionNode> true_expr;
        std::unique_ptr<ExpressionNode> false_expr;

        using ExpressionNode::ExpressionNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kTernaryExpression;
        }
    };

    struct CallExpressionNode : public ExpressionNode {
        std::unique_ptr<ExpressionNode>              callee;
        std::vector<std::unique_ptr<ExpressionNode>> args;

        using ExpressionNode::ExpressionNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kCallExpression;
        }
    };

    struct IndexExpressionNode : public ExpressionNode {
        std::unique_ptr<ExpressionNode> base;
        std::unique_ptr<ExpressionNode> index;

        using ExpressionNode::ExpressionNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kIndexExpression;
        }
    };

    struct VariableExpressionNode : public ExpressionNode {
        enum class NodeType {
            kCommonVariable,
            kFunctionCallee,
            kBlockMember
        };

        TokenType       token_type;
        NodeType        node_type;
        std::string     name;
        SymbolReference linked_symbols{ std::monostate{} };

        using ExpressionNode::ExpressionNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kVariableExpression;
        }
    };

    struct RawExpressionNode : public ExpressionNode {
        std::vector<Token> tokens;

        using ExpressionNode::ExpressionNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kLiteralExpression;
        }
    };

    struct MemberAccessExpressionNode : public ExpressionNode {
        std::unique_ptr<ExpressionNode>         object;
        std::unique_ptr<VariableExpressionNode> member;

        using ExpressionNode::ExpressionNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kMemberAccessExpression;
        }
    };

    struct TypeSpecifier {
        std::vector<Token> specifiers;
        std::vector<Token> layout_params;
        std::vector<std::shared_ptr<ExpressionNode>> array_sizes;

        Token typename_token() const {
            return specifiers.empty() ? Token{} : specifiers.back();
        }

        SourceLocation begin_location() const {
            return specifiers.empty() ? SourceLocation{} : specifiers.front().location;
        }

        bool has_keyword(std::string_view name) const {
            return std::ranges::any_of(specifiers, [name](auto& token) -> bool {
                return token.text == name;
            });
        }

        bool empty() const {
            return specifiers.empty();
        }
    };

    struct VariableDeclarationNode : public DeclarationNode {
        std::unique_ptr<ExpressionNode> init;
        TypeSpecifier                   type_spec;

        using DeclarationNode::DeclarationNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kVariableDeclaration;
        }
    };

    struct DeclarationGroupNode : public StatementNode {
        std::vector<std::unique_ptr<VariableDeclarationNode>> declarations;

        using StatementNode::StatementNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kDeclarationGroup;
        }
    };

    struct FunctionDeclarationNode : public DeclarationNode {
        std::vector<std::unique_ptr<VariableDeclarationNode>> params;
        std::unique_ptr<CompoundStatementNode>                body;
        TypeSpecifier                                         type_spec;

        using DeclarationNode::DeclarationNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kFunctionDeclaration;
        }
    };

    struct InterfaceDeclarationNode : public DeclarationNode {
        std::unique_ptr<CompoundStatementNode> body;
        std::unique_ptr<DeclarationGroupNode>  instances;
        TypeSpecifier                          type_spec;

        using DeclarationNode::DeclarationNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kInterfaceDeclaration;
        }
    };

    struct StructDeclarationNode : public DeclarationNode {
        std::unique_ptr<CompoundStatementNode> body;
        std::unique_ptr<DeclarationGroupNode>  instances;

        using DeclarationNode::DeclarationNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kStructDeclaration;
        }
    };

    struct TranslationUnitNode : public AstNode {
        std::vector<std::unique_ptr<StatementNode>> statements;
        std::vector<PreprocessorNode*> preprocessor_references;

        using AstNode::AstNode;

        AstNodeKind kind() const override {
            return AstNodeKind::kTranslationUnit;
        }
    };
}
