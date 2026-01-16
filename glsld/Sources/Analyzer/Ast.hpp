#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Analyzer/SymbolTable.hpp"
#include "Analyzer/Token.hpp"

namespace glsld {
    enum class AstNodeKind {
        kTranslationUnit,
        kDeclarationGroup,
        kPreprocessor,

        // Declarations
        kFunctionDecl,
        kVariableDecl,
        kInterfaceDecl,
        kStructDecl,

        // Statements
        kCompoundStmt, // { ... }
        kIfStmt,
        kForStmt,
        kWhileStmt,
        kDoStmt,
        kSwitchStmt,
        kCaseStmt,
        kReturnStmt,
        kBreakStmt,
        kContinueStmt,
        kDiscardStmt,
        kExprStmt, // (e.g. "index = 1;")
        kNullStmt, // empty statement ";"

        // Expressions
        kBinaryExpr,
        kUnaryExpr,
        kCallExpr,
        kVariableExpr, // 变量引用
        kLiteralExpr,
        kMemberAccessExpr // struct.member
    };

    struct AstNode {
        SourceLocation begin;
        SourceLocation end;

        virtual ~AstNode() = default;
        virtual AstNodeKind kind() const = 0;
    };

    struct StatementNode : public AstNode {};
    struct ExpressionNode : public AstNode {};

    struct PreprocessorNode : public StatementNode {
        std::string directive;
        std::vector<Token> tokens;
        std::vector<std::string> params;

        const SymbolInfo* symbol{ nullptr };

        AstNodeKind kind() const override {
            return AstNodeKind::kPreprocessor;
        }
    };

    struct DeclarationNode : public StatementNode {
        const SymbolInfo* declared_symbol{ nullptr };
    };

    struct CompoundStatementNode : public StatementNode {
        std::vector<std::unique_ptr<StatementNode>> children;
        Scope* scope{ nullptr };

        AstNodeKind kind() const override {
            return AstNodeKind::kCompoundStmt;
        }
    };

    struct IfStatementNode : public StatementNode {
        std::unique_ptr<ExpressionNode> condition;
        std::unique_ptr<StatementNode>  then_branch;
        std::unique_ptr<StatementNode>  else_branch;

        AstNodeKind kind() const override {
            return AstNodeKind::kIfStmt;
        }
    };

    struct ForStatementNode : public StatementNode {
        std::unique_ptr<StatementNode>  init;
        std::unique_ptr<ExpressionNode> condition;
        std::unique_ptr<ExpressionNode> iteration;
        std::unique_ptr<StatementNode>  body;

        AstNodeKind kind() const override {
            return AstNodeKind::kForStmt;
        }
    };

    struct WhileStatementNode : public StatementNode {
        std::unique_ptr<ExpressionNode> condition;
        std::unique_ptr<StatementNode>  body;

        AstNodeKind kind() const override {
            return AstNodeKind::kWhileStmt;
        }
    };

    struct DoStatementNode : public StatementNode {
        std::unique_ptr<StatementNode>  body;
        std::unique_ptr<ExpressionNode> condition;

        AstNodeKind kind() const override {
            return AstNodeKind::kDoStmt;
        }
    };

    struct SwitchStatementNode : public StatementNode {
        std::unique_ptr<ExpressionNode> condition;
        std::vector<std::unique_ptr<StatementNode>> cases;

        AstNodeKind kind() const override {
            return AstNodeKind::kSwitchStmt;
        }
    };

    struct CaseStatementNode : public StatementNode {
        std::unique_ptr<ExpressionNode> condition; // nullptr for "default"
        std::vector<std::unique_ptr<StatementNode>> body;

        AstNodeKind kind() const override {
            return AstNodeKind::kCaseStmt;
        }
    };

    struct ReturnStatementNode : public StatementNode {
        std::unique_ptr<ExpressionNode> return_value;

        AstNodeKind kind() const override {
            return AstNodeKind::kReturnStmt;
        }
    };

    struct BreakStatementNode : public StatementNode {
        AstNodeKind kind() const override {
            return AstNodeKind::kBreakStmt;
        }
    };

    struct ContinueStatementNode : public StatementNode {
        AstNodeKind kind() const override {
            return AstNodeKind::kContinueStmt;
        }
    };

