#include "stdafx.h"
#include "Parser.hpp"

#include <algorithm>
#include <concepts>
#include <format>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <utility>

#include <magic_enum/magic_enum_all.hpp>
#include "Analyzer/Syntax/Preprocessor.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    Parser::Parser(SourceTable& source_table,
                   const SourceFile* source_file,
                   std::string_view source,
                   IncludeLoader& include_loader,
                   std::span<const std::filesystem::path> include_dirs,
                   int version_replica,
                   std::shared_ptr<const std::atomic<int>> version_pointer,
                   Document& document)

        : source_file_{ source_file }
        , lexer_{ source_file_, source, include_loader, include_dirs }
        , version_replica_{ version_replica }
        , version_pointer_{ version_pointer }
        , document_{ document }
    {
        raw_tokens_.reserve(source.length() / 5);

        do {
            if (version_pointer_ != nullptr && version_replica != version_pointer_->load(std::memory_order::relaxed)) {
                throw std::runtime_error("Lexing cancelled due to version modified.");
            }

            raw_tokens_.push_back(lexer_.AcquireNextToken());
        } while (raw_tokens_.back().type != TokenType::kEndOfFile);

        Preprocessor processor(source_table, source_file_, include_loader, include_dirs, raw_tokens_, document_);
        expanded_tokens_ = processor.Process();

        Parse();
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

    void Parser::Parse() {
        document_.symbols.root_scope()->kind_ = ScopeKind::kGlobalTransparent;
        scope_stack_.push(document_.symbols.root_scope());

        auto ast_root = ParserMainTask();
        if (ast_root == nullptr) {
            return;
        }

        document_.ast = std::move(ast_root);
        document_.raw_tokens = std::move(raw_tokens_);
        document_.expanded_tokens = std::move(expanded_tokens_);

        document_.ast->preprocessor_references = std::move(preprocessor_references_);
    }

    std::unique_ptr<TranslationUnitNode> Parser::ParserMainTask() {
        auto root = std::make_unique<TranslationUnitNode>(current_scope());
        root->begin = SourceLocation(source_file_, 1, 1);

        while (current_token().type != TokenType::kEndOfFile) {
            if (version_pointer_ != nullptr && version_replica_ != version_pointer_->load(std::memory_order::relaxed)) {
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
        std::vector<std::unique_ptr<AttributeNode>> attributes;
        if (current_token().type == TokenType::kOpenBracket && PeekToken().type == TokenType::kOpenBracket) {
            attributes = ParseAttributeList();
        }

        std::unique_ptr<StatementNode> node;
        switch (current_token().type) {
        case TokenType::kPrimitive:
        case TokenType::kBuiltInType:
        case TokenType::kBuiltInFunction:
        case TokenType::kBuiltInVariable:
        case TokenType::kIdentifier:
        case TokenType::kSpirvIntrinsics:
            node = ParseCodeStatement();
            break;
        case TokenType::kKeyword:
            node = ParseControlFlowStatement();
            break;
        case TokenType::kOpenBrace:
            node = ParseScope();
            break;
        case TokenType::kOpenParen:
            node = ParseExpressionStatement();
            break;
        case TokenType::kSharp:
            node = ParsePreprocessor();
            break;
        default:
            while (current_token().type != TokenType::kEndOfFile && current_token().type != TokenType::kSemicolon &&
                   current_token().type != TokenType::kOpenBrace || current_token().type == TokenType::kCloseBrace)
            {
                ConsumeToken();
            }

            MatchAndConsume(TokenType::kSemicolon);
            return std::make_unique<NullStatementNode>(current_scope());
        }

        if (node != nullptr && !attributes.empty()) {
            node->attributes = std::move(attributes);
        }

        return node;
    }

    std::unique_ptr<PreprocessorNode> Parser::ParsePreprocessor() {
        // current token is '#'
        auto node = std::make_unique<PreprocessorNode>(current_scope());
        node->begin = current_token().location;
        MatchAndConsume(TokenType::kSharp);

        if (current_token().type == TokenType::kEndOfFile) {
            return nullptr;
        }

        auto& directive_token = current_token();
        node->directive = std::string(directive_token.text);
        if (directive_token.type == TokenType::kKeyword) {
            // #if defined, #else
            if (directive_token.text == "if" || directive_token.text == "else") {
                expanded_tokens_[token_index_].type = TokenType::kPreprocessor;
            }
        }
        ConsumeToken();

        if (directive_token.text == "define") {
            node = ParseDefine(std::move(node), directive_token.location.filename(), directive_token.location.line());
        } else {
            node->tokens = CaptureDirectiveTokens(directive_token.location.filename(), directive_token.location.line());
            node->end    = GetPreviousTokenEnd();
        }

        preprocessor_references_.push_back(node.get());
        return node;
    }

    std::unique_ptr<PreprocessorNode> Parser::ParseDefine(
        std::unique_ptr<PreprocessorNode> node,
        std::string_view target_file,
        std::size_t directive_physical_line)
    {
        // current token is macro name after "define"
        const auto& macro_token = current_token();

        if (macro_token.location.line() != directive_physical_line) {
            return node;
        }

        node->symbol = document_.symbols.root_scope()->AddSymbol(node.get(), macro_token.text, macro_token.location, SymbolKind::kMacro);
        ConsumeToken();

        auto IsAdjacent = [](const Token& first, const Token& second) -> bool {
            return first.location.line() == second.location.line() &&
                (first.location.column() + first.text.length() == second.location.column());
        };

        // macro function like #define MACRO(x)
        if (current_token().type == TokenType::kOpenParen && IsAdjacent(macro_token, current_token())) {
            ConsumeToken();
            while (current_token().type != TokenType::kEndOfFile && current_token().type != TokenType::kCloseParen) {
                if (current_token().type == TokenType::kIdentifier) {
                    node->params.push_back(current_token().text);
                    ConsumeToken();
                }
                if (!MatchAndConsume(TokenType::kComma)) {
                    break;
                }
            }

            MatchAndConsume(TokenType::kCloseParen);
        }

        node->tokens = CaptureDirectiveTokens(target_file, directive_physical_line);
        node->end    = GetPreviousTokenEnd();
        node->body   = ParseMacroBody(node->tokens, node->symbol);

        return node;
    }

    std::vector<std::unique_ptr<StatementNode>> Parser::ParseMacroBody(std::span<const Token> body_tokens, const SymbolInfo* host_symbol) {
        std::vector<std::unique_ptr<StatementNode>> statements;
        if (body_tokens.empty()) {
            return statements;
        }

        auto saved_tokens      = std::move(expanded_tokens_);
        auto saved_index       = token_index_;
        auto saved_scope_depth = scope_stack_.size();

        std::vector<Token> local_tokens = std::ranges::to<std::vector<Token>>(body_tokens);
        local_tokens.push_back({
            .text     = {},
            .location = local_tokens.back().location,
            .type     = TokenType::kEndOfFile
        });

        expanded_tokens_ = std::move(local_tokens);
        token_index_     = 0;

        EnterScope(body_tokens.front().location, host_symbol, ScopeKind::kMacroTemporary);

        while (current_token().type != TokenType::kEndOfFile) {
            const auto& token = current_token();

            if (token.type == TokenType::kBackslash) {
                ConsumeToken();
                continue;
            }

            auto before    = token_index_;
            auto statement = ParseStatement();
            if (statement != nullptr) {
                statements.push_back(std::move(statement));
            } else {
                ConsumeToken();
            }

            if (token_index_ == before) {
                ConsumeToken();
            }
        }

        LeaveScope(GetPreviousTokenEnd());

        while (scope_stack_.size() > saved_scope_depth) {
            scope_stack_.pop();
        }

        expanded_tokens_ = std::move(saved_tokens);
        token_index_     = saved_index;

        return statements;
    }

    std::unique_ptr<CompoundStatementNode> Parser::ParseScope(const SymbolInfo* host_symbol, ScopeKind kind) {
        auto node = std::make_unique<CompoundStatementNode>(current_scope());
        node->begin = current_token().location;
        MatchAndConsume(TokenType::kOpenBrace);

        node->internal_scope = EnterScope(node->begin, host_symbol, kind);
        node->children = ParseSequence<StatementNode>(TokenType::kCloseBrace, [this]() -> std::unique_ptr<StatementNode> {
            return ParseStatement();
        }, false);

        node->end = GetCurrentTokenEnd();
        MatchAndConsume(TokenType::kCloseBrace);

        LeaveScope(node->end);
        return node;
    }

    std::vector<std::unique_ptr<AttributeNode>> Parser::ParseAttributeList() {
        if (current_token().type != TokenType::kOpenBracket || PeekToken().type != TokenType::kOpenBracket) {
            return {};
        }

        std::vector<std::unique_ptr<AttributeNode>> attributes;

        do {
            // current token is [[ if in the first loop, or , in the following loops
            MatchAndConsume(TokenType::kOpenBracket);
            MatchAndConsume(TokenType::kOpenBracket);
            MatchAndConsume(TokenType::kComma);
            auto node = std::make_unique<AttributeNode>(current_scope());
            node->begin = current_token().location;

            if (PeekToken().type == TokenType::kColonColon) {
                node->namespace_ = current_token();
                ConsumeToken();
            }

            const auto& name_token = current_token();
            if (name_token.type == TokenType::kIdentifier) {
                node->name = name_token;

                const auto* attribute_symbol =
                    document_.symbols.root_scope()->AddSymbol(node.get(), name_token.text, name_token.location, SymbolKind::kAttribute);
                document_.bindings.try_emplace(name_token.location, attribute_symbol);
            }

            ConsumeToken();

            if (MatchAndConsume(TokenType::kOpenParen)) {
                // current token is attribute argument
                node->argument = ParseExpression(Precedence::kLowest);
                MatchAndConsume(TokenType::kCloseParen);
            }

            node->end = GetCurrentTokenEnd();
            attributes.push_back(std::move(node));
        } while (current_token().type == TokenType::kComma);

        MatchAndConsume(TokenType::kCloseBracket);
        MatchAndConsume(TokenType::kCloseBracket);
        return attributes;
    }

    std::unique_ptr<StatementNode> Parser::ParseCodeStatement() {
        // current token is qualifier, type or identifier
        auto statement_begin_index = token_index_;
        auto type_spec = ParseQualifiersAndType();

        if (!type_spec.empty()) {
            // block, current is identifier, and next is '{', or current is '{'
            if ((current_token().type == TokenType::kIdentifier && PeekToken().type == TokenType::kOpenBrace) ||
                (current_token().type == TokenType::kOpenBrace  && type_spec.has_keyword("struct")))
            {
                return ParseBlockBody(std::move(type_spec));
            }

            // function, current is identifier, and next is '('
            if (current_token().type == TokenType::kIdentifier && PeekToken().type == TokenType::kOpenParen) {
                return ParseFunction(std::move(type_spec));
            }

            // common variable
            if (current_token().type != TokenType::kOpenParen) {
                return ParseVariableDeclarationList(std::move(type_spec));
            }
        }

        // expression, including function calling
        const auto& token = current_token();
        bool common_calling = type_spec.empty() &&
            (token.type == TokenType::kIdentifier ||
             token.type == TokenType::kBuiltInFunction ||
             token.type == TokenType::kSpirvIntrinsics);

        bool constructor       = !type_spec.empty() && token.type == TokenType::kOpenParen;
        bool is_expr_primitive = token.text == "true" || token.text == "false";

        if (common_calling || constructor || is_expr_primitive) {
            if (constructor) {
                token_index_ = statement_begin_index;
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

        const auto& name_token = current_token();
        ConsumeToken();

        const auto& begin_location = current_token().location;
        MatchAndConsume(TokenType::kOpenParen);
        auto* located_scope  = current_scope();
        auto* internal_scope = EnterScope(begin_location);

        node->params = ParseParameterList();

        // mangle function name, such as "Func(int array[5], in vec3 v) -> Func(int[5], in vec3)"
        std::vector<std::string> param_typenames;
        for (const auto& param : node->params) {
            std::string param_typename;

            for (const auto& specifier : param->type_spec.specifiers) {
                if (!param_typename.empty()) {
                    param_typename += " ";
                }

                if (specifier.text != "spirv_type") {
                    param_typename += specifier.text;
                } else if (param->type_spec.spirv_type != nullptr) {
                    auto parameters = utils::BuildQualifierParameterList(param->type_spec.spirv_type.get());
                    param_typename += std::format("spirv_type({})", parameters);
                }
            }

            for (const auto& array_size : param->type_spec.array_sizes) {
                if (array_size == nullptr) {
                    param_typename += "[]";
                    continue;
                }

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
        if (current_token().type == TokenType::kOpenBracket && PeekToken().type == TokenType::kOpenBracket) {
            node->attributes = ParseAttributeList();
        }

        SymbolKind kind{};
        if (current_token().type == TokenType::kOpenBrace) {
            // function body
            function_name = std::format("__Impl_{}", function_name);
            kind = SymbolKind::kFunctionImpl;

            auto body_node = std::make_unique<CompoundStatementNode>(located_scope);
            body_node->internal_scope = current_scope();
            body_node->begin = current_token().location;

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
        node->declared_symbol->internal_scope->host_symbol_ = node->declared_symbol;
        node->internal_scope = internal_scope;
        node->internal_scope->host_symbol_ = node->declared_symbol;

        document_.symbols.AddFunctionBaseName(name_token.text, node->declared_symbol);

        return node;
    }

    std::vector<std::unique_ptr<VariableDeclarationNode>> Parser::ParseParameterList() {
        // current token is first parameter or "void"
        std::vector<std::unique_ptr<VariableDeclarationNode>> param_list;

        // Function(void)
        if (current_token().type == TokenType::kPrimitive &&
            current_token().text == "void" &&
            PeekToken().type == TokenType::kCloseParen)
        {
            const auto& token = current_token();
            auto node = std::make_unique<VariableDeclarationNode>(current_scope());

            node->type_spec.specifiers.push_back(token);
            node->begin = token.location;
            node->end   = GetCurrentTokenEnd();

            ConsumeToken();
            param_list.push_back(std::move(node));
            return param_list;
        }

        while (current_token().type != TokenType::kEndOfFile && current_token().type != TokenType::kCloseParen) {
            auto type_spec  = ParseQualifiersAndType();
            auto node       = std::make_unique<VariableDeclarationNode>(current_scope());
            node->begin     = type_spec.begin_location();
            node->type_spec = std::move(type_spec);

            if (current_token().type == TokenType::kIdentifier) {
                const auto& name_token = current_token();
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
            auto virtual_void_node = std::make_unique<VariableDeclarationNode>(current_scope());
            const auto& token      = current_token();

            virtual_void_node->type_spec.specifiers.emplace_back("void", token.location, TokenType::kPrimitive);
            virtual_void_node->begin = token.location;
            virtual_void_node->end   = token.location;

            param_list.push_back(std::move(virtual_void_node));
        }

        return param_list;
    }

    TypeSpecifier Parser::ParseQualifiersAndType() {
        // current token is specifier such as: Func("const int" input)
        TypeSpecifier type_spec;

        while (true) {
            const auto& token = current_token();

            if (token.type == TokenType::kPrimitive && TryParseLayoutQualifier(type_spec))
                continue;
            if (token.type == TokenType::kSpirvIntrinsics && TryParseSpirvIntrinsics(type_spec))
                continue;

            if (token.type == TokenType::kPrimitive || token.type == TokenType::kBuiltInType) {
                // (in, out, uniform, const, struct, ...)
                // (vec3, mat4, float, ...)
                if (token.text == "true" || token.text == "false") {
                    break;
                }

                type_spec.specifiers.push_back(token);
                ConsumeToken();

                if (MatchAndConsume(TokenType::kLessThan)) { // coopmat<float16_t, gl_ScopeSubgroup, M, N, gl_MatrixUseA>;
                    while (current_token().type != TokenType::kEndOfFile &&
                           current_token().type != TokenType::kGreaterThan)
                    {
                        const auto& arg_token = current_token();
                        if (arg_token.type == TokenType::kComma) {
                            ConsumeToken();
                            continue;
                        }

                        type_spec.template_args.push_back(arg_token);
                        ConsumeToken();
                    }

                    MatchAndConsume(TokenType::kGreaterThan);
                }

                continue;
            }

            if (token.type == TokenType::kIdentifier) {
                const auto* symbol_info = current_scope()->FindSymbol(token.text);
                if (symbol_info == nullptr ||
                    (symbol_info->kind != SymbolKind::kStruct && symbol_info->kind != SymbolKind::kInterface))
                {
                    break; // 不是类型标识符
                }

                if (PeekToken().type != TokenType::kIdentifier) {
                    break;
                }

                type_spec.specifiers.push_back(token);
                ConsumeToken();
                continue;
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

    std::vector<Token> Parser::CaptureBalancedTokens(TokenType open, TokenType close) {
        std::vector<Token> captured;
        if (!MatchAndConsume(open)) {
            return captured;
        }

        int level = 1;
        while (current_token().type != TokenType::kEndOfFile && level > 0) {
            const auto& token = current_token();

            if (token.type == open) {
                ++level;
                captured.push_back(token);
                ConsumeToken();
                continue;
            }

            if (token.type == close) {
                --level;
                if (level == 0) {
                    ConsumeToken(); // consume final close
                    break;
                }

                captured.push_back(token);
                ConsumeToken();
                continue;
            }

            captured.push_back(token);
            ConsumeToken();
        }

        return captured;
    }

    namespace {
        QualifierArgumentKind ResolveQualifierArgumentKind(const Token& token) {
            if (token.type == TokenType::kNumberLiteral)
                return QualifierArgumentKind::kNumberLiteral;
            if (token.type == TokenType::kStringLiteral)
                return QualifierArgumentKind::kStringLiteral;
            if (token.text == "true" || token.text == "false")
                return QualifierArgumentKind::kBoolLiteral;

            if (token.type == TokenType::kIdentifier      ||
                token.type == TokenType::kPrimitive       ||
                token.type == TokenType::kBuiltInType     ||
                token.type == TokenType::kBuiltInFunction ||
                token.type == TokenType::kBuiltInVariable ||
                token.type == TokenType::kSpirvIntrinsics)
            {
                return QualifierArgumentKind::kIdentifier;
            }

            return QualifierArgumentKind::kUnknown;
        }

        std::span<const Token> Trim(std::span<const Token> slice) {
            while (!slice.empty() && slice.front().type == TokenType::kComma) {
                slice = slice.subspan(1);
            }

            while (!slice.empty() && slice.back().type == TokenType::kComma) {
                slice = slice.first(slice.size() - 1);
            }

            return slice;
        };

        void ProcessBracketLevel(int& paren_level, int& bracket_level, int& brace_level, TokenType type) {
            if (type == TokenType::kOpenParen)
                ++paren_level;
            else if (type == TokenType::kCloseParen)
                --paren_level;
            else if (type == TokenType::kOpenBracket)
                ++bracket_level;
            else if (type == TokenType::kCloseBracket)
                --bracket_level;
            else if (type == TokenType::kOpenBrace)
                ++brace_level;
            else if (type == TokenType::kCloseBrace)
                --brace_level;
        }

        std::vector<std::span<const Token>> SplitTopLevel(std::span<const Token> slice, TokenType delimiter) {
            int paren_level   = 0;
            int bracket_level = 0;
            int brace_level   = 0;

            std::vector<std::span<const Token>> parts;
            auto begin = 0uz;
            for (auto i = 0uz; i != slice.size(); ++i) {
                const auto type = slice[i].type;
                ProcessBracketLevel(paren_level, bracket_level, brace_level, type);

                if (paren_level == 0 && bracket_level == 0 && brace_level == 0 && type == delimiter) {
                    parts.push_back(slice.subspan(begin, i - begin));
                    begin = i + 1;
                }
            }

            if (begin <= slice.size()) {
                parts.push_back(slice.subspan(begin));
            }

            return parts;
        };

        std::optional<std::size_t> FindTopLevel(std::span<const Token> slice, TokenType target) {
            int paren_level   = 0;
            int bracket_level = 0;
            int brace_level   = 0;

            for (auto i = 0uz; i != slice.size(); ++i) {
                const auto type = slice[i].type;
                ProcessBracketLevel(paren_level, bracket_level, brace_level, type);

                if (paren_level == 0 && bracket_level == 0 && brace_level == 0 && type == target) {
                    return i;
                }
            }

            return std::nullopt;
        };

        bool IsWrappedBy(std::span<const Token> slice, TokenType open, TokenType close) {
            if (slice.size() < 2 || slice.front().type != open || slice.back().type != close) {
                return false;
            }

            int level = 0;
            for (auto i = 0uz; i != slice.size(); ++i) {
                if (slice[i].type == open) {
                    ++level;
                } else if (slice[i].type == close) {
                    --level;
                    if (level == 0 && i + 1 != slice.size()) {
                        return false;
                    }
                }
            }

            return level == 0;
        };
    }

    std::shared_ptr<QualifierArgumentNode> Parser::ParseQualifierArguments(std::span<const Token> tokens) {
        auto ComputeTokenEnd = [](const Token& token) -> SourceLocation {
            return SourceLocation(
                token.location.source_file(),
                token.location.line(),
                token.location.column() + token.text.length()
            );
        };

        auto MakeLeaf = [this, &ComputeTokenEnd](const Token& token) -> std::shared_ptr<QualifierArgumentNode> {
            auto node = std::make_shared<QualifierArgumentNode>(current_scope());

            node->begin         = token.location;
            node->arg_kind      = ResolveQualifierArgumentKind(token);
            node->token         = token;
            node->end           = ComputeTokenEnd(token);
            node->located_scope = current_scope();

            return node;
        };

        auto FinalizeRangeFromChildren = [this, &ComputeTokenEnd](QualifierArgumentNode* node) -> void {
            if (node == nullptr) {
                return;
            }

            node->located_scope = current_scope();

            if (!node->children.empty()) {
                node->begin = node->children.front()->begin;
                node->end   = node->children.back()->end;
            } else {
                node->begin = node->token.location;
                node->end   = ComputeTokenEnd(node->token);
            }
        };

        auto Build = [this, &MakeLeaf, &FinalizeRangeFromChildren](this auto&& self, std::span<const Token> raw_slice)
            -> std::shared_ptr<QualifierArgumentNode> {
            auto slice = Trim(raw_slice);
            if (slice.empty()) {
                return nullptr;
            }

            if (slice.size() == 1) {
                return MakeLeaf(slice.front());
            }

            if (IsWrappedBy(slice, TokenType::kOpenBracket, TokenType::kCloseBracket)) {
                auto node = std::make_shared<QualifierArgumentNode>(current_scope());
                node->arg_kind = QualifierArgumentKind::kArray;
                node->token    = slice.front();

                auto inner = slice.subspan(1, slice.size() - 2);
                auto parts = SplitTopLevel(inner, TokenType::kComma);
                for (auto part : parts) {
                    auto child = self(part);
                    if (child != nullptr) {
                        node->children.push_back(std::move(child));
                    }
                }

                FinalizeRangeFromChildren(node.get());
                return node;
            }

            if (IsWrappedBy(slice, TokenType::kOpenParen, TokenType::kCloseParen)) {
                auto node = std::make_shared<QualifierArgumentNode>(current_scope());
                node->arg_kind = QualifierArgumentKind::kGroup;
                node->token    = slice.front();

                auto inner = slice.subspan(1, slice.size() - 2);
                auto child = self(inner);
                if (child != nullptr) {
                    node->children.push_back(std::move(child));
                }

                FinalizeRangeFromChildren(node.get());
                return node;
            }

            if (auto equal_pos = FindTopLevel(slice, TokenType::kEqual)) {
                auto node = std::make_shared<QualifierArgumentNode>(current_scope());
                node->arg_kind = QualifierArgumentKind::kAssignment;
                node->token    = slice[*equal_pos];

                auto lhs = self(slice.first(*equal_pos));
                auto rhs = self(slice.subspan(*equal_pos + 1));

                if (lhs != nullptr)
                    node->children.push_back(std::move(lhs));
                if (rhs != nullptr)
                    node->children.push_back(std::move(rhs));

                FinalizeRangeFromChildren(node.get());
                return node;
            }

            auto node = std::make_shared<QualifierArgumentNode>(current_scope());
            node->arg_kind = QualifierArgumentKind::kSequence;
            node->token    = slice.front();

            for (const auto& token : slice) {
                node->children.push_back(MakeLeaf(token));
            }

            FinalizeRangeFromChildren(node.get());
            return node;
        };

        return Build(tokens);
    }

    std::shared_ptr<LayoutQualifierNode> Parser::ParseLayoutQualifier() {
        // current token is "layout"
        const auto& token = current_token();
        if (token.type != TokenType::kPrimitive || token.text != "layout") {
            return nullptr;
        }

        auto node = std::make_shared<LayoutQualifierNode>(current_scope());
        node->begin = token.location;
        node->end   = GetCurrentTokenEnd();
        ConsumeToken();

        if (current_token().type != TokenType::kOpenParen) {
            return node;
        }

        node->raw_tokens = CaptureBalancedTokens(TokenType::kOpenParen, TokenType::kCloseParen);

        auto parts = SplitTopLevel(node->raw_tokens, TokenType::kComma);
        for (auto part : parts) {
            auto param = ParseQualifierArguments(part);
            if (param != nullptr) {
                node->params.push_back(std::move(param));
            }
        }

        node->end = GetPreviousTokenEnd(); // CaptureBalancedTokens has consumed ')'
        return node;
    }

    namespace {
        SpirvIntrinsicKind ResolveSpirvIntrinsicKind(std::string_view intrinsic) {
            if (intrinsic == "spirv_type") {
                return SpirvIntrinsicKind::kTypeOverride;
            }

            if (intrinsic == "spirv_decorate"        ||
                intrinsic == "spirv_decorate_id"     ||
                intrinsic == "spirv_decorate_string" ||
                intrinsic == "spirv_storage_class"   ||
                intrinsic == "spirv_by_reference"    ||
                intrinsic == "spirv_literal"         ||
                intrinsic == "spirv_id")
            {
                return SpirvIntrinsicKind::kQualifier;
            }

            if (intrinsic == "spirv_instruction"    ||
                intrinsic == "spirv_execution_mode" ||
                intrinsic == "spirv_execution_mode_id")
            {
                return SpirvIntrinsicKind::kInstruction;
            }

            return SpirvIntrinsicKind::kUnknown;
        }
    }

    std::shared_ptr<SpirvIntrinsicNode> Parser::ParseSpirvIntrinsics() {
        // current token is SPIR-V intrinsic keyword
        if (current_token().type != TokenType::kSpirvIntrinsics) {
            return nullptr;
        }

        const auto& keyword = current_token();

        auto node            = std::make_shared<SpirvIntrinsicNode>(current_scope());
        node->keyword        = keyword;
        node->intrinsic_kind = ResolveSpirvIntrinsicKind(keyword.text);
        node->begin          = keyword.location;
        node->end            = GetCurrentTokenEnd();

        ConsumeToken();

        if (current_token().type != TokenType::kOpenParen) {
            return node;
        }

        node->raw_tokens = CaptureBalancedTokens(TokenType::kOpenParen, TokenType::kCloseParen);

        auto parts = SplitTopLevel(node->raw_tokens, TokenType::kComma);
        for (auto part : parts) {
            auto param = ParseQualifierArguments(part);
            if (param != nullptr) {
                node->params.push_back(std::move(param));
            }
        }

        node->end = GetPreviousTokenEnd(); // CaptureBalancedTokens has consumed ')'
        return node;
    }

    bool Parser::TryParseLayoutQualifier(TypeSpecifier& type_spec) {
        // layout(...)
        const auto& token = current_token();
        if (token.text != "layout") {
            return false;
        }

        type_spec.specifiers.push_back(token);

        auto node = ParseLayoutQualifier();
        if (node == nullptr) {
            return false;
        }

        type_spec.layouts.push_back(node);
        return true;
    }

    bool Parser::TryParseSpirvIntrinsics(TypeSpecifier& type_spec) {
        auto node = ParseSpirvIntrinsics();
        if (node == nullptr) {
            return false;
        }

        type_spec.specifiers.push_back(node->keyword);
        type_spec.spirv_intrinsics.push_back(node);

        if (node->intrinsic_kind == SpirvIntrinsicKind::kTypeOverride) {
            type_spec.spirv_type = node;
        }

        return true;
    }

    std::unique_ptr<DeclarationGroupNode> Parser::ParseVariableDeclarationList(TypeSpecifier type_spec) {
        // current token is variable name or semicolon
        if (current_token().type == TokenType::kSemicolon) {
            auto node   = std::make_unique<DeclarationGroupNode>(current_scope());
            node->begin = type_spec.begin_location();
            node->end   = GetCurrentTokenEnd();

            ConsumeToken();
            return node;
        }

        auto ParseSingleDeclarer = [&]() -> std::unique_ptr<VariableDeclarationNode> {
            if (current_token().type != TokenType::kIdentifier) {
                return nullptr;
            }

            const auto& name_token = current_token();
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

        while (current_token().type == TokenType::kComma) {
            ConsumeToken();
            if (auto next_node = ParseSingleDeclarer()) {
                node->declarations.push_back(std::move(next_node));
            } else {
                break;
            }
        }

        // terminate with semicolon
        if (current_token().type == TokenType::kSemicolon) {
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
        node->begin = current_token().location;
        node->expr  = ParseExpression(Precedence::kLowest);

        if (current_token().type == TokenType::kSemicolon) {
            node->end = GetCurrentTokenEnd();
            MatchAndConsume(TokenType::kSemicolon);
        } else {
            node->end = GetPreviousTokenEnd();
        }

        return node;
    }

    std::unique_ptr<ExpressionNode> Parser::ParsePrefixExpression() {
        const auto& token = current_token();

        // ( expr )
        if (token.type == TokenType::kOpenParen) {
            ConsumeToken();
            auto expr_node = ParseExpression(Precedence::kLowest);

            if (expr_node != nullptr) {
                expr_node->begin = token.location;
                expr_node->end   = GetCurrentTokenEnd();
            }

            MatchAndConsume(TokenType::kCloseParen);
            return expr_node;
        }

        switch (token.type) {
        // 字面量
        case TokenType::kNumberLiteral:
        case TokenType::kStringLiteral:
            return ParseLiteral();

        // 标识符/内置类型/内置函数/Primitive/SpirvIntrinsics (函数本来应该算标识符，但问题是内置函数无需声明)
        case TokenType::kIdentifier:
        case TokenType::kBuiltInType:
        case TokenType::kBuiltInFunction:
        case TokenType::kPrimitive:
        case TokenType::kSpirvIntrinsics:
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
        const auto& token = current_token();

        node->begin = token.location;
        node->tokens.push_back(token);
        ConsumeToken();
        node->end = GetPreviousTokenEnd();

        return node;
    }

    std::unique_ptr<VariableExpressionNode> Parser::ParseVariableReference() {
        auto node = std::make_unique<VariableExpressionNode>(current_scope());
        const auto& token = current_token();

        node->begin          = token.location;
        node->original_token = token;
        node->name           = token.text;

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
        const auto& token = current_token();

        node->begin      = token.location;
        node->op         = token.type;
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
        node->begin = current_token().location;
        MatchAndConsume(TokenType::kOpenBrace);

        if (current_token().type != TokenType::kCloseBrace) {
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

    std::unique_ptr<ExpressionNode>
    Parser::ParseInfixExpression(std::unique_ptr<ExpressionNode> left, TokenType op_type, Precedence precedence) {
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

        // 三元运算符
        case TokenType::kQuestion:
            return ParseTernary(std::move(left));

        // 标准二元运算符 (+, -, *, /, &&, =, etc.)
        default:
            return ParseStandardBinary(std::move(left), op_type, precedence);
        }
    }

    std::unique_ptr<MemberAccessExpressionNode> Parser::ParseMemberAccess(std::unique_ptr<ExpressionNode> object) {
        auto node = std::make_unique<MemberAccessExpressionNode>(current_scope());

        node->begin  = object->begin;
        node->object = std::move(object);

        const auto& member_token = current_token();
        if (member_token.type == TokenType::kIdentifier) {
            if (PeekToken().type != TokenType::kOpenParen) [[likely]] {
                auto member_node            = std::make_unique<VariableExpressionNode>(current_scope());
                member_node->begin          = member_token.location;
                member_node->original_token = member_token;
                member_node->node_type      = VariableExpressionNode::NodeType::kBlockMember;
                member_node->name           = member_token.text;
                member_node->end            = GetCurrentTokenEnd();

                node->member = std::move(member_node);
                ConsumeToken();
            } else { // array.length();, the only member func that GLSL supports for built-in arrays
                expanded_tokens_[token_index_].type = TokenType::kBuiltInFunction;
                auto callee_expr = ParseVariableReference();
                MatchAndConsume(TokenType::kOpenParen); // simulate pratt parsing comsume '(' after callee
                auto callee_node = ParseFunctionCall(std::move(callee_expr));

                node->member = std::move(callee_node);

                TypeDescriptor type_desc{
                    .family        = BaseFamily::kInt,
                    .bits          = 32,
                    .vector_count  = 1,
                    .vector_length = 1
                };

                TypeInfo type_info{
                    .typename_token = {
                        .text     = "int",
                        .location = member_token.location,
                        .type     = TokenType::kPrimitive
                    },
                    .type_desc = type_desc,
                };

                node->evaluated_type = type_info;
            }
        }

        node->end = GetPreviousTokenEnd();
        return node;
    }

    std::unique_ptr<IndexExpressionNode> Parser::ParseArrayIndex(std::unique_ptr<ExpressionNode> base) {
        auto node = std::make_unique<IndexExpressionNode>(current_scope());

        node->begin = base->begin;
        node->base  = std::move(base);
        node->index = ParseExpression(Precedence::kLowest);

        if (current_token().type == TokenType::kCloseBracket) {
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

        if (current_token().type != TokenType::kCloseParen) {
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

    std::unique_ptr<TernaryExpressionNode> Parser::ParseTernary(std::unique_ptr<ExpressionNode> condition) {
        auto node = std::make_unique<TernaryExpressionNode>(current_scope());

        node->begin     = condition->begin;
        node->condition = std::move(condition);
        node->true_expr = ParseExpression(Precedence::kLowest);
        MatchAndConsume(TokenType::kColon);
        node->false_expr = ParseExpression(Precedence::kTernary);

        if (node->false_expr != nullptr) {
            node->end = node->false_expr->end;
        } else {
            node->end = GetPreviousTokenEnd();
        }

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

    std::unique_ptr<BinaryExpressionNode>
    Parser::ParseStandardBinary(std::unique_ptr<ExpressionNode> left, TokenType op_type, Precedence precedence) {
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
            auto op_type = current_token().type;
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
        std::string    name;
        SourceLocation name_location;

        if (current_token().type == TokenType::kIdentifier) {
            // current token is block name
            const auto& block_name = current_token();
            name = block_name.text;
            name_location = block_name.location;
            ConsumeToken();
        } else { // anonymous
            name = std::format("__AnonymousStruct_{}", GetNextAnonymousId());
            name_location = current_token().location;
        }

        bool is_struct = type_spec.has_keyword("struct");

        auto ParseBody = [&](auto& node) -> void {
            node->begin           = type_spec.begin_location();
            auto block_kind       = is_struct ? SymbolKind::kStruct : SymbolKind::kInterface;
            node->declared_symbol = current_scope()->AddSymbol(node.get(), name, name_location, block_kind);
            node->body            = ParseScope(node->declared_symbol, ScopeKind::kBlock);

            if (current_token().type != TokenType::kSemicolon) {
                // struct MyStruct { ... } instance;
                type_spec.specifiers.push_back({
                    .text = name, .location = name_location, .type = TokenType::kIdentifier
                });

                node->instances = ParseVariableDeclarationList(std::move(type_spec));
            } else {
                MatchAndConsume(TokenType::kSemicolon);
            }

            if (!is_struct && node->instances == nullptr) {
                if (node->body != nullptr && node->body->internal_scope != nullptr) {
                    node->body->internal_scope->kind_ = ScopeKind::kBlockTransparent;
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
        const auto& keyword = current_token().text;

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
        node->begin = current_token().location;
        ConsumeToken();

        EnterScope(node->begin);

        MatchAndConsume(TokenType::kOpenParen);
        node->condition = ParseExpression(Precedence::kLowest);
        MatchAndConsume(TokenType::kCloseParen);

        node->then_branch = ParseStatement();

        if (current_token().type == TokenType::kKeyword && current_token().text == "else") {
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
        node->begin = current_token().location;
        ConsumeToken();

        EnterScope(node->begin);
        MatchAndConsume(TokenType::kOpenParen);

        if (current_token().type == TokenType::kSemicolon) {
            // for (; ...
            node->init = std::make_unique<NullStatementNode>(current_scope());
            ConsumeToken();
        } else {
            node->init = ParseStatement();
        }

        if (current_token().type != TokenType::kSemicolon) {
            node->condition = ParseExpression(Precedence::kLowest);
        }
        MatchAndConsume(TokenType::kSemicolon);

        if (current_token().type != TokenType::kCloseParen) {
            node->iteration = ParseExpression(Precedence::kLowest);
        }
        MatchAndConsume(TokenType::kCloseParen);

        node->body = ParseStatement();
        if (node->body->kind() != AstNodeKind::kNullStatement) {
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
        node->begin = current_token().location;
        ConsumeToken();

        EnterScope(node->begin);
        node->body = ParseStatement();

        if (current_token().text == "while") {
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
        node->begin = current_token().location;
        ConsumeToken();

        EnterScope(node->begin);

        MatchAndConsume(TokenType::kOpenParen);
        node->condition = ParseExpression(Precedence::kLowest);
        MatchAndConsume(TokenType::kCloseParen);

        node->body = ParseStatement();

        if (node->body->kind() != AstNodeKind::kNullStatement) {
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
        node->begin = current_token().location;
        ConsumeToken();

        MatchAndConsume(TokenType::kOpenParen);
        node->condition = ParseExpression(Precedence::kLowest);
        MatchAndConsume(TokenType::kCloseParen);

        if (MatchAndConsume(TokenType::kOpenBrace)) {
            EnterScope(node->begin);

            while (current_token().type != TokenType::kEndOfFile && current_token().type != TokenType::kCloseBrace) {
                if (current_token().text == "case" || current_token().text == "default") {
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
        const auto& token = current_token();
        node->begin = token.location;
        ConsumeToken();

        if (token.text == "case") {
            node->condition = ParseExpression(Precedence::kLowest);
        } else if (token.text == "default") {
            node->condition = nullptr;
        }
        MatchAndConsume(TokenType::kColon);

        while (current_token().type != TokenType::kEndOfFile && current_token().type != TokenType::kCloseBrace &&
               current_token().text != "case" && current_token().text != "default")
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
        const auto& keyword_token = current_token();

        if (keyword_token.text == "return") {
            auto node = std::make_unique<ReturnStatementNode>(current_scope());
            node->begin = keyword_token.location;
            ConsumeToken();

            if (current_token().type != TokenType::kSemicolon) {
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

    std::vector<Token> Parser::CaptureDirectiveTokens(std::string_view target_file, std::size_t directive_physical_line) {
        std::vector<Token> collected;
        if (current_token().type == TokenType::kEndOfFile) {
            return collected;
        }

        while (current_token().type != TokenType::kEndOfFile) {
            const auto& token = current_token();
            if (token.location.filename() != target_file) {
                break;
            }

            if (token.location.line() > directive_physical_line) {
                // #define MACRO sth "\"
                if (!collected.empty() && collected.back().type == TokenType::kBackslash) {
                    directive_physical_line = token.location.line();
                } else {
                    break;
                }
            }

            collected.push_back(token);
            ConsumeToken();
        }

        return collected;
    }

    template <typename Ty>
    concept IsVector = requires {
        typename Ty::value_type;
    } && std::same_as<Ty, std::vector<typename Ty::value_type>>;

    template <typename Ty>
    std::vector<std::unique_ptr<Ty>> Parser::ParseSequence(TokenType terminator, auto parse_func, bool consume_terminator) {
        std::vector<std::unique_ptr<Ty>> nodes;

        while (current_token().type != TokenType::kEndOfFile && current_token().type != terminator) {
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

    Scope* Parser::EnterScope(const SourceLocation& location, const SymbolInfo* host_symbol, ScopeKind kind) {
        auto new_scope = std::make_unique<Scope>(current_scope());

        Scope* new_scope_ptr           = new_scope.get();
        new_scope_ptr->kind_           = kind;
        new_scope_ptr->interval_.first = location;
        new_scope_ptr->host_symbol_    = host_symbol;

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
