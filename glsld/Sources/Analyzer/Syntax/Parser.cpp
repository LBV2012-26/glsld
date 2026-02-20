#include "stdafx.h"
#include "Parser.hpp"

#include <concepts>
#include <format>
#include <utility>

namespace glsld {
    Parser::Parser(std::string_view source, DocumentSymbols& symbols, int version_replica,
                   std::shared_ptr<const std::atomic<int>> version_pointer)
        : lexer_{ source }
        , symbols_{ symbols }
        , version_replica_{ version_replica }
        , version_pointer_{ version_pointer }
    {
        Token token;
        do {
            token = lexer_.AcquireNextToken();
            tokens_.push_back(token);
        } while (token.type != TokenType::kEndOfFile);
    }

    std::unique_ptr<TranslationUnitNode> Parser::Parse() {
        symbols_.root_scope()->kind_ = ScopeKind::kTransparent;
        scope_stack_.push(symbols_.root_scope());
        return ParserMainTask();
    }

    Parser::Precedence Parser::GetInfixPrecedence(TokenType type) {
        switch (type) {
        // 0. 分隔符/终结符
        case TokenType::kSemicolon:
        case TokenType::kCloseParen:
        case TokenType::kCloseBracket:
        case TokenType::kCloseBrace:
        case TokenType::kColon:
            return Precedence::kLowest;

        // 1. 逗号
        case TokenType::kComma:
            return Precedence::kComma;

        // 2. 赋值运算 (右结合)
        case TokenType::kEqual:
        case TokenType::kPlusEqual:
        case TokenType::kMinusEqual:
        case TokenType::kStarEqual:
        case TokenType::kSlashEqual:
        case TokenType::kPercentEqual:
        case TokenType::kLeftShiftEqual:
        case TokenType::kRightShiftEqual:
        case TokenType::kAmpersandEqual:
        case TokenType::kCaretEqual:
        case TokenType::kVerticalBarEqual:
            return Precedence::kAssignment;

        // 3. 三元运算 (右结合起始符)
        case TokenType::kQuestion:
            return Precedence::kTernary;

        // 4-6. 逻辑运算
        case TokenType::kVerticalBarVerticalBar:
            return Precedence::kLogicalOr;
        case TokenType::kCaretCaret:
            return Precedence::kLogicalXor;
        case TokenType::kAmpersandAmpersand:
            return Precedence::kLogicalAnd;

        // 7-9. 位运算
        case TokenType::kVerticalBar:
            return Precedence::kBitwiseOr;
        case TokenType::kCaret:
            return Precedence::kBitwiseXor;
        case TokenType::kAmpersand:
            return Precedence::kBitwiseAnd;

        // 10-11. 比较运算
        case TokenType::kEqualEqual:
        case TokenType::kNotEqual:
            return Precedence::kEquality;

        case TokenType::kLessThan:
        case TokenType::kGreaterThan:
        case TokenType::kLessEqual:
        case TokenType::kGreaterEqual:
            return Precedence::kRelational;

        // 12. 位移
        case TokenType::kLeftShift:
        case TokenType::kRightShift:
            return Precedence::kShift;

        // 13-14. 算术运算
        case TokenType::kPlus:
        case TokenType::kMinus:
            return Precedence::kAdditive;

        case TokenType::kStar:
        case TokenType::kSlash:
        case TokenType::kPercent:
            return Precedence::kMultiplicative;

        // 15. 后缀/最高级
        case TokenType::kDot:         // 成员
        case TokenType::kOpenBracket: // 数组
        case TokenType::kOpenParen:   // 函数调用
        case TokenType::kPlusPlus:    // 后缀自增
        case TokenType::kMinusMinus:  // 后缀自减
            return Precedence::kPostfix;

        default:
            return Precedence::kLowest;
        }
    }

    bool Parser::IsRightAssociative(TokenType type) {
        switch (type) {
        case TokenType::kEqual:
        case TokenType::kPlusEqual:
        case TokenType::kMinusEqual:
        case TokenType::kStarEqual:
        case TokenType::kSlashEqual:
        case TokenType::kPercentEqual:
        case TokenType::kLeftShiftEqual:
        case TokenType::kRightShiftEqual:
        case TokenType::kAmpersandEqual:
        case TokenType::kCaretEqual:
        case TokenType::kVerticalBarEqual:
        case TokenType::kQuestion:
            return true;
        default:
            return false;
        }
    }

    std::unique_ptr<TranslationUnitNode> Parser::ParserMainTask() {
        auto root = std::make_unique<TranslationUnitNode>(current_scope());
        root->begin = { 1, 1 };

        while (CurrentToken().type != TokenType::kEndOfFile) {
            if (version_pointer_ != nullptr && version_replica_ != version_pointer_->load()) {
                return nullptr;
            }

            auto statement = ParseStatement();
            if (statement != nullptr) {
                root->statements.push_back(std::move(statement));
            }
        }

        root->end = GetPreviousTokenEnd();
        return root;
    }

