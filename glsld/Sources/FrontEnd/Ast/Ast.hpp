#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <variant>

#include <Base/Arena.hpp>
#include <FrontEnd/Syntax/Symbol.hpp>
#include <FrontEnd/Syntax/Token.hpp>

namespace glsld {
    using ResolvedNamePart = std::pair<Token, const SymbolInfo*>;

    struct ExpressionNode;
    struct LayoutQualifierNode;
    struct SpirvIntrinsicNode;
    struct FunctionTypeSpec;
    struct TypeSpec {
        Arena*                            arena{ nullptr };
        ArenaVector<Token>                specifiers{ ArenaAllocator<Token>(arena) };
        ArenaVector<ExpressionNode*>      template_args{ ArenaAllocator<ExpressionNode*>(arena) };
        ArenaVector<ExpressionNode*>      array_sizes{ ArenaAllocator<ExpressionNode*>(arena) };
        ArenaVector<LayoutQualifierNode*> layouts{ ArenaAllocator<LayoutQualifierNode*>(arena) };
        ArenaVector<SpirvIntrinsicNode*>  spirv_intrinsics{ ArenaAllocator<SpirvIntrinsicNode*>(arena) };
        FunctionTypeSpec*                 function_type{ nullptr };
        const SpirvIntrinsicNode*         spirv_type{ nullptr };
        const SymbolInfo*                 named_type_symbol{ nullptr };
        std::string_view                  written_typename;
        std::span<const ResolvedNamePart> name_prefixes;
        SourceLocation                    typename_begin;

        TypeSpec(Arena* arena);

        Token typename_token() const;
        SourceLocation begin_location() const;
        bool empty() const;
        bool has_keyword(std::string_view name) const;
    };

    struct FunctionTypeSpec {
        TypeSpec              return_type;
        ArenaVector<TypeSpec> param_types;

        explicit FunctionTypeSpec(Arena* arena);

        FunctionTypeSpec& operator=(const FunctionTypeSpec& other);
        FunctionTypeSpec& operator=(FunctionTypeSpec&&) noexcept = default;
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
        kDeclaration,
        kTypeAliasDeclaration,
        kNamespaceDeclaration,
        kFunctionDeclaration,
        kVariableDeclaration,
        kInterfaceDeclaration,
        kStructDeclaration,
        kEnumeratorDeclaration,
        kEnumDeclaration,

        // Statements
        kStatement,
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
        kExpression,
        kInitializerListExpression,
        kCastExpression,        // (int)value;
        kBinaryExpression,
        kUnaryExpression,
        kTernaryExpression,
        kCallExpression,
        kIndexExpression,
        kVariableExpression,    // 变量引用
        kRawExpression,
        kMemberAccessExpression // struct.member
    };

    struct AstNode {
        Arena*         arena{ nullptr };
        SourceLocation begin;
        SourceLocation end;
        Scope*         located_scope{ nullptr };
        Scope*         internal_scope{ nullptr };

        AstNode(Arena* arena, Scope* scope);
        AstNode(const AstNode&) = delete;
        AstNode(AstNode&&)      = delete;
        virtual ~AstNode()      = default;

        AstNode& operator=(const AstNode&) = delete;
        AstNode& operator=(AstNode&&)      = delete;

        virtual AstNodeKind kind() const = 0;
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
        AstNodeKind kind() const override;
    };

    struct LayoutQualifierNode : public AstNode {
        ArenaVector<Token>                  raw_tokens{ ArenaAllocator<Token>(arena) }; // 不包含外层括号
        ArenaVector<QualifierArgumentNode*> params{ ArenaAllocator<QualifierArgumentNode*>(arena) };

        using AstNode::AstNode;
        AstNodeKind kind() const override;
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
        AstNodeKind kind() const override;
    };

    struct ExpressionNode : public AstNode {
        TypeInfo evaluated_type;

        using AstNode::AstNode;
        AstNodeKind kind() const override;
    };

    struct AttributeNode final : public AstNode {
        Token           namespace_;
        Token           name;
        ExpressionNode* argument{ nullptr };

        using AstNode::AstNode;
        AstNodeKind kind() const override;
    };

    struct StatementNode : public AstNode {
        ArenaVector<AttributeNode*> attributes{ ArenaAllocator<AttributeNode*>(arena) };

        using AstNode::AstNode;
        AstNodeKind kind() const override;
    };

