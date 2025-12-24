#include "stdafx.h"
#include "Parser.hpp"

#include <algorithm>
#include <format>
#include <memory>

namespace glsld {
    Parser::Parser(std::string_view source, DocumentSymbols& symbols)
        : lexer_{ source }
        , symbols_{ symbols }
    {
        Token token{};
        do {
            token = lexer_.AcquireNextToken();
            tokens_.push_back(token);
        } while (token.type != TokenType::kEndOfFile);
    }

    void Parser::Parse() {
        symbols_.root_scope()->kind_ = ScopeKind::kTransparent;
        scope_stack_.push(symbols_.root_scope());
        ParserMainTask();
    }

    void Parser::ParserMainTask() {
        while (CurrentToken().type != TokenType::kEndOfFile) {
            ParseStatement();
        }
    }

    void Parser::ParseStatement() {
        switch (CurrentToken().type) {
        case TokenType::kKeyword:
        case TokenType::kKeyword_Typed:
        case TokenType::kIdentifier:
            ParseDeclaration();
            break;
        case TokenType::kKeyword_Control:
            ParseControlFlowStatement();
            break;
        case TokenType::kOpenBrace:
            ParseScope();
            break;
        case TokenType::kCloseBrace: // may syntax error but we allow this
            LeaveScope(CurrentToken().location);
            ConsumeToken();
            break;
        case TokenType::kSharp:
            ParsePreprocessor();
            break;
        default:
            while (CurrentToken().type != TokenType::kEndOfFile && CurrentToken().type != TokenType::kSemicolon &&
                   CurrentToken().type != TokenType::kOpenBrace && CurrentToken().type != TokenType::kCloseBrace)
            {
                ConsumeToken();
            }

            MatchAndConsume(TokenType::kSemicolon);
        }
    }

    void Parser::ParsePreprocessor() {
        // current token is '#'
        MatchAndConsume(TokenType::kSharp);

        if (CurrentToken().type == TokenType::kEndOfFile) {
            return;
        }

        auto& current_token = CurrentToken();
        if (current_token.type == TokenType::kKeyword_Control) {
            // #if defined, #else
            if (current_token.text == "if" || current_token.text == "else") {
                tokens_[token_index_].type = TokenType::kPreprocessor;
            }
        }

        const auto& directive_token = CurrentToken();

        if (directive_token.text == "define") {
            ConsumeToken();
            ParseDefine();
        } else {
            ConsumeToEndOfLine();
        }
    }

    void Parser::ParseDefine() {
        if (CurrentToken().type != TokenType::kIdentifier) {
            ConsumeToEndOfLine();
            return;
        }

        const auto& macro_token = CurrentToken();
        SymbolInfo symbol(macro_token.text, macro_token.location, SymbolKind::kMacro);
        current_scope()->AddSymbol(symbol);

        ConsumeToEndOfLine();
    }

    Scope* Parser::ParseScope(ScopeKind kind) {
        const auto& begin_location = CurrentToken().location;
        MatchAndConsume(TokenType::kOpenBrace);

        Scope* entered_scope = EnterScope(begin_location, kind);
        SkipUntilToken(TokenType::kCloseBrace, [this]() -> void { ParseStatement(); }, false);

        const auto& end_location = CurrentToken().location;
        MatchAndConsume(TokenType::kCloseBrace);

        LeaveScope(end_location);
        return entered_scope;
    }

    void Parser::ParseDeclaration() {
        // current token is qualifier, type or identifier
        auto qualifiers = ParseQualifiersAndType();

        auto GetBlockKind = [&qualifiers]() -> SymbolKind {
            for (const auto& qualifier : qualifiers) {
                if (qualifier.text == "uniform" || qualifier.text == "buffer") {
                    return SymbolKind::kInterface;
                }

                if (qualifier.text == "struct") {
                    return SymbolKind::kStruct;
                }
            }

            return SymbolKind::kVariable;
        };

        if (CurrentToken().type == TokenType::kIdentifier && PeekToken().type == TokenType::kOpenBrace) {
            SymbolKind kind = GetBlockKind();

            if (kind != SymbolKind::kVariable) {
                ParseBlockBody(kind);
                return;
            }
        }

        if (qualifiers.empty()) {
            if (CurrentToken().type == TokenType::kIdentifier) {
                SkipUntilToken(TokenType::kSemicolon, [this]() -> void { ConsumeToken(); });
                return;
            }
        }

        if (CurrentToken().type != TokenType::kIdentifier) {
            // layout(early_fragment_tests) in;
            // precision highp float;
            // current token is ';'
            SkipUntilToken(TokenType::kSemicolon, [this]() -> void { ConsumeToken(); });
            return;
        }

        if (PeekToken().type == TokenType::kOpenParen) {
            ParseFunction();
        } else {
            ParseVariableDeclarationList();
        }
    }