    std::unique_ptr<StatementNode> Parser::ParseStatement() {
        switch (CurrentToken().type) {
        case TokenType::kPrimitive:
        case TokenType::kBuiltInType:
        case TokenType::kBuiltInFunction:
        case TokenType::kBuiltInVariable:
        case TokenType::kIdentifier:
            return ParseCodeStatement();
            break;
        case TokenType::kKeyword:
            return ParseControlFlowStatement();
            break;
        case TokenType::kOpenBrace:
            return ParseScope();
            break;
        case TokenType::kSharp:
            return ParsePreprocessor();
            break;
        default:
            while (CurrentToken().type != TokenType::kEndOfFile && CurrentToken().type != TokenType::kSemicolon &&
                   CurrentToken().type != TokenType::kOpenBrace || CurrentToken().type == TokenType::kCloseBrace)
            {
                ConsumeToken();
            }

            MatchAndConsume(TokenType::kSemicolon);
            return std::make_unique<NullStatementNode>(current_scope());
        }
    }

    std::unique_ptr<PreprocessorNode> Parser::ParsePreprocessor() {
        // current token is '#'
        auto node = std::make_unique<PreprocessorNode>(current_scope());
        node->begin = CurrentToken().location;
        MatchAndConsume(TokenType::kSharp);

        if (CurrentToken().type == TokenType::kEndOfFile) {
            return nullptr;
        }

        auto& directive_token = CurrentToken();
        node->directive = std::string(directive_token.text);
        if (directive_token.type == TokenType::kKeyword) {
            // #if defined, #else
            if (directive_token.text == "if" || directive_token.text == "else") {
                tokens_[token_index_].type = TokenType::kPreprocessor;
            }
        }
        ConsumeToken();

        if (directive_token.text == "define") {
            return ParseDefine(std::move(node), directive_token.location.line);
        } else {
            node->tokens = CaptureDirectiveTokens(directive_token.location.line);
            node->end    = GetPreviousTokenEnd();
            return node;
        }
    }

    std::unique_ptr<PreprocessorNode> Parser::ParseDefine(std::unique_ptr<PreprocessorNode> node, std::size_t directive_physical_line) {
        // current token is macro name after "define"
        if (CurrentToken().type == TokenType::kIdentifier) {
            const auto& macro_token = CurrentToken();

            node->symbol = current_scope()->AddSymbol(node.get(), macro_token.text, macro_token.location, SymbolKind::kMacro);
            ConsumeToken();

            auto IsAdjacent = [](const Token& first, const Token& second) -> bool {
                return first.location.line == second.location.line &&
                      (first.location.column + first.text.length() == second.location.column);
            };

            // macro function like #define MACRO(x)
            if (CurrentToken().type == TokenType::kOpenParen && IsAdjacent(macro_token, CurrentToken())) {
                ConsumeToken();
                while (CurrentToken().type != TokenType::kEndOfFile && CurrentToken().type != TokenType::kCloseParen) {
                    if (CurrentToken().type == TokenType::kIdentifier) {
                        node->params.push_back(CurrentToken().text);
                        ConsumeToken();
                    }
                    if (!MatchAndConsume(TokenType::kComma)) {
                        break;
                    }
                }

                MatchAndConsume(TokenType::kCloseParen);
            }
        }

        node->tokens = CaptureDirectiveTokens(directive_physical_line);
        node->end    = GetPreviousTokenEnd();
        return node;
    }

    std::unique_ptr<CompoundStatementNode> Parser::ParseScope(ScopeKind kind) {
        auto node = std::make_unique<CompoundStatementNode>(current_scope());
        node->begin = CurrentToken().location;
        MatchAndConsume(TokenType::kOpenBrace);

        node->internal_scope = EnterScope(node->begin, kind);
        node->children = ParseSequence<StatementNode>(TokenType::kCloseBrace, [this]() -> std::unique_ptr<StatementNode> {
            return ParseStatement();
        }, false);

        node->end = GetCurrentTokenEnd();
        MatchAndConsume(TokenType::kCloseBrace);

        LeaveScope(node->end);
        return node;
    }

    std::unique_ptr<StatementNode> Parser::ParseCodeStatement() {
        // current token is qualifier, type or identifier
        auto type_spec = ParseQualifiersAndType();

        // block, current is identifier, and next is '{'
        if (!type_spec.empty() && CurrentToken().type == TokenType::kIdentifier && PeekToken().type == TokenType::kOpenBrace) {
            return ParseBlockBody(std::move(type_spec));
        }

        // function, current is identifier, and next is '('
        if (!type_spec.empty() && CurrentToken().type == TokenType::kIdentifier && PeekToken().type == TokenType::kOpenParen) {
            return ParseFunction(std::move(type_spec));
        }

        // common variable
        if (!type_spec.empty() && CurrentToken().type != TokenType::kOpenParen) {
            return ParseVariableDeclarationList(std::move(type_spec));
        }

        // expression, including function calling
        bool common_calling = type_spec.empty() && (CurrentToken().type == TokenType::kIdentifier ||
                                                    CurrentToken().type == TokenType::kBuiltInFunction);
        bool constructor = !type_spec.empty() && CurrentToken().type == TokenType::kOpenParen;

        if (common_calling || constructor) {
            if (constructor) {
                ConsumeToken(-1);
            }

            return ParseExpressionStatement();
        }

        return nullptr;
    }

