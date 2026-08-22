#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <stack>
#include <string_view>
#include <vector>

#include <ankerl/unordered_dense.h>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Base/FileSystem/IncludeLoader.hpp"
#include "Base/FileSystem/Source.hpp"
#include "Base/Arena.hpp"

namespace glsld {
    class Parser {
    public:
        Parser(Document& document,
               SourceTable& source_table,
               const SourceFile* source_file,
               std::vector<Token> raw_tokens,
               IncludeLoader& include_loader,
               IncludeDirectoryHandle include_dirs,
               int version_replica,
               VersionPointer version_pointer);

    private:
        enum class Precedence : int {
            kLowest = 0,
            kComma,          // ,
            kAssignment,     // = += -= *= /= %= <<= >>= &= ^= |=
            kTernary,        // ?:
            kLogicalOr,      // ||
            kLogicalXor,     // ^^
            kLogicalAnd,     // &&
            kBitwiseOr,      // |
            kBitwiseXor,     // ^
            kBitwiseAnd,     // &
            kEquality,       // == !=
            kRelational,     // < > <= >=
            kShift,          // << >>
            kAdditive,       // + -
            kMultiplicative, // * / %
            kTypecast,       // (type)expr
            kPrefix,         // ! ~ - + ++ --
            kPostfix,        // . [ ] ( ) ++ --
            kHighest
        };

        Precedence GetInfixPrecedence(TokenType type);
        bool IsRightAssociative(TokenType type);

        void Parse(SourceTable& source_table,
                   IncludeLoader& include_loader,
                   IncludeDirectoryHandle include_dirs);

        template <typename NodeType, typename... Types>
        NodeType* MakeNode(Types&&... args);

        TranslationUnitNode* ParserMainTask();
        StatementNode* ParseStatement();
        PreprocessorNode* ParsePreprocessor();

        PreprocessorNode* ParseDefine(
            PreprocessorNode* node,
            std::string_view target_file,
            std::uint32_t directive_physical_line);

        ArenaVector<StatementNode*> ParseMacroBody(std::span<const Token> body_tokens, const SymbolInfo* host_symbol);
        CompoundStatementNode* ParseScope(const SymbolInfo* host_symbol = nullptr, ScopeKind kind = ScopeKind::kCommon);
        ArenaVector<AttributeNode*> ParseAttributeList();
        StatementNode* ParseCodeStatement();
        FunctionDeclarationNode* ParseFunction(TypeSpec type_spec);
        ArenaVector<VariableDeclarationNode*> ParseParameterList();

        TypeSpec ParseTypeSpec();
        ArenaVector<Token> CaptureBalancedTokens(TokenType open, TokenType close);
        QualifierArgumentNode* ParseQualifierArguments(std::span<const Token> tokens);
        LayoutQualifierNode* ParseLayoutQualifier();
        SpirvIntrinsicNode* ParseSpirvIntrinsics();
        ExpressionNode* ParseTemplateArgument();

        bool TryParseLayoutQualifier(TypeSpec& type_spec);
        bool TryParseSpirvIntrinsics(TypeSpec& type_spec);

        DeclarationGroupNode* ParseVariableDeclarationList(TypeSpec type_spec);
        ExpressionStatementNode* ParseExpressionStatement();

        bool IsCastExpression();

        ExpressionNode* ParsePrefixExpression();
        CastExpressionNode* ParseCastExpression();
        RawExpressionNode* ParseLiteral();
        VariableExpressionNode* ParseVariableReference();
        UnaryExpressionNode* ParsePrefixUnary();
        InitializerListExpressionNode* ParseInitializerList();

        ExpressionNode* ParseInfixExpression(ExpressionNode* left, TokenType op_type, Precedence precedence);
        MemberAccessExpressionNode* ParseMemberAccess(ExpressionNode* object);
        IndexExpressionNode* ParseArrayIndex(ExpressionNode* base);
        CallExpressionNode* ParseFunctionCall(ExpressionNode* callee);
        TernaryExpressionNode* ParseTernary(ExpressionNode* condition);
        UnaryExpressionNode* ParsePostfixUnary(ExpressionNode* operand, TokenType op_type);
        BinaryExpressionNode* ParseStandardBinary(ExpressionNode* left, TokenType op_type, Precedence precedence);
        ExpressionNode* ParseExpression(Precedence min_prec);

        DeclarationNode* ParseBlockBody(TypeSpec type_spec);
        StatementNode* ParseControlFlowStatement();
        IfStatementNode* ParseIfStatement();
        ForStatementNode* ParseForStatement();
        DoStatementNode* ParseDoStatement();
        WhileStatementNode* ParseWhileStatement();
        SwitchStatementNode* ParseSwitchStatement();
        CaseStatementNode* ParseCaseLabel();
        StatementNode* ParseJumpStatement();

        const Token& current_token() const;
        const Token& PeekToken(std::int64_t offset = 1) const;
        void ConsumeToken(std::ptrdiff_t count = 1);
        bool MatchAndConsume(TokenType type);
        ArenaVector<Token> CaptureDirectiveTokens(std::string_view target_file, std::uint32_t directive_physical_line);
        SourceLocation GetCurrentTokenEnd() const;
        SourceLocation GetPreviousTokenEnd() const;

        template <typename Ty>
        ArenaVector<Ty*> ParseSequence(TokenType terminator, auto parse_func, bool consume_terminator);

        Scope* EnterScope(const SourceLocation& location, const SymbolInfo* host_symbol = nullptr, ScopeKind kind = ScopeKind::kCommon);
        void   LeaveScope(const SourceLocation& location);

        std::vector<std::string> MangleParameterNames(const FunctionDeclarationNode* node);
        std::string MangleFunctionName(std::string_view base_name, std::span<const std::string> param_typenames);
        std::size_t GetNextAnonymousId();

        Scope* current_scope();

        Document&                               document_;
        const SourceFile*                       source_file_;
        std::vector<Token>                      raw_tokens_;
        std::vector<Token>                      expanded_tokens_;
        ArenaVector<PreprocessorNode*>          pprefs_{ ArenaAllocator<PreprocessorNode*>(*document_.arena) };
        std::stack<Scope*, std::vector<Scope*>> scope_stack_;
        std::size_t                             token_index_{};
        std::size_t                             anonymous_block_index_{};
        int                                     version_replica_{};
        VersionPointer                          version_pointer_;

        // Parser Global States
        // --------------------------------------------------------------------
        // for re-attach syntax for builtin variable, like
        // const int gl_BuiltinVariable = 0; // builtin declaration
        // layout(constant_id = 0) gl_BuiltinVariable; // re-attach
        ankerl::unordered_dense::map<const SymbolInfo*, VariableDeclarationNode*> variable_declaration_cache_;
    };
}

#include "Parser.inl"
