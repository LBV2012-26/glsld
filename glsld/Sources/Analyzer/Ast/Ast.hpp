// Ast.hpp
#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Token.hpp"

namespace glsld {
    enum class AstNodeKind {
        kTranslationUnit,
        kDeclarationGroup,
        kPreprocessor,
        kAttribute,
        kQualifierArgument,
        kLayoutQualifier,
        kSpirvIntrinsic,

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

    struct AstNode {
        SourceLocation begin;
        SourceLocation end;
        Scope* located_scope{ nullptr };
        Scope* internal_scope{ nullptr };

        AstNode(Scope* scope);
        AstNode(const AstNode& other);
        AstNode(AstNode&&) noexcept = default;
        virtual ~AstNode() = default;

        AstNode& operator=(const AstNode& other);
        AstNode& operator=(AstNode&&) noexcept = default;

        virtual AstNodeKind kind() const = 0;
        virtual std::unique_ptr<AstNode> Clone() const = 0;

        template <typename Self>
        auto DefaultClone(this Self&& self);
    };

    enum class QualifierArgumentKind {
        kUnknown,
        kIdentifier,
        kNumberLiteral,
        kStringLiteral,
        kBoolLiteral,
        kAssignment, // =
        kArray,      // [a, b, c]
        kGroup,      // (a, b, c)
        kSequence    // token sequence
    };

    struct QualifierArgumentNode final : public AstNode {
        QualifierArgumentKind arg_kind{ QualifierArgumentKind::kUnknown };
        Token                 token;
        std::vector<std::unique_ptr<QualifierArgumentNode>> children;

        using AstNode::AstNode;
        QualifierArgumentNode(const QualifierArgumentNode& other);
        QualifierArgumentNode(QualifierArgumentNode&&) noexcept = default;
        ~QualifierArgumentNode() override = default;