    std::unique_ptr<FunctionDeclarationNode> Parser::ParseFunction(TypeSpecifier type_spec) {
        // current token is function name
        auto node = std::make_unique<FunctionDeclarationNode>(current_scope());
        node->begin     = type_spec.begin_location();
        node->type_spec = std::move(type_spec);

        const auto& name_token = CurrentToken();
        ConsumeToken();

        const auto& begin_location = CurrentToken().location;
        MatchAndConsume(TokenType::kOpenParen);
        auto* located_scope  = current_scope();
        auto* internal_scope = EnterScope(begin_location);

        node->params = ParseParameterList();

        // mangle function name, such as "Func(int array[5], in vec3 v) -> Func(int[5], in vec3)"
        std::vector<std::string> param_typenames;
        for (const auto& param : node->params) {
            std::string param_typename;
            for (const auto& specifier : param->type_spec.specifiers) {
                if (param_typename.empty()) {
                    param_typename = specifier.text;
                } else {
                    param_typename += " " + specifier.text;
                }
            }

            // if (!param_typename.contains("in")    && !param_typename.contains("out") &&
            //     !param_typename.contains("inout") && !param_typename.contains("void"))
            // { // default as "in" if no in/out/inout qualifier
            //     auto const_pos = param_typename.find("const");
            //     if (const_pos == std::string::npos) {
            //         param_typename = "in " + param_typename;
            //     } else {
            //         param_typename.insert(const_pos + 6, "in ");
            //     }
            // }

            for (const auto& array_size : param->type_spec.array_sizes) {
                std::string array_dimension;

                if (array_size->kind() == AstNodeKind::kLiteralExpression) {
                    const auto* raw_node = static_cast<const RawExpressionNode*>(array_size.get());
                    for (const auto& token : raw_node->tokens) {
                        array_dimension += token.text;
                    }
                } else if (array_size->kind() == AstNodeKind::kVariableExpression) {
                    const auto* var_expr = static_cast<const VariableExpressionNode*>(array_size.get());
                    array_dimension = var_expr->name;
                }

                param_typename = std::format("{}[{}]", param_typename, array_dimension);
            }

            param_typenames.push_back(std::move(param_typename));
        }

        std::string function_name = MangleFunctionName(name_token.text, param_typenames);

        // current token is ')'
        MatchAndConsume(TokenType::kCloseParen);
        SymbolKind kind{};
        if (CurrentToken().type == TokenType::kOpenBrace) {
            // function body
            function_name = std::format("__Impl_{}", function_name);
            kind = SymbolKind::kFunctionImpl;

            auto body_node = std::make_unique<CompoundStatementNode>(located_scope);
            body_node->internal_scope = current_scope();
            body_node->begin = CurrentToken().location;

            MatchAndConsume(TokenType::kOpenBrace);

            body_node->children = ParseSequence<StatementNode>(TokenType::kCloseBrace, [this]() -> std::unique_ptr<StatementNode> {
                return ParseStatement();
            }, false);

            body_node->end = GetCurrentTokenEnd();
            node->body = std::move(body_node);
            MatchAndConsume(TokenType::kCloseBrace);
        } else {
            function_name = std::format("__Decl_{}", function_name);
            kind = SymbolKind::kFunctionDecl;
            MatchAndConsume(TokenType::kSemicolon);
        }

        node->end = GetPreviousTokenEnd();
        LeaveScope(node->end);
        // current scope is function located scope
        node->declared_symbol = current_scope()->AddSymbol(node.get(), function_name, name_token.location, kind);
        node->declared_symbol->internal_scope = internal_scope;
        node->internal_scope = internal_scope;

        return node;
    }

    std::vector<std::unique_ptr<VariableDeclarationNode>> Parser::ParseParameterList() {
        // current token is first parameter or "void"
        std::vector<std::unique_ptr<VariableDeclarationNode>> param_list;

        // Function(void)
        if (CurrentToken().type == TokenType::kPrimitive &&
            CurrentToken().text == "void" &&
            PeekToken().type == TokenType::kCloseParen)
        {
            const auto& current_token = CurrentToken();
            auto node = std::make_unique<VariableDeclarationNode>(current_scope());

            node->type_spec.specifiers.push_back(current_token);
            node->begin = current_token.location;
            node->end   = GetCurrentTokenEnd();

            ConsumeToken();
            param_list.push_back(std::move(node));
            return param_list;
        }

        while (CurrentToken().type != TokenType::kEndOfFile && CurrentToken().type != TokenType::kCloseParen) {
            auto type_spec  = ParseQualifiersAndType();
            auto node       = std::make_unique<VariableDeclarationNode>(current_scope());
            node->begin     = type_spec.begin_location();
            node->type_spec = std::move(type_spec);

            if (CurrentToken().type == TokenType::kIdentifier) {
                const auto& name_token = CurrentToken();
                node->declared_symbol  = current_scope()->AddSymbol(node.get(), name_token.text, name_token.location, SymbolKind::kParameter);

                ConsumeToken();

                while (MatchAndConsume(TokenType::kOpenBracket)) {
                    node->type_spec.array_sizes.push_back(ParseExpression(Precedence::kLowest));
                    MatchAndConsume(TokenType::kCloseBracket);
                }
            }

            node->end = GetPreviousTokenEnd();
            param_list.push_back(std::move(node));

            if (!MatchAndConsume(TokenType::kComma)) {
                break;
            }
        }

        if (param_list.empty()) { // void main() -> void main(void)
            auto virtual_void_node    = std::make_unique<VariableDeclarationNode>(current_scope());
            const auto& current_token = CurrentToken();

            virtual_void_node->type_spec.specifiers.emplace_back("void", current_token.location, TokenType::kPrimitive);
            virtual_void_node->begin = current_token.location;
            virtual_void_node->end   = current_token.location;

            param_list.push_back(std::move(virtual_void_node));
        }

        return param_list;
    }