    struct PreprocessorNode final : public StatementNode {
        std::string_view              directive;
        ArenaVector<Token>            tokens{ ArenaAllocator<Token>(arena) };
        ArenaVector<std::string_view> params{ ArenaAllocator<std::string_view>(arena) };
        ArenaVector<StatementNode*>   body{ ArenaAllocator<StatementNode*>(arena) };
        const SymbolInfo*             symbol{ nullptr };
        bool                          is_function{ false };

        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct CompoundStatementNode final : public StatementNode {
        ArenaVector<StatementNode*> children{ ArenaAllocator<StatementNode*>(arena) };

        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct IfStatementNode final : public StatementNode {
        ExpressionNode* condition{ nullptr };
        StatementNode*  then_branch{ nullptr };
        StatementNode*  else_branch{ nullptr };

        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct ForStatementNode final : public StatementNode {
        StatementNode*  init{ nullptr };
        ExpressionNode* condition{ nullptr };
        ExpressionNode* iteration{ nullptr };
        StatementNode*  body{ nullptr };

        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct WhileStatementNode final : public StatementNode {
        ExpressionNode* condition{ nullptr };
        StatementNode*  body{ nullptr };

        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct DoStatementNode final : public StatementNode {
        StatementNode*  body{ nullptr };
        ExpressionNode* condition{ nullptr };

        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct SwitchStatementNode final : public StatementNode {
        ExpressionNode*             condition{ nullptr };
        ArenaVector<StatementNode*> cases{ ArenaAllocator<StatementNode*>(arena) };

        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct CaseStatementNode final : public StatementNode {
        ExpressionNode*             condition{ nullptr }; // nullptr for "default"
        ArenaVector<StatementNode*> body{ ArenaAllocator<StatementNode*>(arena) };

        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct ReturnStatementNode final : public StatementNode {
        ExpressionNode* return_value{ nullptr };

        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct BreakStatementNode final : public StatementNode {
        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct ContinueStatementNode final : public StatementNode {
        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct DiscardStatementNode final : public StatementNode {
        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct ExpressionStatementNode final : public StatementNode {
        ExpressionNode* expr{ nullptr };

        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct NullStatementNode final : public StatementNode {
        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct InitializerListExpressionNode final : public ExpressionNode {
        ArenaVector<ExpressionNode*> elements{ ArenaAllocator<ExpressionNode*>(arena) };

        using ExpressionNode::ExpressionNode;
        AstNodeKind kind() const override;
    };

    struct CastExpressionNode final : public ExpressionNode {
        TypeSpec        target_type{ arena };
        ExpressionNode* operand{ nullptr };

        using ExpressionNode::ExpressionNode;
        AstNodeKind kind() const override;
    };

    struct BinaryExpressionNode final : public ExpressionNode {
        TokenType       op{};
        ExpressionNode* left{ nullptr };
        ExpressionNode* right{ nullptr };

        using ExpressionNode::ExpressionNode;
        AstNodeKind kind() const override;
    };

    struct UnaryExpressionNode final : public ExpressionNode {
        TokenType       op{};
        bool            is_postfix{ false };
        ExpressionNode* operand{ nullptr };

        using ExpressionNode::ExpressionNode;
        AstNodeKind kind() const override;
    };

    struct TernaryExpressionNode final : public ExpressionNode {
        ExpressionNode* condition{ nullptr };
        ExpressionNode* true_expr{ nullptr };
        ExpressionNode* false_expr{ nullptr };

        using ExpressionNode::ExpressionNode;
        AstNodeKind kind() const override;
    };

    struct CallExpressionNode final : public ExpressionNode {
        ExpressionNode*              callee{ nullptr };
        ArenaVector<ExpressionNode*> args{ ArenaAllocator<ExpressionNode*>(arena) };

        using ExpressionNode::ExpressionNode;
        AstNodeKind kind() const override;
    };

    struct IndexExpressionNode final : public ExpressionNode {
        ExpressionNode* base{ nullptr };
        ExpressionNode* index{ nullptr };

        using ExpressionNode::ExpressionNode;
        AstNodeKind kind() const override;
    };

    struct VariableExpressionNode final : public ExpressionNode {
        enum class NodeType {
            kCommonVariable,
            kFunctionCallee,
            kBlockMember
        };

        Token                             original_token;
        NodeType                          node_type;
        std::string_view                  name;
        SymbolReferenceView               linked_symbols{ std::monostate{} };
        const SymbolInfo*                 named_type_symbol{ nullptr };
        QualifiedScope                    qualifier_scope;
        std::span<const ResolvedNamePart> name_prefixes;
        std::string_view                  written_name;

        using ExpressionNode::ExpressionNode;
        AstNodeKind kind() const override;
    };

    struct RawExpressionNode final : public ExpressionNode {
        ArenaVector<Token> tokens{ ArenaAllocator<Token>(arena) };

        using ExpressionNode::ExpressionNode;
        AstNodeKind kind() const override;
    };

    struct MemberAccessExpressionNode final : public ExpressionNode {
        ExpressionNode* object{ nullptr };
        ExpressionNode* member{ nullptr };

        using ExpressionNode::ExpressionNode;
        AstNodeKind kind() const override;
    };

    struct DeclarationNode : public StatementNode {
        SymbolInfo* declared_symbol{ nullptr };

        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct TypeAliasDeclarationNode final : public DeclarationNode {
        Token    name;
        TypeSpec type_spec{ arena };

        using DeclarationNode::DeclarationNode;
        AstNodeKind kind() const override;
    };

    struct NamespaceDeclarationNode final : public DeclarationNode {
        ArenaVector<Token>            names{ ArenaAllocator<Token>(arena) };
        ArenaVector<ResolvedNamePart> resolved_names{ ArenaAllocator<ResolvedNamePart>(arena) };
        ArenaVector<StatementNode*>   children{ ArenaAllocator<StatementNode*>(arena) };
        std::string_view              alias_target; // 空：namespace X { ... }  非空：namespace X = Y::Z;

        using DeclarationNode::DeclarationNode;
        AstNodeKind kind() const override;

        const Token* name() const;
    };

    struct VariableDeclarationNode final : public DeclarationNode {
        ExpressionNode* init{ nullptr };
        TypeSpec        type_spec{ arena };
        bool            is_variadic{ false };

        using DeclarationNode::DeclarationNode;
        AstNodeKind kind() const override;
    };

    struct DeclarationGroupNode final : public StatementNode {
        TypeSpec                              type_spec{ arena };
        ArenaVector<VariableDeclarationNode*> declarations{ ArenaAllocator<VariableDeclarationNode*>(arena) };

        using StatementNode::StatementNode;
        AstNodeKind kind() const override;
    };

    struct FunctionDeclarationNode final : public DeclarationNode {
        ArenaVector<VariableDeclarationNode*> params{ ArenaAllocator<VariableDeclarationNode*>(arena) };
        CompoundStatementNode*                body{ nullptr };
        TypeSpec                              type_spec{ arena };

        using DeclarationNode::DeclarationNode;
        AstNodeKind kind() const override;
    };

    struct InterfaceDeclarationNode final : public DeclarationNode {
        CompoundStatementNode* body{ nullptr };
        DeclarationGroupNode*  instances{ nullptr };
        TypeSpec               type_spec{ arena };

        using DeclarationNode::DeclarationNode;
        AstNodeKind kind() const override;
    };

    struct StructDeclarationNode final : public DeclarationNode {
        CompoundStatementNode* body{ nullptr };
        DeclarationGroupNode*  instances{ nullptr };

        using DeclarationNode::DeclarationNode;
        AstNodeKind kind() const override;
    };

    using EnumInteger = std::variant<std::int64_t, std::uint64_t>;
    struct EnumeratorDeclarationNode final : DeclarationNode {
        Token                      name;
        ExpressionNode*            init{ nullptr };
        std::optional<EnumInteger> value;

        using DeclarationNode::DeclarationNode;
        AstNodeKind kind() const override;
    };

    struct EnumDeclarationNode final : DeclarationNode {
        Token                 name;
        TypeSpec              underlying_type{ arena };
        DeclarationGroupNode* instances{ nullptr };

        ArenaVector<EnumeratorDeclarationNode*> enumerators{
            ArenaAllocator<EnumeratorDeclarationNode*>(arena)
        };

        using DeclarationNode::DeclarationNode;
        AstNodeKind kind() const override;
    };

    struct TranslationUnitNode final : public AstNode {
        ArenaVector<StatementNode*>    statements{ ArenaAllocator<StatementNode*>(arena) };
        ArenaVector<PreprocessorNode*> pprefs{ ArenaAllocator<PreprocessorNode*>(arena) };

        using AstNode::AstNode;
        AstNodeKind kind() const override;
    };
}

#include "Ast.inl"