    void Parser::ParseFunction() {
        // current token is function name
        const auto& name_token = CurrentToken();
        ConsumeToken();

        const auto& begin_location = CurrentToken().location;
        MatchAndConsume(TokenType::kOpenParen);
        EnterScope(begin_location);

        std::vector<std::string> param_list;
        if (CurrentToken().type != TokenType::kCloseParen) {
            param_list = ParseParameterList();
        } else {
            param_list.push_back("void");
        }

        std::string function_name = MangleFunctionName(name_token.text, param_list);

        // current token is ')'
        MatchAndConsume(TokenType::kCloseParen);
        SymbolKind kind{};
        if (CurrentToken().type == TokenType::kOpenBrace) {
            // function body
            function_name = std::format("__Impl_{}", function_name);
            kind = SymbolKind::kFunctionImpl;
            MatchAndConsume(TokenType::kOpenBrace);
            SkipUntilToken(TokenType::kCloseBrace, [this]() -> void { ParseStatement(); }, false);

            const auto& end_location = CurrentToken().location;
            MatchAndConsume(TokenType::kCloseBrace);
            LeaveScope(end_location);
        } else {
            function_name = std::format("__Decl_{}", function_name);
            kind = SymbolKind::kFunctionDecl;
            MatchAndConsume(TokenType::kSemicolon);
            LeaveScope(CurrentToken().location);
        }

        SymbolInfo symbol(function_name, name_token.location, kind);
        current_scope()->AddSymbol(symbol);
    }

    std::vector<std::string> Parser::ParseParameterList() {
        // current token is first parameter or "void"
        std::vector<std::string> param_list;

        // Function(void)
        if (CurrentToken().type == TokenType::kKeyword &&
            CurrentToken().text == "void" &&
            PeekToken().type == TokenType::kCloseParen)
        {
            ConsumeToken();
            param_list.push_back("void");
            return param_list;
        }

        while (CurrentToken().type != TokenType::kEndOfFile && CurrentToken().type != TokenType::kCloseParen) {
            auto qualifiers = ParseQualifiersAndType();

            std::string parameter;
            for (std::size_t i = 0; i != qualifiers.size(); ++i) {
                parameter += qualifiers[i].text;
                parameter += (i + 1 != qualifiers.size()) ? " " : "";
            }

            param_list.push_back(parameter);

            if (CurrentToken().type == TokenType::kIdentifier) {
                const auto& param_name_token = CurrentToken();
                SymbolInfo param_symbol(param_name_token.text, param_name_token.location, SymbolKind::kParameter);
                current_scope()->AddSymbol(param_symbol);

                ConsumeToken();

                if (MatchAndConsume(TokenType::kOpenBracket)) {
                    SkipUntilToken(TokenType::kCloseBracket, [this]() -> void { ConsumeToken(); });
                }
            }

            if (MatchAndConsume(TokenType::kComma)) {
                continue;
            } else {
                break;
            }
        }

        return param_list;
    }

    std::vector<Token> Parser::ParseQualifiersAndType() {
        // current token is qualifier or type such as: Func("const" int input)
        std::vector<Token> qualifiers;

        while (true) {
            const auto& current_token = CurrentToken();

            // layout(...)
            if (current_token.type == TokenType::kKeyword && current_token.text == "layout") {
                qualifiers.push_back(current_token);
                ParseLayoutQualifier();
                continue;
            }

            // (in, out, uniform, const, struct, ...)
            // (vec3, mat4, float, ...)
            if (current_token.type == TokenType::kKeyword || current_token.type == TokenType::kKeyword_Typed) {
                qualifiers.push_back(current_token);
                ConsumeToken();
                continue;
            }

            // MyStruct s
            if (current_token.type == TokenType::kIdentifier) {
                const auto& next_token = PeekToken();
                if (next_token.type == TokenType::kIdentifier) {
                    qualifiers.push_back(current_token);
                    ConsumeToken();
                    continue;
                } else {
                    break;
                }
            }

            break;
        }

        return qualifiers;
    }

    void Parser::ParseLayoutQualifier() {
        // current token is "layout"
        ConsumeToken();

        if (!MatchAndConsume(TokenType::kOpenParen)) {
            return;
        }

        while (CurrentToken().type != TokenType::kEndOfFile && CurrentToken().type != TokenType::kCloseParen) {
            if (CurrentToken().type == TokenType::kIdentifier || CurrentToken().type == TokenType::kKeyword) {
                ConsumeToken();
                if (MatchAndConsume(TokenType::kEqual)) {
                    if (CurrentToken().type == TokenType::kNumberLiteral || CurrentToken().type == TokenType::kIdentifier) {
                        ConsumeToken();
                    }
                }

                MatchAndConsume(TokenType::kComma);
            } else {
                ConsumeToken();
            }
        }

        MatchAndConsume(TokenType::kCloseParen);
    }