    TypeSpecifier Parser::ParseQualifiersAndType() {
        // current token is specifier such as: Func("const int" input)
        TypeSpecifier type_spec;

        while (true) {
            const auto& current_token = CurrentToken();

            if (current_token.type == TokenType::kPrimitive && current_token.text == "layout") {
                // layout(...)
                type_spec.specifiers.push_back(current_token);
                type_spec.layout_params = ParseLayoutQualifier();
                continue;
            } else if (current_token.type == TokenType::kPrimitive || current_token.type == TokenType::kBuiltInType) {
                // (in, out, uniform, const, struct, ...)
                // (vec3, mat4, float, ...)
                type_spec.specifiers.push_back(current_token);
                ConsumeToken();
                continue;
            } else if (current_token.type == TokenType::kIdentifier) {
                const auto* symbol_info = current_scope()->FindSymbol(current_token.text);
                if (symbol_info == nullptr || (symbol_info->kind != SymbolKind::kStruct && symbol_info->kind != SymbolKind::kInterface)) {
                    break; // 不是类型标识符
                }

                type_spec.specifiers.push_back(current_token);
                ConsumeToken();
            }

            break;
        }

        while (MatchAndConsume(TokenType::kOpenBracket)) {
            // current token is array index
            auto node = ParseExpression(Precedence::kLowest);
            type_spec.array_sizes.push_back(std::move(node));
            MatchAndConsume(TokenType::kCloseBracket);
        }

        return type_spec;
    }

    std::vector<Token> Parser::ParseLayoutQualifier() {
        // current token is "layout"
        // temporarily implementation, just capture all tokens
        ConsumeToken();

        std::vector<Token> tokens;
        if (!MatchAndConsume(TokenType::kOpenParen)) {
            return tokens;
        }

        while (CurrentToken().type != TokenType::kEndOfFile && CurrentToken().type != TokenType::kCloseParen &&
               CurrentToken().type != TokenType::kOpenBrace && CurrentToken().type != TokenType::kCloseBrace)
        {
            if (CurrentToken().type == TokenType::kIdentifier || CurrentToken().type == TokenType::kPrimitive) {
                tokens.push_back(CurrentToken());
                ConsumeToken();

                if (CurrentToken().type == TokenType::kEqual) {
                    tokens.push_back(CurrentToken());
                    ConsumeToken();

                    if (CurrentToken().type == TokenType::kNumberLiteral || CurrentToken().type == TokenType::kIdentifier) {
                        tokens.push_back(CurrentToken());
                        ConsumeToken();
                    }
                }

                if (CurrentToken().type == TokenType::kComma) {
                    tokens.push_back(CurrentToken());
                    ConsumeToken();
                }
            } else {
                tokens.push_back(CurrentToken());
                ConsumeToken();
            }
        }

        MatchAndConsume(TokenType::kCloseParen);
        return tokens;
    }

    std::unique_ptr<DeclarationGroupNode> Parser::ParseVariableDeclarationList(TypeSpecifier type_spec) {
        // current token is variable name or semicolon
        if (CurrentToken().type == TokenType::kSemicolon) {
            auto node   = std::make_unique<DeclarationGroupNode>(current_scope());
            node->begin = type_spec.begin_location();
            node->end   = GetCurrentTokenEnd();

            ConsumeToken();
            return node;
        }

        auto ParseSingleDeclarer = [&]() -> std::unique_ptr<VariableDeclarationNode> {
            if (CurrentToken().type != TokenType::kIdentifier) {
                return nullptr;
            }

            const auto& name_token = CurrentToken();
            auto node = std::make_unique<VariableDeclarationNode>(current_scope());

            node->begin           = type_spec.begin_location();
            node->type_spec       = type_spec;
            node->declared_symbol = current_scope()->AddSymbol(node.get(), name_token.text, name_token.location, SymbolKind::kVariable);

            ConsumeToken();

            // array
            while (MatchAndConsume(TokenType::kOpenBracket)) {
                node->type_spec.array_sizes.push_back(ParseExpression(Precedence::kLowest));
                MatchAndConsume(TokenType::kCloseBracket);
            }

            // Type name = expr;
            if (MatchAndConsume(TokenType::kEqual)) {
                node->init = ParseExpression(Precedence::kAssignment);
            }

            node->end = GetPreviousTokenEnd();
            return node;
        };

        auto node = std::make_unique<DeclarationGroupNode>(current_scope());
        node->begin = type_spec.begin_location();

        if (auto first_node = ParseSingleDeclarer()) {
            node->declarations.push_back(std::move(first_node));
        }

        while (CurrentToken().type == TokenType::kComma) {
            ConsumeToken();
            if (auto next_node = ParseSingleDeclarer()) {
                node->declarations.push_back(std::move(next_node));
            } else {
                break;
            }
        }

        // terminate with semicolon
        if (CurrentToken().type == TokenType::kSemicolon) {
            auto end_location = GetCurrentTokenEnd();
            if (!node->declarations.empty()) {
                node->declarations.back()->end = end_location;
            }

            node->end = end_location;
            ConsumeToken();
        } else if (!node->declarations.empty()) {
            node->end = node->declarations.back()->end;
        }

        return node;
    }

