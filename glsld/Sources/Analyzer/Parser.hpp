#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <stack>
#include <string_view>
#include <vector>

#include "Analyzer/SymbolTable.hpp"
#include "Analyzer/TinyLexer.hpp"
#include "Analyzer/Token.hpp"

namespace glsld {
    class Parser {
    public:
        Parser(std::string_view source, DocumentSymbols& symbols);

        void Parse();
        const auto& tokens() const;

    private:
        void ParserMainTask();
        void ParseStatement();
        void ParsePreprocessor();
        void ParseDefine();
        Scope* ParseScope(ScopeKind kind = ScopeKind::kCommon);
        void ParseDeclaration();
        void ParseFunction();
        std::vector<std::string> ParseParameterList();
        std::vector<Token> ParseQualifiersAndType();
        void ParseLayoutQualifier();
        void ParseVariableDeclarationList();
        void ParseBlockBody(SymbolKind block_kind);
        void ParseControlFlowStatement();
        void ParseIfStatement();
        void ParseForStatement();
        void ParseDoStatement();
        void ParseWhileStatement();
        void ParseSwitchStatement();
        void ParseCaseLabel();
        void ParseJumpStatement();

        Token& CurrentToken();
        Token& PeekToken(std::int64_t offset = 1);
        void ConsumeToken(std::size_t count = 1);
        void ConsumeToEndOfLine();
        bool MatchAndConsume(TokenType type);
        void SkipUntilToken(TokenType type, auto operate, bool consume_target = true);
        void SkipParenthesesGroup();

        Scope* EnterScope(const SourceLocation& location, ScopeKind kind = ScopeKind::kCommon);
        void LeaveScope(const SourceLocation& location);

        std::string MangleFunctionName(std::string_view base_name, std::span<const std::string> param_types);

        Scope* current_scope();

        TinyLexer lexer_;
        DocumentSymbols& symbols_;
        std::vector<Token> tokens_;
        std::stack<Scope*, std::vector<Scope*>> scope_stack_;
        std::size_t token_index_{};
    };
}

namespace glsld {
    inline const auto& Parser::tokens() const {
        return tokens_;
    }

    inline Token& Parser::CurrentToken() {
        return tokens_[token_index_];
    }

    inline Scope* Parser::current_scope() {
        return scope_stack_.top();
    }
}