        QualifierArgumentNode& operator=(const QualifierArgumentNode& other);
        QualifierArgumentNode& operator=(QualifierArgumentNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct LayoutQualifierNode : public AstNode {
        std::vector<Token>                                  raw_tokens; // 不包含外层括号
        std::vector<std::unique_ptr<QualifierArgumentNode>> params;

        using AstNode::AstNode;
        LayoutQualifierNode(const LayoutQualifierNode& other);
        LayoutQualifierNode(LayoutQualifierNode&&) noexcept = default;
        ~LayoutQualifierNode() override = default;

        LayoutQualifierNode& operator=(const LayoutQualifierNode& other);
        LayoutQualifierNode& operator=(LayoutQualifierNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    enum class SpirvIntrinsicKind {
        kUnknown,
        kTypeOverride, // spirv_type
        kQualifier,    // spirv_decorate / spirv_storage_class / spirv_by_reference / spirv_literal
        kInstruction   // spirv_instruction / spirv_execution_mode / spirv_execution_mode_id
    };

    struct SpirvIntrinsicNode final : public LayoutQualifierNode {
        SpirvIntrinsicKind intrinsic_kind{ SpirvIntrinsicKind::kUnknown };
        Token              keyword;

        using LayoutQualifierNode::LayoutQualifierNode;
        SpirvIntrinsicNode(const SpirvIntrinsicNode& other);
        SpirvIntrinsicNode(SpirvIntrinsicNode&&) noexcept = default;
        ~SpirvIntrinsicNode() override = default;

        SpirvIntrinsicNode& operator=(const SpirvIntrinsicNode& other);
        SpirvIntrinsicNode& operator=(SpirvIntrinsicNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct ExpressionNode : public AstNode {
        TypeInfo evaluated_type;

        using AstNode::AstNode;
        ExpressionNode(const ExpressionNode& other);
        ExpressionNode(ExpressionNode&&) noexcept = default;
        ~ExpressionNode() override = default;

        ExpressionNode& operator=(const ExpressionNode& other);
        ExpressionNode& operator=(ExpressionNode&&) noexcept = default;
    };

    struct AttributeNode final : public AstNode {
        Token                           namespace_;
        Token                           name;
        std::unique_ptr<ExpressionNode> argument;

        using AstNode::AstNode;
        AttributeNode(const AttributeNode& other);
        AttributeNode(AttributeNode&&) noexcept = default;
        ~AttributeNode() override = default;

        AttributeNode& operator=(const AttributeNode& other);
        AttributeNode& operator=(AttributeNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct StatementNode : public AstNode {
        std::vector<std::unique_ptr<AttributeNode>> attributes;

        using AstNode::AstNode;
        StatementNode(const StatementNode& other);
        StatementNode(StatementNode&&) noexcept = default;
        ~StatementNode() override = default;

        StatementNode& operator=(const StatementNode& other);
        StatementNode& operator=(StatementNode&&) noexcept = default;
    };

    struct PreprocessorNode final : public StatementNode {
        std::string                                 directive;
        std::vector<Token>                          tokens;
        std::vector<std::string>                    params;
        std::vector<std::unique_ptr<StatementNode>> body;
        const SymbolInfo* symbol{ nullptr };

        using StatementNode::StatementNode;
        PreprocessorNode(const PreprocessorNode& other);
        PreprocessorNode(PreprocessorNode&&) noexcept = default;
        ~PreprocessorNode() override = default;

        PreprocessorNode& operator=(const PreprocessorNode& other);
        PreprocessorNode& operator=(PreprocessorNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct DeclarationNode : public StatementNode {
        SymbolInfo* declared_symbol{ nullptr };

        using StatementNode::StatementNode;
        DeclarationNode(const DeclarationNode& other);
        DeclarationNode(DeclarationNode&&) noexcept = default;
        ~DeclarationNode() override = default;

        DeclarationNode& operator=(const DeclarationNode& other);
        DeclarationNode& operator=(DeclarationNode&&) noexcept = default;
    };

    struct CompoundStatementNode final : public StatementNode {
        std::vector<std::unique_ptr<StatementNode>> children;

        using StatementNode::StatementNode;
        CompoundStatementNode(const CompoundStatementNode& other);
        CompoundStatementNode(CompoundStatementNode&&) noexcept = default;
        ~CompoundStatementNode() override = default;

        CompoundStatementNode& operator=(const CompoundStatementNode& other);
        CompoundStatementNode& operator=(CompoundStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct IfStatementNode final : public StatementNode {
        std::unique_ptr<ExpressionNode> condition;
        std::unique_ptr<StatementNode>  then_branch;
        std::unique_ptr<StatementNode>  else_branch;

        using StatementNode::StatementNode;
        IfStatementNode(const IfStatementNode& other);
        IfStatementNode(IfStatementNode&&) noexcept = default;
        ~IfStatementNode() override = default;

        IfStatementNode& operator=(const IfStatementNode& other);
        IfStatementNode& operator=(IfStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct ForStatementNode final : public StatementNode {
        std::unique_ptr<StatementNode>  init;
        std::unique_ptr<ExpressionNode> condition;
        std::unique_ptr<ExpressionNode> iteration;
        std::unique_ptr<StatementNode>  body;

        using StatementNode::StatementNode;
        ForStatementNode(const ForStatementNode& other);
        ForStatementNode(ForStatementNode&&) noexcept = default;
        ~ForStatementNode() override = default;

        ForStatementNode& operator=(const ForStatementNode& other);
        ForStatementNode& operator=(ForStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct WhileStatementNode final : public StatementNode {
        std::unique_ptr<ExpressionNode> condition;
        std::unique_ptr<StatementNode>  body;

        using StatementNode::StatementNode;
        WhileStatementNode(const WhileStatementNode& other);
        WhileStatementNode(WhileStatementNode&&) noexcept = default;
        ~WhileStatementNode() override = default;

        WhileStatementNode& operator=(const WhileStatementNode& other);
        WhileStatementNode& operator=(WhileStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct DoStatementNode final : public StatementNode {
        std::unique_ptr<StatementNode>  body;
        std::unique_ptr<ExpressionNode> condition;

        using StatementNode::StatementNode;
        DoStatementNode(const DoStatementNode& other);
        DoStatementNode(DoStatementNode&&) noexcept = default;
        ~DoStatementNode() override = default;

        DoStatementNode& operator=(const DoStatementNode& other);
        DoStatementNode& operator=(DoStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct SwitchStatementNode final : public StatementNode {
        std::unique_ptr<ExpressionNode>             condition;
        std::vector<std::unique_ptr<StatementNode>> cases;

        using StatementNode::StatementNode;
        SwitchStatementNode(const SwitchStatementNode& other);
        SwitchStatementNode(SwitchStatementNode&&) noexcept = default;
        ~SwitchStatementNode() override = default;

        SwitchStatementNode& operator=(const SwitchStatementNode& other);
        SwitchStatementNode& operator=(SwitchStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct CaseStatementNode final : public StatementNode {
        std::unique_ptr<ExpressionNode>             condition; // nullptr for "default"
        std::vector<std::unique_ptr<StatementNode>> body;

        using StatementNode::StatementNode;
        CaseStatementNode(const CaseStatementNode& other);
        CaseStatementNode(CaseStatementNode&&) noexcept = default;
        ~CaseStatementNode() override = default;

        CaseStatementNode& operator=(const CaseStatementNode& other);
        CaseStatementNode& operator=(CaseStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct ReturnStatementNode final : public StatementNode {
        std::unique_ptr<ExpressionNode> return_value;

        using StatementNode::StatementNode;
        ReturnStatementNode(const ReturnStatementNode& other);
        ReturnStatementNode(ReturnStatementNode&&) noexcept = default;
        ~ReturnStatementNode() override = default;

        ReturnStatementNode& operator=(const ReturnStatementNode& other);
        ReturnStatementNode& operator=(ReturnStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct BreakStatementNode final : public StatementNode {
        using StatementNode::StatementNode;
        BreakStatementNode(const BreakStatementNode& other);
        BreakStatementNode(BreakStatementNode&&) noexcept = default;
        ~BreakStatementNode() override = default;

        BreakStatementNode& operator=(const BreakStatementNode& other);
        BreakStatementNode& operator=(BreakStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct ContinueStatementNode final : public StatementNode {
        using StatementNode::StatementNode;
        ContinueStatementNode(const ContinueStatementNode& other);
        ContinueStatementNode(ContinueStatementNode&&) noexcept = default;
        ~ContinueStatementNode() override = default;

        ContinueStatementNode& operator=(const ContinueStatementNode& other);
        ContinueStatementNode& operator=(ContinueStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct DiscardStatementNode final : public StatementNode {
        using StatementNode::StatementNode;
        DiscardStatementNode(const DiscardStatementNode& other);
        DiscardStatementNode(DiscardStatementNode&&) noexcept = default;
        ~DiscardStatementNode() override = default;

        DiscardStatementNode& operator=(const DiscardStatementNode& other);
        DiscardStatementNode& operator=(DiscardStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct ExpressionStatementNode final : public StatementNode {
        std::unique_ptr<ExpressionNode> expr;

        using StatementNode::StatementNode;
        ExpressionStatementNode(const ExpressionStatementNode& other);
        ExpressionStatementNode(ExpressionStatementNode&&) noexcept = default;
        ~ExpressionStatementNode() override = default;

        ExpressionStatementNode& operator=(const ExpressionStatementNode& other);
        ExpressionStatementNode& operator=(ExpressionStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct NullStatementNode final : public StatementNode {
        using StatementNode::StatementNode;
        NullStatementNode(const NullStatementNode& other);
        NullStatementNode(NullStatementNode&&) noexcept = default;
        ~NullStatementNode() override = default;

        NullStatementNode& operator=(const NullStatementNode& other);
        NullStatementNode& operator=(NullStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct InitializerListExpressionNode final : public ExpressionNode {
        std::vector<std::unique_ptr<ExpressionNode>> elements;

        using ExpressionNode::ExpressionNode;
        InitializerListExpressionNode(const InitializerListExpressionNode& other);
        InitializerListExpressionNode(InitializerListExpressionNode&&) noexcept = default;
        ~InitializerListExpressionNode() override = default;

        InitializerListExpressionNode& operator=(const InitializerListExpressionNode& other);
        InitializerListExpressionNode& operator=(InitializerListExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct BinaryExpressionNode final : public ExpressionNode {
        TokenType                       op{};
        std::unique_ptr<ExpressionNode> left;
        std::unique_ptr<ExpressionNode> right;

        using ExpressionNode::ExpressionNode;
        BinaryExpressionNode(const BinaryExpressionNode& other);
        BinaryExpressionNode(BinaryExpressionNode&&) noexcept = default;
        ~BinaryExpressionNode() override = default;

        BinaryExpressionNode& operator=(const BinaryExpressionNode& other);
        BinaryExpressionNode& operator=(BinaryExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct UnaryExpressionNode final : public ExpressionNode {
        TokenType                       op{};
        bool                            is_postfix{ false };
        std::unique_ptr<ExpressionNode> operand;

        using ExpressionNode::ExpressionNode;
        UnaryExpressionNode(const UnaryExpressionNode& other);
        UnaryExpressionNode(UnaryExpressionNode&&) noexcept = default;
        ~UnaryExpressionNode() override = default;

        UnaryExpressionNode& operator=(const UnaryExpressionNode& other);
        UnaryExpressionNode& operator=(UnaryExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct TernaryExpressionNode final : public ExpressionNode {
        std::unique_ptr<ExpressionNode> condition;
        std::unique_ptr<ExpressionNode> true_expr;
        std::unique_ptr<ExpressionNode> false_expr;

        using ExpressionNode::ExpressionNode;
        TernaryExpressionNode(const TernaryExpressionNode& other);
        TernaryExpressionNode(TernaryExpressionNode&&) noexcept = default;
        ~TernaryExpressionNode() override = default;

        TernaryExpressionNode& operator=(const TernaryExpressionNode& other);
        TernaryExpressionNode& operator=(TernaryExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct CallExpressionNode final : public ExpressionNode {
        std::unique_ptr<ExpressionNode>              callee;
        std::vector<std::unique_ptr<ExpressionNode>> args;

        using ExpressionNode::ExpressionNode;
        CallExpressionNode(const CallExpressionNode& other);
        CallExpressionNode(CallExpressionNode&&) noexcept = default;
        ~CallExpressionNode() override = default;

        CallExpressionNode& operator=(const CallExpressionNode& other);
        CallExpressionNode& operator=(CallExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct IndexExpressionNode final : public ExpressionNode {
        std::unique_ptr<ExpressionNode> base;
        std::unique_ptr<ExpressionNode> index;

        using ExpressionNode::ExpressionNode;
        IndexExpressionNode(const IndexExpressionNode& other);
        IndexExpressionNode(IndexExpressionNode&&) noexcept = default;
        ~IndexExpressionNode() override = default;

        IndexExpressionNode& operator=(const IndexExpressionNode& other);
        IndexExpressionNode& operator=(IndexExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct VariableExpressionNode final : public ExpressionNode {
        enum class NodeType {
            kCommonVariable,
            kFunctionCallee,
            kBlockMember
        };

        Token           original_token;
        NodeType        node_type;
        std::string     name;
        SymbolReference linked_symbols{ std::monostate{} };

        using ExpressionNode::ExpressionNode;
        VariableExpressionNode(const VariableExpressionNode& other);
        VariableExpressionNode(VariableExpressionNode&&) noexcept = default;
        ~VariableExpressionNode() override = default;

        VariableExpressionNode& operator=(const VariableExpressionNode& other);
        VariableExpressionNode& operator=(VariableExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct RawExpressionNode final : public ExpressionNode {
        std::vector<Token> tokens;

        using ExpressionNode::ExpressionNode;
        RawExpressionNode(const RawExpressionNode& other);
        RawExpressionNode(RawExpressionNode&&) noexcept = default;
        ~RawExpressionNode() override = default;

        RawExpressionNode& operator=(const RawExpressionNode& other);
        RawExpressionNode& operator=(RawExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct MemberAccessExpressionNode final : public ExpressionNode {
        std::unique_ptr<ExpressionNode> object;
        std::unique_ptr<ExpressionNode> member;

        using ExpressionNode::ExpressionNode;
        MemberAccessExpressionNode(const MemberAccessExpressionNode& other);
        MemberAccessExpressionNode(MemberAccessExpressionNode&&) noexcept = default;
        ~MemberAccessExpressionNode() override = default;

        MemberAccessExpressionNode& operator=(const MemberAccessExpressionNode& other);
        MemberAccessExpressionNode& operator=(MemberAccessExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct TypeSpecifier {
        std::vector<Token>                                specifiers;
        std::vector<std::unique_ptr<ExpressionNode>>      template_args;
        std::vector<std::unique_ptr<ExpressionNode>>      array_sizes;
        std::vector<std::unique_ptr<LayoutQualifierNode>> layouts;
        std::vector<std::unique_ptr<SpirvIntrinsicNode>>  spirv_intrinsics;
        const SpirvIntrinsicNode*                         spirv_type{ nullptr };

        TypeSpecifier() = default;
        TypeSpecifier(const TypeSpecifier& other);
        TypeSpecifier(TypeSpecifier&&) noexcept = default;
        ~TypeSpecifier() = default;

        TypeSpecifier& operator=(const TypeSpecifier& other);
        TypeSpecifier& operator=(TypeSpecifier&&) noexcept = default;

        Token typename_token() const {
            return specifiers.empty() ? Token{} : specifiers.back();
        }

        SourceLocation begin_location() const {
            return specifiers.empty() ? SourceLocation{} : specifiers.front().location;
        }

        bool empty() const {
            return specifiers.empty();
        }

        bool has_keyword(std::string_view name) const;
    };

    struct VariableDeclarationNode final : public DeclarationNode {
        std::unique_ptr<ExpressionNode> init;
        TypeSpecifier                   type_spec;

        using DeclarationNode::DeclarationNode;
        VariableDeclarationNode(const VariableDeclarationNode& other);
        VariableDeclarationNode(VariableDeclarationNode&&) noexcept = default;
        ~VariableDeclarationNode() override = default;

        VariableDeclarationNode& operator=(const VariableDeclarationNode& other);
        VariableDeclarationNode& operator=(VariableDeclarationNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct DeclarationGroupNode final : public StatementNode {
        std::vector<std::unique_ptr<VariableDeclarationNode>> declarations;

        using StatementNode::StatementNode;
        DeclarationGroupNode(const DeclarationGroupNode& other);
        DeclarationGroupNode(DeclarationGroupNode&&) noexcept = default;
        ~DeclarationGroupNode() override = default;

        DeclarationGroupNode& operator=(const DeclarationGroupNode& other);
        DeclarationGroupNode& operator=(DeclarationGroupNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct FunctionDeclarationNode final : public DeclarationNode {
        std::vector<std::unique_ptr<VariableDeclarationNode>> params;
        std::unique_ptr<CompoundStatementNode>                body;
        TypeSpecifier                                         type_spec;

        using DeclarationNode::DeclarationNode;
        FunctionDeclarationNode(const FunctionDeclarationNode& other);
        FunctionDeclarationNode(FunctionDeclarationNode&&) noexcept = default;
        ~FunctionDeclarationNode() override = default;

        FunctionDeclarationNode& operator=(const FunctionDeclarationNode& other);
        FunctionDeclarationNode& operator=(FunctionDeclarationNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct InterfaceDeclarationNode final : public DeclarationNode {
        std::unique_ptr<CompoundStatementNode> body;
        std::unique_ptr<DeclarationGroupNode>  instances;
        TypeSpecifier                          type_spec;

        using DeclarationNode::DeclarationNode;
        InterfaceDeclarationNode(const InterfaceDeclarationNode& other);
        InterfaceDeclarationNode(InterfaceDeclarationNode&&) noexcept = default;
        ~InterfaceDeclarationNode() override = default;

        InterfaceDeclarationNode& operator=(const InterfaceDeclarationNode& other);
        InterfaceDeclarationNode& operator=(InterfaceDeclarationNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct StructDeclarationNode final : public DeclarationNode {
        std::unique_ptr<CompoundStatementNode> body;
        std::unique_ptr<DeclarationGroupNode>  instances;

        using DeclarationNode::DeclarationNode;
        StructDeclarationNode(const StructDeclarationNode& other);
        StructDeclarationNode(StructDeclarationNode&&) noexcept = default;
        ~StructDeclarationNode() override = default;

        StructDeclarationNode& operator=(const StructDeclarationNode& other);
        StructDeclarationNode& operator=(StructDeclarationNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };

    struct TranslationUnitNode final : public AstNode {
        std::vector<std::unique_ptr<StatementNode>> statements;
        std::vector<PreprocessorNode*> preprocessor_references;

        using AstNode::AstNode;
        TranslationUnitNode(const TranslationUnitNode& other);
        TranslationUnitNode(TranslationUnitNode&&) noexcept = default;
        ~TranslationUnitNode() override = default;

        TranslationUnitNode& operator=(const TranslationUnitNode& other);
        TranslationUnitNode& operator=(TranslationUnitNode&&) noexcept = default;

        AstNodeKind kind() const override;
        std::unique_ptr<AstNode> Clone() const override;
    };
}

#include "Ast.inl"