    std::unique_ptr<ExpressionStatementNode> Parser::ParseExpressionStatement() {
        auto node   = std::make_unique<ExpressionStatementNode>(current_scope());
        node->begin = CurrentToken().location;
        node->expr  = ParseExpression(Precedence::kLowest);

        if (CurrentToken().type == TokenType::kSemicolon) {
            node->end = GetCurrentTokenEnd();
            MatchAndConsume(TokenType::kSemicolon);
        } else {
            node->end = GetPreviousTokenEnd();
        }

        return node;
    }

    std::unique_ptr<ExpressionNode> Parser::ParsePrefixExpression() {
        // ( expr )
        if (MatchAndConsume(TokenType::kOpenParen)) {
            auto expr_node = ParseExpression(Precedence::kLowest);
            MatchAndConsume(TokenType::kCloseParen);
            return expr_node;
        }

        const auto& current_token = CurrentToken();

        switch (current_token.type) {
        // 字面量
        case TokenType::kNumberLiteral:
        case TokenType::kStringLiteral:
            return ParseLiteral();

        // 标识符/内置类型/内置函数/Primitive (函数本来应该算标识符，但问题是内置函数无需声明)
        case TokenType::kIdentifier:
        case TokenType::kBuiltInType:
        case TokenType::kBuiltInFunction:
        case TokenType::kPrimitive:
            return ParseVariableReference();

        // 前缀一元运算符 (!b, -x, ++i, --j, ~mask)
        case TokenType::kExclamation:
        case TokenType::kMinus:
        case TokenType::kPlusPlus:
        case TokenType::kMinusMinus:
        case TokenType::kTilde:
            return ParsePrefixUnary();

        // 初始化列表 { ... }
        case TokenType::kOpenBrace:
            return ParseInitializerList();

        default:
            return nullptr;
        }
    }

    std::unique_ptr<RawExpressionNode> Parser::ParseLiteral() {
        // current token is raw literal
        auto node = std::make_unique<RawExpressionNode>(current_scope());
        const auto& current_token = CurrentToken();

        node->begin = current_token.location;
        node->tokens.push_back(current_token);
        ConsumeToken();
        node->end = GetPreviousTokenEnd();

        return node;
    }

    std::unique_ptr<VariableExpressionNode> Parser::ParseVariableReference() {
        auto node = std::make_unique<VariableExpressionNode>(current_scope());
        const auto& current_token = CurrentToken();

        node->begin      = current_token.location;
        node->token_type = current_token.type;
        node->name       = current_token.text;

        if (PeekToken().type != TokenType::kOpenParen) {
            node->node_type = VariableExpressionNode::NodeType::kCommonVariable;
        } else {
            node->node_type = VariableExpressionNode::NodeType::kFunctionCallee;
        }

        ConsumeToken();
        node->end = GetPreviousTokenEnd();

        return node;
    }

    std::unique_ptr<UnaryExpressionNode> Parser::ParsePrefixUnary() {
        auto node = std::make_unique<UnaryExpressionNode>(current_scope());
        const auto& current_token = CurrentToken();

        node->begin      = current_token.location;
        node->op         = current_token.type;
        node->is_postfix = false;
        ConsumeToken();
        node->operand    = ParseExpression(Precedence::kPrefix);

        if (node->operand != nullptr) {
            node->end = node->operand->end;
        } else {
            node->end = GetPreviousTokenEnd();
        }

        return node;
    }

    std::unique_ptr<InitializerListExpressionNode> Parser::ParseInitializerList() {
        // current token is {
        auto node = std::make_unique<InitializerListExpressionNode>(current_scope());
        node->begin = CurrentToken().location;
        MatchAndConsume(TokenType::kOpenBrace);

        if (CurrentToken().type != TokenType::kCloseBrace) {
            while (true) {
                node->elements.push_back(ParseExpression(Precedence::kAssignment));
                if (!MatchAndConsume(TokenType::kComma)) {
                    break;
                }
            }
        }

        node->end = GetCurrentTokenEnd();
        MatchAndConsume(TokenType::kCloseBrace);
        return node;
    }

    std::unique_ptr<ExpressionNode> Parser::ParseInfixExpression(std::unique_ptr<ExpressionNode> left,
                                                                 TokenType op_type, Precedence precedence) {
        switch (op_type) {
        // 成员访问 (object.number)
        case TokenType::kDot:
            return ParseMemberAccess(std::move(left));

        // 数组下标 (array[index])
        case TokenType::kOpenBracket:
            return ParseArrayIndex(std::move(left));

        // 函数调用 (Func(args...))
        case TokenType::kOpenParen:
            return ParseFunctionCall(std::move(left));

        // 后缀运算符
        case TokenType::kPlusPlus:
        case TokenType::kMinusMinus:
            return ParsePostfixUnary(std::move(left), op_type);

        // 标准二元运算符 (+, -, *, /, &&, =, etc.)
        default:
            return ParseStandardBinary(std::move(left), op_type, precedence);
        }
    }

