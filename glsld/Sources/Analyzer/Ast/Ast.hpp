#pragma once

#include <cstddef>
#include <span>
#include <string_view>
#include <variant>

#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Base/Arena.hpp"

namespace glsld {
    struct ExpressionNode;
    struct LayoutQualifierNode;
    struct SpirvIntrinsicNode;

    struct TypeSpec {
        Arena* arena{ nullptr };
        ArenaVector<Token>                specifiers{ ArenaAllocator<Token>(arena) };
        ArenaVector<ExpressionNode*>      template_args{ ArenaAllocator<ExpressionNode*>(arena) };
        ArenaVector<ExpressionNode*>      array_sizes{ ArenaAllocator<ExpressionNode*>(arena) };
        ArenaVector<LayoutQualifierNode*> layouts{ ArenaAllocator<LayoutQualifierNode*>(arena) };
        ArenaVector<SpirvIntrinsicNode*>  spirv_intrinsics{ ArenaAllocator<SpirvIntrinsicNode*>(arena) };
        const SpirvIntrinsicNode*         spirv_type{ nullptr };

        TypeSpec(Arena* arena);
        TypeSpec(const TypeSpec& other);
        TypeSpec(TypeSpec&&) noexcept = default;
        ~TypeSpec() = default;

        TypeSpec& operator=(const TypeSpec& other);
        TypeSpec& operator=(TypeSpec&&) noexcept = default;

        Token typename_token() const;
        SourceLocation begin_location() const;
        bool empty() const;
        bool has_keyword(std::string_view name) const;
    };

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
        kCastExpression,        // (int)value;
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
        Arena*         arena{ nullptr };
        SourceLocation begin;
        SourceLocation end;
        Scope*         located_scope{ nullptr };
        Scope*         internal_scope{ nullptr };

        AstNode(Arena* arena, Scope* scope);
        AstNode(const AstNode& other);
        AstNode(AstNode&&) noexcept = default;
        virtual ~AstNode() = default;

        AstNode& operator=(const AstNode& other);
        AstNode& operator=(AstNode&&) noexcept = default;

        virtual AstNodeKind kind() const = 0;
        virtual AstNode* Clone() const = 0;

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
        QualifierArgumentKind               arg_kind{ QualifierArgumentKind::kUnknown };
        Token                               token;
        ExpressionNode*                     rhs_expr{ nullptr };
        ArenaVector<QualifierArgumentNode*> children{ ArenaAllocator<QualifierArgumentNode*>(arena) };

        using AstNode::AstNode;
        QualifierArgumentNode(const QualifierArgumentNode& other);
        QualifierArgumentNode(QualifierArgumentNode&&) noexcept = default;
        ~QualifierArgumentNode() override = default;