    struct DiscardStatementNode : public StatementNode {
        AstNodeKind kind() const override {
            return AstNodeKind::kDiscardStmt;
        }
    };

    struct ExpressionStatementNode : public StatementNode {
        std::unique_ptr<ExpressionNode> expr;

        AstNodeKind kind() const override {
            return AstNodeKind::kExprStmt;
        }
    };

    struct NullStatementNode : public StatementNode {
        AstNodeKind kind() const override {
            return AstNodeKind::kNullStmt;
        }
    };

    struct BinaryExpressionNode : public ExpressionNode {
        TokenType op;
        std::unique_ptr<ExpressionNode> left;
        std::unique_ptr<ExpressionNode> right;

        AstNodeKind kind() const override {
            return AstNodeKind::kBinaryExpr;
        }
    };

    struct UnaryExpressionNode : public ExpressionNode {
        TokenType op;
        std::unique_ptr<ExpressionNode> operand;

        AstNodeKind kind() const override {
            return AstNodeKind::kUnaryExpr;
        }
    };

    struct CallExpressionNode : public ExpressionNode {
        std::unique_ptr<ExpressionNode> callee;
        std::vector<std::unique_ptr<ExpressionNode>> args;

        AstNodeKind kind() const override {
            return AstNodeKind::kCallExpr;
        }
    };

    struct VariableExpressionNode : public ExpressionNode {
        std::string name;
        Token type_name;
        const SymbolInfo* referenced_symbol{ nullptr };

        AstNodeKind kind() const override {
            return AstNodeKind::kVariableExpr;
        }
    };

    struct RawExpressionNode : public ExpressionNode {
        std::vector<Token> tokens;

        AstNodeKind kind() const override {
            return AstNodeKind::kLiteralExpr;
        }
    };

    struct MemberAccessExpressionNode : public ExpressionNode {
        std::unique_ptr<ExpressionNode> object;
        std::string member_name;

        AstNodeKind kind() const override {
            return AstNodeKind::kMemberAccessExpr;
        }
    };

    struct TypeSpecifier {
        std::vector<Token> qualifiers;
        std::vector<Token> layout_params;

        Token typename_token() const {
            return qualifiers.empty() ? Token{} : qualifiers.back();
        }

        SourceLocation begin_location() const {
            return qualifiers.empty() ? SourceLocation{} : qualifiers.front().location;
        }

        bool has_keyword(std::string_view name) const {
            return std::ranges::any_of(qualifiers, [name](auto& token) -> bool {
                return token.text == name;
            });
        }

        bool empty() const {
            return qualifiers.empty();
        }
    };

    struct VariableDeclarationNode : public DeclarationNode {
        std::unique_ptr<ExpressionNode> init;
        std::unique_ptr<ExpressionNode> array_size;
        TypeSpecifier type_spec;

        AstNodeKind kind() const override {
            return AstNodeKind::kVariableDecl;
        }
    };

    struct DeclarationGroupNode : public StatementNode {
        std::vector<std::unique_ptr<VariableDeclarationNode>> decls;

        AstNodeKind kind() const override {
            return AstNodeKind::kDeclarationGroup;
        }
    };

    struct FunctionDeclarationNode : public DeclarationNode {
        std::vector<std::unique_ptr<VariableDeclarationNode>> params;
        std::unique_ptr<CompoundStatementNode> body;

        AstNodeKind kind() const override {
            return AstNodeKind::kFunctionDecl;
        }
    };

    struct InterfaceDeclarationNode : public DeclarationNode {
        std::vector<Token> qualifiers;
        std::unique_ptr<CompoundStatementNode> body;
        std::unique_ptr<DeclarationGroupNode> instances;

        AstNodeKind kind() const override {
            return AstNodeKind::kInterfaceDecl;
        }
    };

    struct StructDeclarationNode : public DeclarationNode {
        std::unique_ptr<CompoundStatementNode> body;
        std::unique_ptr<DeclarationGroupNode> instances;

        AstNodeKind kind() const override {
            return AstNodeKind::kStructDecl;
        }
    };

    struct TranslationUnitNode : public AstNode {
        std::vector<std::unique_ptr<StatementNode>> stmts;

        AstNodeKind kind() const override {
            return AstNodeKind::kTranslationUnit;
        }
    };
}