    std::unique_ptr<MemberAccessExpressionNode> Parser::ParseMemberAccess(std::unique_ptr<ExpressionNode> object) {
        auto node = std::make_unique<MemberAccessExpressionNode>(current_scope());

        node->begin  = object->begin;
        node->object = std::move(object);

        if (CurrentToken().type == TokenType::kIdentifier) {
            const auto& member_token = CurrentToken();

            auto member_node        = std::make_unique<VariableExpressionNode>(current_scope());
            member_node->begin      = member_token.location;
            member_node->token_type = member_token.type;
            member_node->node_type  = VariableExpressionNode::NodeType::kBlockMember;
            member_node->name       = member_token.text;
            member_node->end        = GetCurrentTokenEnd();

            node->member = std::move(member_node);

            ConsumeToken();
        }

        node->end = GetPreviousTokenEnd();
        return node;
    }

    std::unique_ptr<IndexExpressionNode> Parser::ParseArrayIndex(std::unique_ptr<ExpressionNode> base) {
        auto node = std::make_unique<IndexExpressionNode>(current_scope());

        node->begin = base->begin;
        node->base  = std::move(base);
        node->index = ParseExpression(Precedence::kLowest);

        if (CurrentToken().type == TokenType::kCloseBracket) {
            node->end = GetCurrentTokenEnd();
            ConsumeToken();
        } else {
            node->end = GetPreviousTokenEnd();
        }

        return node;
    }

    std::unique_ptr<CallExpressionNode> Parser::ParseFunctionCall(std::unique_ptr<ExpressionNode> callee) {
        auto node = std::make_unique<CallExpressionNode>(current_scope());

        node->begin  = callee->begin;
        node->callee = std::move(callee);

        if (CurrentToken().type != TokenType::kCloseParen) {
            while (true) {
                node->args.push_back(ParseExpression(Precedence::kAssignment));
                if (!MatchAndConsume(TokenType::kComma)) {
                    break;
                }
            }
        }

        MatchAndConsume(TokenType::kCloseParen);
        node->end = GetPreviousTokenEnd();
        return node;
    }

    std::unique_ptr<UnaryExpressionNode> Parser::ParsePostfixUnary(std::unique_ptr<ExpressionNode> operand, TokenType op_type) {
        auto node = std::make_unique<UnaryExpressionNode>(current_scope());

        node->begin      = operand->begin;
        node->operand    = std::move(operand);
        node->op         = op_type;
        node->is_postfix = true;
        node->end        = GetPreviousTokenEnd();

        return node;
    }

    std::unique_ptr<BinaryExpressionNode> Parser::ParseStandardBinary(std::unique_ptr<ExpressionNode> left,
                                                                      TokenType op_type, Precedence precedence) {
        auto node = std::make_unique<BinaryExpressionNode>(current_scope());

        node->begin = left->begin;
        node->left  = std::move(left);
        node->op    = op_type;

        Precedence next_min_prec =
            IsRightAssociative(op_type) ? precedence : static_cast<Precedence>(static_cast<int>(precedence) + 1);

        node->right = ParseExpression(next_min_prec);
        if (node->right != nullptr) {
            node->end = node->right->end;
        } else {
            node->end = GetPreviousTokenEnd();
        }

        return node;
    }

    std::unique_ptr<ExpressionNode> Parser::ParseExpression(Precedence min_prec) {
        auto left = ParsePrefixExpression();
        if (left == nullptr) {
            return nullptr;
        }

        while (true) {
            auto op_type = CurrentToken().type;
            auto op_prec = GetInfixPrecedence(op_type);

            if (op_prec == Precedence::kLowest || op_prec < min_prec) {
                break;
            }

            ConsumeToken();
            left = ParseInfixExpression(std::move(left), op_type, op_prec);
        }

        return left;
    }

    std::unique_ptr<DeclarationNode> Parser::ParseBlockBody(TypeSpecifier type_spec) {
        // current token is block name
        const auto& block_name = CurrentToken();
        ConsumeToken();

        bool is_struct = type_spec.has_keyword("struct");

        auto ParseBody = [&](auto& node) -> void {
            node->begin           = type_spec.begin_location();
            auto block_kind       = is_struct ? SymbolKind::kStruct : SymbolKind::kInterface;
            node->declared_symbol = current_scope()->AddSymbol(node.get(), block_name.text, block_name.location, block_kind);
            node->body            = ParseScope(ScopeKind::kBlock);

            if (CurrentToken().type != TokenType::kSemicolon) {
                // struct MyStruct { ... } instance;
                type_spec.specifiers.push_back(block_name);
                node->instances = ParseVariableDeclarationList(std::move(type_spec));
            } else {
                MatchAndConsume(TokenType::kSemicolon);
            }

            if (!is_struct && node->instances == nullptr) {
                if (node->body != nullptr && node->body->internal_scope != nullptr) {
                    node->body->internal_scope->kind_ = ScopeKind::kTransparent;
                }
            }

            if (node->body != nullptr && node->declared_symbol != nullptr) {
                node->declared_symbol->internal_scope = node->body->internal_scope;
            }

            node->internal_scope = node->body->internal_scope;
            node->end = GetPreviousTokenEnd();
        };

        if (is_struct) {
            auto node = std::make_unique<StructDeclarationNode>(current_scope());
            ParseBody(node);
            return node;
        } else {
            auto node = std::make_unique<InterfaceDeclarationNode>(current_scope());
            node->type_spec = type_spec;
            ParseBody(node);
            return node;
        }
    }