    void Parser::ParseVariableDeclarationList() {
        // current token is variable name
        const auto& name_token = CurrentToken();
        ConsumeToken();

        SymbolInfo first_var_symbol(name_token.text, name_token.location, SymbolKind::kVariable);
        current_scope()->AddSymbol(first_var_symbol);

        auto SkipArrayAndInitialize = [this]() -> void {
            if (MatchAndConsume(TokenType::kOpenBracket)) { // skip array
                SkipUntilToken(TokenType::kCloseBracket, [this]() -> void { ConsumeToken(); });
            }

            if (MatchAndConsume(TokenType::kEqual)) { // skip initialize
                int paren_level = 0;
                while (CurrentToken().type != TokenType::kEndOfFile) {
                    const auto& current_token = CurrentToken();
                    if (current_token.type == TokenType::kOpenParen) {
                        ++paren_level;
                    } else if (current_token.type == TokenType::kCloseParen) {
                        --paren_level;
                    }

                    if (paren_level == 0 && (current_token.type == TokenType::kSemicolon ||
                                             current_token.type == TokenType::kComma))
                    {
                        break;
                    }

                    ConsumeToken();
                }
            }
        };

        SkipArrayAndInitialize();

        while (MatchAndConsume(TokenType::kComma)) {
            if (CurrentToken().type != TokenType::kIdentifier) {
                break;
            }

            const auto& next_name_token = CurrentToken();
            SymbolInfo next_var_symbol(next_name_token.text, next_name_token.location, SymbolKind::kVariable);
            current_scope()->AddSymbol(next_var_symbol);
            ConsumeToken();

            SkipArrayAndInitialize();
        }

        MatchAndConsume(TokenType::kSemicolon);
    }

    void Parser::ParseBlockBody(SymbolKind block_kind) {
        // current token is block name
        const auto& block_name = CurrentToken();
        ConsumeToken();

        SymbolInfo block_symbol(block_name.text, block_name.location, block_kind);
        current_scope()->AddSymbol(block_symbol);

        auto* entered_scope = ParseScope(ScopeKind::kStruct);
        bool  has_instance  = false;

        while (CurrentToken().type != TokenType::kEndOfFile) {
            if (CurrentToken().type != TokenType::kIdentifier) {
                break;
            }

            const auto& instance_token = CurrentToken();
            SymbolInfo instance_symbol(instance_token.text, instance_token.location, SymbolKind::kVariable);
            current_scope()->AddSymbol(instance_symbol);
            has_instance = true;
            ConsumeToken();

            if (MatchAndConsume(TokenType::kOpenBracket)) {
                SkipUntilToken(TokenType::kCloseBracket, [this]() -> void { ConsumeToken(); });
            }

            if (MatchAndConsume(TokenType::kComma)) {
                continue;
            } else {
                break;
            }
        }

        if (!has_instance && block_kind != SymbolKind::kStruct) {
            entered_scope->kind_ = ScopeKind::kTransparent;
        }

        MatchAndConsume(TokenType::kSemicolon);
    }

    void Parser::ParseControlFlowStatement() {
        const auto& keyword = CurrentToken().text;

        if (keyword == "if") {
            ParseIfStatement();
        } else if (keyword == "else") {
            ConsumeToken();
            ParseStatement();
        } else if (keyword == "for") {
            ParseForStatement();
        } else if (keyword == "do") {
            ParseDoStatement();
        } else if (keyword == "while") {
            ParseWhileStatement();
        } else if (keyword == "switch") {
            ParseSwitchStatement();
        } else if (keyword == "case" || keyword == "default") {
            ParseCaseLabel();
        } else if (keyword == "return" || keyword == "break" || keyword == "continue" || keyword == "discard") {
            ParseJumpStatement();
        } else {
            ConsumeToken();
        }
    }

    void Parser::ParseIfStatement() {
        // current token is "if"
        ConsumeToken();

        SkipParenthesesGroup();
        ParseStatement();

        if (CurrentToken().type == TokenType::kKeyword_Control && CurrentToken().text == "else") {
            ConsumeToken(); // else
            ParseStatement();
        }
    }

