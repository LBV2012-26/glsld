#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <stack>
#include <string_view>
#include <vector>

#include "Analyzer/Ast.hpp"
#include "Analyzer/SymbolTable.hpp"
#include "Analyzer/TinyLexer.hpp"
#include "Analyzer/Token.hpp"

namespace glsld {
    class Parser {
    public:
        Parser(std::string_view source, DocumentSymbols& symbols);

        std::unique_ptr<TranslationUnitNode> Parse();
        const auto& tokens() const;

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

        std::unique_ptr<TranslationUnitNode> ParserMainTask();
        std::unique_ptr<StatementNode> ParseStatement();
        std::unique_ptr<PreprocessorNode> ParsePreprocessor();
        std::unique_ptr<PreprocessorNode> ParseDefine(std::unique_ptr<PreprocessorNode> node, std::size_t directive_physical_line);
        std::unique_ptr<CompoundStatementNode> ParseScope(ScopeKind kind = ScopeKind::kCommon);
        std::unique_ptr<StatementNode> ParseCodeStatement();
        std::unique_ptr<FunctionDeclarationNode> ParseFunction(const TypeSpecifier& type_spec);
        std::vector<std::unique_ptr<VariableDeclarationNode>> ParseParameterList();
        TypeSpecifier ParseQualifiersAndType();
        std::vector<Token> ParseLayoutQualifier();
        std::unique_ptr<DeclarationGroupNode> ParseVariableDeclarationList(const TypeSpecifier& type_spec);
        std::unique_ptr<ExpressionStatementNode> ParseExpressionStatement();
        std::unique_ptr<ExpressionNode> ParsePrefixExpression();
        std::unique_ptr<ExpressionNode> ParseInfixExpression(std::unique_ptr<ExpressionNode> left, TokenType op_type, Precedence precedence);
        std::unique_ptr<ExpressionNode> ParseExpression(Precedence min_prec);
        std::unique_ptr<DeclarationNode> ParseBlockBody(const TypeSpecifier& type_spec);
        std::unique_ptr<StatementNode> ParseControlFlowStatement();
        std::unique_ptr<IfStatementNode> ParseIfStatement();
        std::unique_ptr<ForStatementNode> ParseForStatement();
        std::unique_ptr<DoStatementNode> ParseDoStatement();
        std::unique_ptr<WhileStatementNode> ParseWhileStatement();
        std::unique_ptr<SwitchStatementNode> ParseSwitchStatement();
        std::unique_ptr<CaseStatementNode> ParseCaseLabel();
        std::unique_ptr<StatementNode> ParseJumpStatement();

        const Token& CurrentToken() const;
        const Token& PeekToken(std::int64_t offset = 1) const;
        void ConsumeToken(std::size_t count = 1);
        std::vector<Token> CaptureDirectiveTokens(std::size_t directive_physical_line);
        bool MatchAndConsume(TokenType type);
        SourceLocation GetCurrentTokenEnd() const;
        SourceLocation GetPreviousTokenEnd() const;

        template <typename Ty>
        std::vector<std::unique_ptr<Ty>> ParseSequence(TokenType terminator, auto parse_func, bool consume_terminator);

        Scope* EnterScope(SourceLocation location, ScopeKind kind = ScopeKind::kCommon);
        void LeaveScope(SourceLocation location);

        std::string MangleFunctionName(std::string_view base_name, std::span<const std::string> param_typenames);

        Scope* current_scope();

        TinyLexer lexer_;
        DocumentSymbols& symbols_;
        std::vector<Token> tokens_;
        std::stack<Scope*, std::vector<Scope*>> scope_stack_;
        std::size_t token_index_{};
    };
}

#include "Parser.inl"