    std::unique_ptr<StatementNode> Parser::ParseControlFlowStatement() {
        const auto& keyword = CurrentToken().text;

        if (keyword == "if") {
            return ParseIfStatement();
        } else if (keyword == "for") {
            return ParseForStatement();
        } else if (keyword == "do") {
            return ParseDoStatement();
        } else if (keyword == "while") {
            return ParseWhileStatement();
        } else if (keyword == "switch") {
            return ParseSwitchStatement();
        } else if (keyword == "return" || keyword == "break" || keyword == "continue" || keyword == "discard") {
            return ParseJumpStatement();
        }

        // error
        ConsumeToken();
        return nullptr;
    }

    std::unique_ptr<IfStatementNode> Parser::ParseIfStatement() {
        // current token is "if"
        auto node = std::make_unique<IfStatementNode>(current_scope());
        node->begin = CurrentToken().location;
        ConsumeToken();

        EnterScope(node->begin);

        MatchAndConsume(TokenType::kOpenParen);
        node->condition = ParseExpression(Precedence::kLowest);
        MatchAndConsume(TokenType::kCloseParen);

        node->then_branch = ParseStatement();

        if (CurrentToken().type == TokenType::kKeyword && CurrentToken().text == "else") {
            ConsumeToken(); // else
            node->else_branch = ParseStatement();
        }

        if (node->else_branch != nullptr) {
            node->end = node->else_branch->end;
        } else if (node->then_branch != nullptr) {
            node->end = node->then_branch->end;
        }

        node->internal_scope = current_scope();
        LeaveScope(node->end);
        return node;
    }

    std::unique_ptr<ForStatementNode> Parser::ParseForStatement() {
        // current token is "for"
        auto node = std::make_unique<ForStatementNode>(current_scope());
        node->begin = CurrentToken().location;
        ConsumeToken();

        EnterScope(node->begin);
        MatchAndConsume(TokenType::kOpenParen);

        if (CurrentToken().type == TokenType::kSemicolon) {
            // for (; ...
            node->init = std::make_unique<NullStatementNode>(current_scope());
            ConsumeToken();
        } else {
            node->init = ParseStatement();
        }

        if (CurrentToken().type != TokenType::kSemicolon) {
            node->condition = ParseExpression(Precedence::kLowest);
        }
        MatchAndConsume(TokenType::kSemicolon);

        if (CurrentToken().type != TokenType::kCloseParen) {
            node->iteration = ParseExpression(Precedence::kLowest);
        }
        MatchAndConsume(TokenType::kCloseParen);

        node->body = ParseStatement();
        if (node->body != nullptr) {
            node->end = node->body->end;
        } else {
            node->end = GetPreviousTokenEnd();
        }

        node->internal_scope = current_scope();
        LeaveScope(node->end);
        return node;
    }

    std::unique_ptr<DoStatementNode> Parser::ParseDoStatement() {
        // current token is "do"
        auto node = std::make_unique<DoStatementNode>(current_scope());
        node->begin = CurrentToken().location;
        ConsumeToken();

        EnterScope(node->begin);
        node->body = ParseStatement();

        if (CurrentToken().text == "while") {
            ConsumeToken();
            MatchAndConsume(TokenType::kOpenParen);
            node->condition = ParseExpression(Precedence::kLowest);
            MatchAndConsume(TokenType::kCloseParen);
        }

        node->end = GetCurrentTokenEnd();
        MatchAndConsume(TokenType::kSemicolon);

        node->internal_scope = current_scope();
        LeaveScope(node->end);
        return node;
    }

    std::unique_ptr<WhileStatementNode> Parser::ParseWhileStatement() {
        // current token is "while"
        auto node = std::make_unique<WhileStatementNode>(current_scope());
        node->begin = CurrentToken().location;
        ConsumeToken();

        EnterScope(node->begin);

        MatchAndConsume(TokenType::kOpenParen);
        node->condition = ParseExpression(Precedence::kLowest);
        MatchAndConsume(TokenType::kCloseParen);

        node->body = ParseStatement();

        if (node->body != nullptr) {
            node->end = node->body->end;
        } else {
            node->end = GetPreviousTokenEnd();
        }

        node->internal_scope = current_scope();
        LeaveScope(node->end);
        return node;
    }

    std::unique_ptr<SwitchStatementNode> Parser::ParseSwitchStatement() {
        // current token is "switch"
        auto node = std::make_unique<SwitchStatementNode>(current_scope());
        node->begin = CurrentToken().location;
        ConsumeToken();

        MatchAndConsume(TokenType::kOpenParen);
        node->condition = ParseExpression(Precedence::kLowest);
        MatchAndConsume(TokenType::kCloseParen);

        if (MatchAndConsume(TokenType::kOpenBrace)) {
            EnterScope(node->begin);

            while (CurrentToken().type != TokenType::kEndOfFile && CurrentToken().type != TokenType::kCloseBrace) {
                if (CurrentToken().text == "case" || CurrentToken().text == "default") {
                    node->cases.push_back(ParseCaseLabel());
                } else {
                    // statements outside case/default labels
                    // syntax error, discard them
                    ParseStatement();
                }
            }

            node->end = GetCurrentTokenEnd();
            node->internal_scope = current_scope();
            MatchAndConsume(TokenType::kCloseBrace);
            LeaveScope(node->end);
        }

        return node;
    }