        QualifierArgumentNode& operator=(const QualifierArgumentNode& other);
        QualifierArgumentNode& operator=(QualifierArgumentNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct LayoutQualifierNode : public AstNode {
        ArenaVector<Token>                  raw_tokens{ ArenaAllocator<Token>(arena) }; // 不包含外层括号
        ArenaVector<QualifierArgumentNode*> params{ ArenaAllocator<QualifierArgumentNode*>(arena) };

        using AstNode::AstNode;
        LayoutQualifierNode(const LayoutQualifierNode& other);
        LayoutQualifierNode(LayoutQualifierNode&&) noexcept = default;
        ~LayoutQualifierNode() override = default;

        LayoutQualifierNode& operator=(const LayoutQualifierNode& other);
        LayoutQualifierNode& operator=(LayoutQualifierNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
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
        AstNode* Clone() const override;
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
        Token           namespace_;
        Token           name;
        ExpressionNode* argument{ nullptr };

        using AstNode::AstNode;
        AttributeNode(const AttributeNode& other);
        AttributeNode(AttributeNode&&) noexcept = default;
        ~AttributeNode() override = default;

        AttributeNode& operator=(const AttributeNode& other);
        AttributeNode& operator=(AttributeNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct StatementNode : public AstNode {
        ArenaVector<AttributeNode*> attributes{ ArenaAllocator<AttributeNode*>(arena) };

        using AstNode::AstNode;
        StatementNode(Arena* arena, Scope* scope);
        StatementNode(const StatementNode& other);
        StatementNode(StatementNode&&) noexcept = default;
        ~StatementNode() override = default;

        StatementNode& operator=(const StatementNode& other);
        StatementNode& operator=(StatementNode&&) noexcept = default;
    };

    struct PreprocessorNode final : public StatementNode {
        std::string_view              directive;
        ArenaVector<Token>            tokens{ ArenaAllocator<Token>(arena) };
        ArenaVector<std::string_view> params{ ArenaAllocator<std::string_view>(arena) };
        ArenaVector<StatementNode*>   body{ ArenaAllocator<StatementNode*>(arena) };
        const SymbolInfo*             symbol{ nullptr };

        using StatementNode::StatementNode;
        PreprocessorNode(const PreprocessorNode& other);
        PreprocessorNode(PreprocessorNode&&) noexcept = default;
        ~PreprocessorNode() override = default;

        PreprocessorNode& operator=(const PreprocessorNode& other);
        PreprocessorNode& operator=(PreprocessorNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
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
        ArenaVector<StatementNode*> children{ ArenaAllocator<StatementNode*>(arena) };

        using StatementNode::StatementNode;
        CompoundStatementNode(const CompoundStatementNode& other);
        CompoundStatementNode(CompoundStatementNode&&) noexcept = default;
        ~CompoundStatementNode() override = default;

        CompoundStatementNode& operator=(const CompoundStatementNode& other);
        CompoundStatementNode& operator=(CompoundStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct IfStatementNode final : public StatementNode {
        ExpressionNode* condition{ nullptr };
        StatementNode*  then_branch{ nullptr };
        StatementNode*  else_branch{ nullptr };

        using StatementNode::StatementNode;
        IfStatementNode(const IfStatementNode& other);
        IfStatementNode(IfStatementNode&&) noexcept = default;
        ~IfStatementNode() override = default;

        IfStatementNode& operator=(const IfStatementNode& other);
        IfStatementNode& operator=(IfStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct ForStatementNode final : public StatementNode {
        StatementNode*  init{ nullptr };
        ExpressionNode* condition{ nullptr };
        ExpressionNode* iteration{ nullptr };
        StatementNode*  body{ nullptr };

        using StatementNode::StatementNode;
        ForStatementNode(const ForStatementNode& other);
        ForStatementNode(ForStatementNode&&) noexcept = default;
        ~ForStatementNode() override = default;

        ForStatementNode& operator=(const ForStatementNode& other);
        ForStatementNode& operator=(ForStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct WhileStatementNode final : public StatementNode {
        ExpressionNode* condition{ nullptr };
        StatementNode*  body{ nullptr };

        using StatementNode::StatementNode;
        WhileStatementNode(const WhileStatementNode& other);
        WhileStatementNode(WhileStatementNode&&) noexcept = default;
        ~WhileStatementNode() override = default;

        WhileStatementNode& operator=(const WhileStatementNode& other);
        WhileStatementNode& operator=(WhileStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct DoStatementNode final : public StatementNode {
        StatementNode*  body{ nullptr };
        ExpressionNode* condition{ nullptr };

        using StatementNode::StatementNode;
        DoStatementNode(const DoStatementNode& other);
        DoStatementNode(DoStatementNode&&) noexcept = default;
        ~DoStatementNode() override = default;

        DoStatementNode& operator=(const DoStatementNode& other);
        DoStatementNode& operator=(DoStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct SwitchStatementNode final : public StatementNode {
        ExpressionNode*             condition{ nullptr };
        ArenaVector<StatementNode*> cases{ ArenaAllocator<StatementNode*>(arena) };

        using StatementNode::StatementNode;
        SwitchStatementNode(const SwitchStatementNode& other);
        SwitchStatementNode(SwitchStatementNode&&) noexcept = default;
        ~SwitchStatementNode() override = default;

        SwitchStatementNode& operator=(const SwitchStatementNode& other);
        SwitchStatementNode& operator=(SwitchStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct CaseStatementNode final : public StatementNode {
        ExpressionNode*             condition{ nullptr }; // nullptr for "default"
        ArenaVector<StatementNode*> body{ ArenaAllocator<StatementNode*>(arena) };

        using StatementNode::StatementNode;
        CaseStatementNode(const CaseStatementNode& other);
        CaseStatementNode(CaseStatementNode&&) noexcept = default;
        ~CaseStatementNode() override = default;

        CaseStatementNode& operator=(const CaseStatementNode& other);
        CaseStatementNode& operator=(CaseStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct ReturnStatementNode final : public StatementNode {
        ExpressionNode* return_value{ nullptr };

        using StatementNode::StatementNode;
        ReturnStatementNode(const ReturnStatementNode& other);
        ReturnStatementNode(ReturnStatementNode&&) noexcept = default;
        ~ReturnStatementNode() override = default;

        ReturnStatementNode& operator=(const ReturnStatementNode& other);
        ReturnStatementNode& operator=(ReturnStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct BreakStatementNode final : public StatementNode {
        using StatementNode::StatementNode;
        BreakStatementNode(const BreakStatementNode& other);
        BreakStatementNode(BreakStatementNode&&) noexcept = default;
        ~BreakStatementNode() override = default;

        BreakStatementNode& operator=(const BreakStatementNode& other);
        BreakStatementNode& operator=(BreakStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct ContinueStatementNode final : public StatementNode {
        using StatementNode::StatementNode;
        ContinueStatementNode(const ContinueStatementNode& other);
        ContinueStatementNode(ContinueStatementNode&&) noexcept = default;
        ~ContinueStatementNode() override = default;

        ContinueStatementNode& operator=(const ContinueStatementNode& other);
        ContinueStatementNode& operator=(ContinueStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct DiscardStatementNode final : public StatementNode {
        using StatementNode::StatementNode;
        DiscardStatementNode(const DiscardStatementNode& other);
        DiscardStatementNode(DiscardStatementNode&&) noexcept = default;
        ~DiscardStatementNode() override = default;

        DiscardStatementNode& operator=(const DiscardStatementNode& other);
        DiscardStatementNode& operator=(DiscardStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct ExpressionStatementNode final : public StatementNode {
        ExpressionNode* expr{ nullptr };

        using StatementNode::StatementNode;
        ExpressionStatementNode(const ExpressionStatementNode& other);
        ExpressionStatementNode(ExpressionStatementNode&&) noexcept = default;
        ~ExpressionStatementNode() override = default;

        ExpressionStatementNode& operator=(const ExpressionStatementNode& other);
        ExpressionStatementNode& operator=(ExpressionStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct NullStatementNode final : public StatementNode {
        using StatementNode::StatementNode;
        NullStatementNode(const NullStatementNode& other);
        NullStatementNode(NullStatementNode&&) noexcept = default;
        ~NullStatementNode() override = default;

        NullStatementNode& operator=(const NullStatementNode& other);
        NullStatementNode& operator=(NullStatementNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct InitializerListExpressionNode final : public ExpressionNode {
        ArenaVector<ExpressionNode*> elements{ ArenaAllocator<ExpressionNode*>(arena) };

        using ExpressionNode::ExpressionNode;
        InitializerListExpressionNode(const InitializerListExpressionNode& other);
        InitializerListExpressionNode(InitializerListExpressionNode&&) noexcept = default;
        ~InitializerListExpressionNode() override = default;

        InitializerListExpressionNode& operator=(const InitializerListExpressionNode& other);
        InitializerListExpressionNode& operator=(InitializerListExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct CastExpressionNode final : public ExpressionNode {
        TypeSpec        target_type{ arena };
        ExpressionNode* operand{ nullptr };

        using ExpressionNode::ExpressionNode;
        CastExpressionNode(const CastExpressionNode& other);
        CastExpressionNode(CastExpressionNode&&) noexcept = default;
        ~CastExpressionNode() = default;

        CastExpressionNode& operator=(const CastExpressionNode& other);
        CastExpressionNode& operator=(CastExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct BinaryExpressionNode final : public ExpressionNode {
        TokenType       op{};
        ExpressionNode* left{ nullptr };
        ExpressionNode* right{ nullptr };

        using ExpressionNode::ExpressionNode;
        BinaryExpressionNode(const BinaryExpressionNode& other);
        BinaryExpressionNode(BinaryExpressionNode&&) noexcept = default;
        ~BinaryExpressionNode() override = default;

        BinaryExpressionNode& operator=(const BinaryExpressionNode& other);
        BinaryExpressionNode& operator=(BinaryExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct UnaryExpressionNode final : public ExpressionNode {
        TokenType       op{};
        bool            is_postfix{ false };
        ExpressionNode* operand{ nullptr };

        using ExpressionNode::ExpressionNode;
        UnaryExpressionNode(const UnaryExpressionNode& other);
        UnaryExpressionNode(UnaryExpressionNode&&) noexcept = default;
        ~UnaryExpressionNode() override = default;

        UnaryExpressionNode& operator=(const UnaryExpressionNode& other);
        UnaryExpressionNode& operator=(UnaryExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct TernaryExpressionNode final : public ExpressionNode {
        ExpressionNode* condition{ nullptr };
        ExpressionNode* true_expr{ nullptr };
        ExpressionNode* false_expr{ nullptr };

        using ExpressionNode::ExpressionNode;
        TernaryExpressionNode(const TernaryExpressionNode& other);
        TernaryExpressionNode(TernaryExpressionNode&&) noexcept = default;
        ~TernaryExpressionNode() override = default;

        TernaryExpressionNode& operator=(const TernaryExpressionNode& other);
        TernaryExpressionNode& operator=(TernaryExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct CallExpressionNode final : public ExpressionNode {
        ExpressionNode*              callee{ nullptr };
        ArenaVector<ExpressionNode*> args{ ArenaAllocator<ExpressionNode*>(arena) };

        using ExpressionNode::ExpressionNode;
        CallExpressionNode(const CallExpressionNode& other);
        CallExpressionNode(CallExpressionNode&&) noexcept = default;
        ~CallExpressionNode() override = default;

        CallExpressionNode& operator=(const CallExpressionNode& other);
        CallExpressionNode& operator=(CallExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct IndexExpressionNode final : public ExpressionNode {
        ExpressionNode* base{ nullptr };
        ExpressionNode* index{ nullptr };

        using ExpressionNode::ExpressionNode;
        IndexExpressionNode(const IndexExpressionNode& other);
        IndexExpressionNode(IndexExpressionNode&&) noexcept = default;
        ~IndexExpressionNode() override = default;

        IndexExpressionNode& operator=(const IndexExpressionNode& other);
        IndexExpressionNode& operator=(IndexExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct VariableExpressionNode final : public ExpressionNode {
        enum class NodeType {
            kCommonVariable,
            kFunctionCallee,
            kBlockMember
        };

        Token               original_token;
        NodeType            node_type;
        std::string_view    name;
        SymbolReferenceView linked_symbols{ std::monostate{} };

        using ExpressionNode::ExpressionNode;
        VariableExpressionNode(const VariableExpressionNode& other);
        VariableExpressionNode(VariableExpressionNode&&) noexcept = default;
        ~VariableExpressionNode() override = default;

        VariableExpressionNode& operator=(const VariableExpressionNode& other);
        VariableExpressionNode& operator=(VariableExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct RawExpressionNode final : public ExpressionNode {
        ArenaVector<Token> tokens{ ArenaAllocator<Token>(arena) };

        using ExpressionNode::ExpressionNode;
        RawExpressionNode(const RawExpressionNode& other);
        RawExpressionNode(RawExpressionNode&&) noexcept = default;
        ~RawExpressionNode() override = default;

        RawExpressionNode& operator=(const RawExpressionNode& other);
        RawExpressionNode& operator=(RawExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct MemberAccessExpressionNode final : public ExpressionNode {
        ExpressionNode* object{ nullptr };
        ExpressionNode* member{ nullptr };

        using ExpressionNode::ExpressionNode;
        MemberAccessExpressionNode(const MemberAccessExpressionNode& other);
        MemberAccessExpressionNode(MemberAccessExpressionNode&&) noexcept = default;
        ~MemberAccessExpressionNode() override = default;

        MemberAccessExpressionNode& operator=(const MemberAccessExpressionNode& other);
        MemberAccessExpressionNode& operator=(MemberAccessExpressionNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct VariableDeclarationNode final : public DeclarationNode {
        ExpressionNode* init{ nullptr };
        TypeSpec        type_spec{ arena };
        bool            is_variadic{ false };

        using DeclarationNode::DeclarationNode;
        VariableDeclarationNode(const VariableDeclarationNode& other);
        VariableDeclarationNode(VariableDeclarationNode&&) noexcept = default;
        ~VariableDeclarationNode() override = default;

        VariableDeclarationNode& operator=(const VariableDeclarationNode& other);
        VariableDeclarationNode& operator=(VariableDeclarationNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct DeclarationGroupNode final : public StatementNode {
        ArenaVector<VariableDeclarationNode*> declarations{ ArenaAllocator<VariableDeclarationNode*>(arena) };

        using StatementNode::StatementNode;
        DeclarationGroupNode(const DeclarationGroupNode& other);
        DeclarationGroupNode(DeclarationGroupNode&&) noexcept = default;
        ~DeclarationGroupNode() override = default;

        DeclarationGroupNode& operator=(const DeclarationGroupNode& other);
        DeclarationGroupNode& operator=(DeclarationGroupNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct FunctionDeclarationNode final : public DeclarationNode {
        ArenaVector<VariableDeclarationNode*> params{ ArenaAllocator<VariableDeclarationNode*>(arena) };
        CompoundStatementNode*                body{ nullptr };
        TypeSpec                              type_spec{ arena };

        using DeclarationNode::DeclarationNode;
        FunctionDeclarationNode(const FunctionDeclarationNode& other);
        FunctionDeclarationNode(FunctionDeclarationNode&&) noexcept = default;
        ~FunctionDeclarationNode() override = default;

        FunctionDeclarationNode& operator=(const FunctionDeclarationNode& other);
        FunctionDeclarationNode& operator=(FunctionDeclarationNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct InterfaceDeclarationNode final : public DeclarationNode {
        CompoundStatementNode* body{ nullptr };
        DeclarationGroupNode*  instances{ nullptr };
        TypeSpec               type_spec{ arena };

        using DeclarationNode::DeclarationNode;
        InterfaceDeclarationNode(const InterfaceDeclarationNode& other);
        InterfaceDeclarationNode(InterfaceDeclarationNode&&) noexcept = default;
        ~InterfaceDeclarationNode() override = default;

        InterfaceDeclarationNode& operator=(const InterfaceDeclarationNode& other);
        InterfaceDeclarationNode& operator=(InterfaceDeclarationNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct StructDeclarationNode final : public DeclarationNode {
        CompoundStatementNode* body{ nullptr };
        DeclarationGroupNode*  instances{ nullptr };

        using DeclarationNode::DeclarationNode;
        StructDeclarationNode(const StructDeclarationNode& other);
        StructDeclarationNode(StructDeclarationNode&&) noexcept = default;
        ~StructDeclarationNode() override = default;

        StructDeclarationNode& operator=(const StructDeclarationNode& other);
        StructDeclarationNode& operator=(StructDeclarationNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };

    struct TranslationUnitNode final : public AstNode {
        ArenaVector<StatementNode*>    statements{ ArenaAllocator<StatementNode*>(arena) };
        ArenaVector<PreprocessorNode*> pprefs{ ArenaAllocator<PreprocessorNode*>(arena) };

        using AstNode::AstNode;
        TranslationUnitNode(const TranslationUnitNode& other);
        TranslationUnitNode(TranslationUnitNode&&) noexcept = default;
        ~TranslationUnitNode() override = default;

        TranslationUnitNode& operator=(const TranslationUnitNode& other);
        TranslationUnitNode& operator=(TranslationUnitNode&&) noexcept = default;

        AstNodeKind kind() const override;
        AstNode* Clone() const override;
    };
}

#include "Ast.inl"