    void Parser::ParseForStatement() {
        // current token is "for"
        ConsumeToken();

        const auto& begin_location = CurrentToken().location;
        MatchAndConsume(TokenType::kOpenParen);

        EnterScope(begin_location);

        if (CurrentToken().type == TokenType::kSemicolon) {
            // for (; ...
            ConsumeToken();
        } else {
            bool looks_like_declaration = false;
            const auto& current_token = CurrentToken();
            if (current_token.type == TokenType::kKeyword || current_token.type == TokenType::kKeyword_Typed) {
                looks_like_declaration = true;
            } else if (current_token.type == TokenType::kIdentifier) {
                if (PeekToken().type == TokenType::kIdentifier) {
                    looks_like_declaration = true;
                }
            }

            if (looks_like_declaration) {
                ParseDeclaration();
            } else {
                SkipUntilToken(TokenType::kSemicolon, [this]() -> void { ConsumeToken(); });
            }
        }

        SkipUntilToken(TokenType::kSemicolon,  [this]() -> void { ConsumeToken(); });
        SkipUntilToken(TokenType::kCloseParen, [this]() -> void { ConsumeToken(); });

        ParseStatement();
        LeaveScope(PeekToken(-1).location);
    }

    void Parser::ParseDoStatement() {
        // current token is "do"
        ConsumeToken();
        ParseStatement();

        const auto& current_token = CurrentToken();
        if (current_token.type == TokenType::kKeyword_Control && current_token.text == "while") {
            ConsumeToken();
            SkipParenthesesGroup();
            MatchAndConsume(TokenType::kSemicolon);
        }
    }

    void Parser::ParseWhileStatement() {
        // current token is "while"
        ConsumeToken();
        SkipParenthesesGroup();
        ParseStatement();
    }

    void Parser::ParseSwitchStatement() {
        // current token is "switch"
        ConsumeToken();
        SkipParenthesesGroup();
        ParseStatement();
    }

    void Parser::ParseCaseLabel() {
        // current token is "case" or "default"
        ConsumeToken();
        SkipUntilToken(TokenType::kColon, [this]() -> void { ConsumeToken(); });
    }

    void Parser::ParseJumpStatement() {
        // current token is "return", "break", "continue" or "discard"
        ConsumeToken();
        SkipUntilToken(TokenType::kSemicolon, [this]() -> void { ConsumeToken(); });
    }

    Token& Parser::PeekToken(std::int64_t offset) {
        if (token_index_ + offset >= tokens_.size()) {
            return tokens_.back();
        }

        return tokens_[token_index_ + offset];
    }

    void Parser::ConsumeToken(std::size_t count) {
        token_index_ = std::min(token_index_ + count, tokens_.size() - 1);
    }

    void Parser::ConsumeToEndOfLine() {
        if (CurrentToken().type == TokenType::kEndOfFile) {
            return;
        }

        const auto start_line = CurrentToken().location.line;
        while (CurrentToken().type != TokenType::kEndOfFile && CurrentToken().location.line == start_line) {
            ConsumeToken();
        }
    }

    bool Parser::MatchAndConsume(TokenType type) {
        if (CurrentToken().type == type) {
            ConsumeToken();
            return true;
        }

        return false;
    }

    void Parser::SkipUntilToken(TokenType type, auto operate, bool consume_target) {
        while (CurrentToken().type != TokenType::kEndOfFile && CurrentToken().type != type) {
            operate();
        }

        if (consume_target) {
            MatchAndConsume(type);
        }
    }

    void Parser::SkipParenthesesGroup() {
        if (!MatchAndConsume(TokenType::kOpenParen)) {
            return;
        }

        int paren_level = 1;
        while (paren_level > 0 && CurrentToken().type != TokenType::kEndOfFile) {
            const auto& current_token = CurrentToken();
            if (current_token.type == TokenType::kOpenParen) {
                ++paren_level;
            } else if (current_token.type == TokenType::kCloseParen) {
                --paren_level;
            }

            ConsumeToken();
        }
    }

    Scope* Parser::EnterScope(const SourceLocation& location, ScopeKind kind) {
        auto new_scope = std::make_unique<Scope>(current_scope());

        Scope* new_scope_ptr           = new_scope.get();
        new_scope_ptr->kind_           = kind;
        new_scope_ptr->interval_.first = location;

        current_scope()->children_.push_back(std::move(new_scope));
        scope_stack_.push(new_scope_ptr);

        return new_scope_ptr;
    }

    void Parser::LeaveScope(const SourceLocation& location) {
        if (scope_stack_.size() > 1) {
            current_scope()->interval_.second = location;
            scope_stack_.pop();
        }
    }

    std::string Parser::MangleFunctionName(std::string_view base_name, std::span<const std::string> param_types) {
        std::string mangled(base_name);
        mangled += "(";

        for (std::size_t i = 0; i != param_types.size(); ++i) {
            mangled += param_types[i];
            mangled += i == param_types.size() - 1 ? ")" : ", ";
        }

        return mangled;
    }
}