    std::unique_ptr<CaseStatementNode> Parser::ParseCaseLabel() {
        // current token is "case" or "default"
        auto node = std::make_unique<CaseStatementNode>(current_scope());
        const auto& current_token = CurrentToken();
        node->begin = current_token.location;
        ConsumeToken();

        if (current_token.text == "case") {
            node->condition = ParseExpression(Precedence::kLowest);
        } else if (current_token.text == "default") {
            node->condition = nullptr;
        }
        MatchAndConsume(TokenType::kColon);

        while (CurrentToken().type != TokenType::kEndOfFile && CurrentToken().type != TokenType::kCloseBrace &&
               CurrentToken().text != "case" && CurrentToken().text != "default")
        {
            auto statement = ParseStatement();
            if (statement != nullptr) {
                node->body.push_back(std::move(statement));
            }
        }

        if (!node->body.empty()) {
            node->end = node->body.back()->end;
        } else {
            node->end = GetPreviousTokenEnd();
        }

        return node;
    }

    std::unique_ptr<StatementNode> Parser::ParseJumpStatement() {
        // current token is "return", "break", "continue" or "discard"
        const auto& keyword_token = CurrentToken();

        if (keyword_token.text == "return") {
            auto node = std::make_unique<ReturnStatementNode>(current_scope());
            node->begin = keyword_token.location;
            ConsumeToken();

            if (CurrentToken().type != TokenType::kSemicolon) {
                node->return_value = ParseExpression(Precedence::kLowest);
            }

            node->end = GetCurrentTokenEnd();
            MatchAndConsume(TokenType::kSemicolon);
            return node;
        }

        std::unique_ptr<StatementNode> node;
        if (keyword_token.text == "break") {
            node = std::make_unique<BreakStatementNode>(current_scope());
        } else if (keyword_token.text == "continue") {
            node = std::make_unique<ContinueStatementNode>(current_scope());
        } else if (keyword_token.text == "discard") {
            node = std::make_unique<DiscardStatementNode>(current_scope());
        }

        if (node != nullptr) {
            node->begin = keyword_token.location;
            ConsumeToken();
            node->end = GetCurrentTokenEnd();
            MatchAndConsume(TokenType::kSemicolon);
            return node;
        }

        return nullptr;
    }

    std::vector<Token> Parser::CaptureDirectiveTokens(std::size_t directive_physical_line) {
        std::vector<Token> collected;
        if (CurrentToken().type == TokenType::kEndOfFile) {
            return collected;
        }

        while (CurrentToken().type != TokenType::kEndOfFile) {
            const auto& current_token = CurrentToken();
            if (current_token.location.line > directive_physical_line) {
                // #define MACRO sth "\"
                if (!collected.empty() && collected.back().type == TokenType::kBackslash) {
                    directive_physical_line = current_token.location.line;
                } else {
                    break;
                }
            }

            collected.push_back(current_token);
            ConsumeToken();
        }

        return collected;
    }

    bool Parser::MatchAndConsume(TokenType type) {
        if (CurrentToken().type == type) {
            ConsumeToken();
            return true;
        }

        return false;
    }

    template <typename Ty>
    concept IsVector = requires {
        typename Ty::value_type;
    } && std::same_as<Ty, std::vector<typename Ty::value_type>>;

    template <typename Ty>
    std::vector<std::unique_ptr<Ty>> Parser::ParseSequence(TokenType terminator, auto parse_func, bool consume_terminator) {
        std::vector<std::unique_ptr<Ty>> nodes;

        while (CurrentToken().type != TokenType::kEndOfFile && CurrentToken().type != terminator) {
            auto result = parse_func();

            if constexpr (IsVector<decltype(result)>) {
                for (auto& node : result) {
                    if (node != nullptr) {
                        nodes.push_back(std::move(node));
                    }
                }
            } else {
                if (result != nullptr) {
                    nodes.push_back(std::move(result));
                }
            }
        }

        if (consume_terminator) {
            MatchAndConsume(terminator);
        }

        return nodes;
    }

    Scope* Parser::EnterScope(SourceLocation location, ScopeKind kind) {
        auto new_scope = std::make_unique<Scope>(current_scope());

        Scope* new_scope_ptr           = new_scope.get();
        new_scope_ptr->kind_           = kind;
        new_scope_ptr->interval_.first = location;

        current_scope()->children_.push_back(std::move(new_scope));
        scope_stack_.push(new_scope_ptr);

        return new_scope_ptr;
    }

    void Parser::LeaveScope(SourceLocation location) {
        if (scope_stack_.size() > 1) {
            current_scope()->interval_.second = location;
            scope_stack_.pop();
        }
    }

    std::string Parser::MangleFunctionName(std::string_view base_name, std::span<const std::string> param_typenames) {
        std::string mangled_name(base_name);
        mangled_name += "(";

        const std::size_t typename_size = param_typenames.size();
        for (std::size_t i = 0; i != typename_size; ++i) {
            mangled_name += param_typenames[i];
            mangled_name += i == typename_size - 1 ? ")" : ", ";
        }

        return mangled_name;
    }
}
