#pragma once

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <filesystem>
#include <memory>
#include <span>
#include <stack>
#include <string_view>
#include <vector>

#include <ankerl/unordered_dense.h>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Lexer.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Base/FileSystem/IncludeLoader.hpp"
#include "Base/FileSystem/Source.hpp"

namespace glsld {
    class Parser {
    public:
        Parser(SourceTable& source_table,
               const SourceFile* source_file,
               std::vector<Token> raw_tokens,
               IncludeLoader& include_loader,
               IncludeDirectoryHandle include_dirs,
               int version_replica,
               VersionPointer version_pointer,
               Document& document);

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
            kPrefix,         // ! ~ - + ++ --
            kPostfix,        // . [ ] ( ) ++ --
            kHighest
        };

        Precedence GetInfixPrecedence(TokenType type);
        bool IsRightAssociative(TokenType type);

        void Parse(SourceTable& source_table,
                   IncludeLoader& include_loader,
                   IncludeDirectoryHandle include_dirs);

        std::unique_ptr<TranslationUnitNode> ParserMainTask();
        std::unique_ptr<StatementNode> ParseStatement();
        std::unique_ptr<PreprocessorNode> ParsePreprocessor();
        std::unique_ptr<PreprocessorNode> ParseDefine(std::unique_ptr<PreprocessorNode> node, std::string_view target_file, std::uint32_t directive_physical_line);
        std::vector<std::unique_ptr<StatementNode>> ParseMacroBody(std::span<const Token> body_tokens, const SymbolInfo* host_symbol);
        std::unique_ptr<CompoundStatementNode> ParseScope(const SymbolInfo* host_symbol = nullptr, ScopeKind kind = ScopeKind::kCommon);
        std::vector<std::unique_ptr<AttributeNode>> ParseAttributeList();
        std::unique_ptr<StatementNode> ParseCodeStatement();
        std::unique_ptr<FunctionDeclarationNode> ParseFunction(TypeSpecifier type_spec);
        std::vector<std::unique_ptr<VariableDeclarationNode>> ParseParameterList();

        TypeSpecifier ParseQualifiersAndType();
        std::vector<Token> CaptureBalancedTokens(TokenType open, TokenType close);
        std::unique_ptr<QualifierArgumentNode> ParseQualifierArguments(std::span<const Token> tokens);
        std::unique_ptr<LayoutQualifierNode> ParseLayoutQualifier();
        std::unique_ptr<SpirvIntrinsicNode> ParseSpirvIntrinsics();
        std::unique_ptr<ExpressionNode> ParseTemplateArgument();

        bool TryParseLayoutQualifier(TypeSpecifier& type_spec);
        bool TryParseSpirvIntrinsics(TypeSpecifier& type_spec);

        std::unique_ptr<DeclarationGroupNode> ParseVariableDeclarationList(TypeSpecifier type_spec);
        std::unique_ptr<ExpressionStatementNode> ParseExpressionStatement();

        std::unique_ptr<ExpressionNode> ParsePrefixExpression();
        std::unique_ptr<RawExpressionNode> ParseLiteral();
        std::unique_ptr<VariableExpressionNode> ParseVariableReference();
        std::unique_ptr<UnaryExpressionNode> ParsePrefixUnary();
        std::unique_ptr<InitializerListExpressionNode> ParseInitializerList();

        std::unique_ptr<ExpressionNode> ParseInfixExpression(std::unique_ptr<ExpressionNode> left, TokenType op_type, Precedence precedence);
        std::unique_ptr<MemberAccessExpressionNode> ParseMemberAccess(std::unique_ptr<ExpressionNode> object);
        std::unique_ptr<IndexExpressionNode> ParseArrayIndex(std::unique_ptr<ExpressionNode> base);
        std::unique_ptr<CallExpressionNode> ParseFunctionCall(std::unique_ptr<ExpressionNode> callee);
        std::unique_ptr<TernaryExpressionNode> ParseTernary(std::unique_ptr<ExpressionNode> condition);
        std::unique_ptr<UnaryExpressionNode> ParsePostfixUnary(std::unique_ptr<ExpressionNode> operand, TokenType op_type);
        std::unique_ptr<BinaryExpressionNode> ParseStandardBinary(std::unique_ptr<ExpressionNode> left, TokenType op_type, Precedence precedence);
        std::unique_ptr<ExpressionNode> ParseExpression(Precedence min_prec);

        std::unique_ptr<DeclarationNode> ParseBlockBody(TypeSpecifier type_spec);
        std::unique_ptr<StatementNode> ParseControlFlowStatement();
        std::unique_ptr<IfStatementNode> ParseIfStatement();
        std::unique_ptr<ForStatementNode> ParseForStatement();
        std::unique_ptr<DoStatementNode> ParseDoStatement();
        std::unique_ptr<WhileStatementNode> ParseWhileStatement();
        std::unique_ptr<SwitchStatementNode> ParseSwitchStatement();
        std::unique_ptr<CaseStatementNode> ParseCaseLabel();
        std::unique_ptr<StatementNode> ParseJumpStatement();

        const Token& current_token() const;
        const Token& PeekToken(std::int64_t offset = 1) const;
        void ConsumeToken(std::ptrdiff_t count = 1);
        bool MatchAndConsume(TokenType type);
        std::vector<Token> CaptureDirectiveTokens(std::string_view target_file, std::uint32_t directive_physical_line);
        SourceLocation GetCurrentTokenEnd() const;
        SourceLocation GetPreviousTokenEnd() const;

        template <typename Ty>
        std::vector<std::unique_ptr<Ty>> ParseSequence(TokenType terminator, auto parse_func, bool consume_terminator);

        Scope* EnterScope(const SourceLocation& location, const SymbolInfo* host_symbol = nullptr, ScopeKind kind = ScopeKind::kCommon);
        void   LeaveScope(const SourceLocation& location);

        std::vector<std::string> MangleParameterNames(const FunctionDeclarationNode* node);
        std::string MangleFunctionName(std::string_view base_name, std::span<const std::string> param_typenames);
        std::size_t GetNextAnonymousId();

        Scope* current_scope();

        const SourceFile*                       source_file_;
        std::vector<Token>                      raw_tokens_;
        std::vector<Token>                      expanded_tokens_;
        std::vector<PreprocessorNode*>          preprocessor_references_;
        std::stack<Scope*, std::vector<Scope*>> scope_stack_;
        std::size_t                             token_index_{};
        std::size_t                             anonymous_block_index_{};
        int                                     version_replica_{};
        VersionPointer                          version_pointer_;
        Document&                               document_;

        // for re-attach syntax like
        // const int kConstant = 0;
        // layout(constant_id = 0) kConstant;
        ankerl::unordered_dense::map<const SymbolInfo*, VariableDeclarationNode*> variable_declaration_cache_;
    };
}

#include "Parser.inl"
