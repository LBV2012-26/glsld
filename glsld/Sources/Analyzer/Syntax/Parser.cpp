#include "pch.hpp"
#include "Parser.hpp"

#include <algorithm>
#include <concepts>
#include <format>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <utility>

#include <magic_enum/magic_enum_all.hpp>
#include "Analyzer/Syntax/Preprocessor.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    Parser::Parser(Document& document,
                   SourceTable& source_table,
                   const SourceFile* source_file,
                   std::vector<Token> raw_tokens,
                   IncludeLoader& include_loader,
                   IncludeDirectoryHandle include_dirs,
                   int version_replica,
                   VersionPointer version_pointer)

        : source_file_{ source_file }
        , raw_tokens_{ std::move(raw_tokens) }
        , version_replica_{ version_replica }
        , version_pointer_{ version_pointer }
        , document_{ document }
    {
        document_.PrepareInjectedMacros(source_file_);
        Parse(source_table, include_loader, include_dirs);
    }

    Parser::Precedence Parser::GetInfixPrecedence(TokenType type) {
        static constexpr auto kTokenPrecedence = []() -> auto {
            std::array<Precedence, magic_enum::enum_count<TokenType>()> table{};
            std::ranges::fill(table, Precedence::kLowest);

            table[std::to_underlying(TokenType::kComma)]                  = Precedence::kComma;
            table[std::to_underlying(TokenType::kEqual)]                  = Precedence::kAssignment;
            table[std::to_underlying(TokenType::kPlusEqual)]              = Precedence::kAssignment;
            table[std::to_underlying(TokenType::kMinusEqual)]             = Precedence::kAssignment;
            table[std::to_underlying(TokenType::kStarEqual)]              = Precedence::kAssignment;
            table[std::to_underlying(TokenType::kSlashEqual)]             = Precedence::kAssignment;
            table[std::to_underlying(TokenType::kPercentEqual)]           = Precedence::kAssignment;
            table[std::to_underlying(TokenType::kLeftShiftEqual)]         = Precedence::kAssignment;
            table[std::to_underlying(TokenType::kRightShiftEqual)]        = Precedence::kAssignment;
            table[std::to_underlying(TokenType::kAmpersandEqual)]         = Precedence::kAssignment;
            table[std::to_underlying(TokenType::kCaretEqual)]             = Precedence::kAssignment;
            table[std::to_underlying(TokenType::kVerticalBarEqual)]       = Precedence::kAssignment;
            table[std::to_underlying(TokenType::kQuestion)]               = Precedence::kTernary;
            table[std::to_underlying(TokenType::kVerticalBarVerticalBar)] = Precedence::kLogicalOr;
            table[std::to_underlying(TokenType::kCaretCaret)]             = Precedence::kLogicalXor;
            table[std::to_underlying(TokenType::kAmpersandAmpersand)]     = Precedence::kLogicalAnd;
            table[std::to_underlying(TokenType::kVerticalBar)]            = Precedence::kBitwiseOr;
            table[std::to_underlying(TokenType::kCaret)]                  = Precedence::kBitwiseXor;
            table[std::to_underlying(TokenType::kAmpersand)]              = Precedence::kBitwiseAnd;
            table[std::to_underlying(TokenType::kEqualEqual)]             = Precedence::kEquality;
            table[std::to_underlying(TokenType::kNotEqual)]               = Precedence::kEquality;
            table[std::to_underlying(TokenType::kLessThan)]               = Precedence::kRelational;
            table[std::to_underlying(TokenType::kGreaterThan)]            = Precedence::kRelational;
            table[std::to_underlying(TokenType::kLessEqual)]              = Precedence::kRelational;
            table[std::to_underlying(TokenType::kGreaterEqual)]           = Precedence::kRelational;
            table[std::to_underlying(TokenType::kLeftShift)]              = Precedence::kShift;
            table[std::to_underlying(TokenType::kRightShift)]             = Precedence::kShift;
            table[std::to_underlying(TokenType::kPlus)]                   = Precedence::kAdditive;
            table[std::to_underlying(TokenType::kMinus)]                  = Precedence::kAdditive;
            table[std::to_underlying(TokenType::kStar)]                   = Precedence::kMultiplicative;
            table[std::to_underlying(TokenType::kSlash)]                  = Precedence::kMultiplicative;
            table[std::to_underlying(TokenType::kPercent)]                = Precedence::kMultiplicative;
            table[std::to_underlying(TokenType::kDot)]                    = Precedence::kPostfix;
            table[std::to_underlying(TokenType::kOpenBracket)]            = Precedence::kPostfix;
            table[std::to_underlying(TokenType::kOpenParen)]              = Precedence::kPostfix;
            table[std::to_underlying(TokenType::kPlusPlus)]               = Precedence::kPostfix;
            table[std::to_underlying(TokenType::kMinusMinus)]             = Precedence::kPostfix;

            return table;
        }();

        return kTokenPrecedence[std::to_underlying(type)];
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

    void Parser::Parse(
        SourceTable& source_table,
        IncludeLoader& include_loader,
        IncludeDirectoryHandle include_dirs)
    {
        Preprocessor processor(document_, source_table, source_file_, include_loader, include_dirs, raw_tokens_);
        expanded_tokens_ = processor.Process();

        document_.symbols.root_scope()->kind_ = ScopeKind::kGlobalTransparent;
        scope_stack_.push(document_.symbols.root_scope());

        auto* ast_root = ParserMainTask();
        if (ast_root == nullptr) {
            return;
        }

        document_.ast             = ast_root;
        document_.raw_tokens      = std::move(raw_tokens_);
        document_.expanded_tokens = std::move(expanded_tokens_);

        document_.ast->pprefs = std::move(pprefs_);
    }

    template <typename NodeType, typename... Types>
    NodeType* Parser::MakeNode(Types&&... args) {
        auto& arena = *document_.arena;
        return arena.Construct<NodeType>(&arena, std::forward<Types>(args)...);
    }

    TranslationUnitNode* Parser::ParserMainTask() {
        auto* root  = MakeNode<TranslationUnitNode>(current_scope());
        root->begin = SourceLocation(source_file_, 1, 1);
        root->statements.reserve(expanded_tokens_.size() / 10);

        while (current_token().type != TokenType::kEndOfFile) {
            if (version_pointer_ != nullptr && version_replica_ != version_pointer_->load(std::memory_order::relaxed)) {
                return nullptr;
            }

            auto* statement = ParseStatement();
            if (statement != nullptr) {
                root->statements.push_back(statement);
            }
        }

        root->end = GetPreviousTokenEnd();
        return root;
    }

    StatementNode* Parser::ParseStatement() {
        ArenaVector<AttributeNode*> attributes{ ArenaAllocator<AttributeNode*>(*document_.arena) };
        if (current_token().type == TokenType::kOpenBracket && PeekToken().type == TokenType::kOpenBracket) {
            attributes = ParseAttributeList();
        }

        StatementNode* node;
        switch (current_token().type) {
        case TokenType::kPrimitive:
        case TokenType::kBuiltInType:
        case TokenType::kBuiltInFunction:
        case TokenType::kIdentifier:
        case TokenType::kSpirvIntrinsic:
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
            return MakeNode<NullStatementNode>(current_scope());
        }

        if (node != nullptr && !attributes.empty()) {
            node->attributes = std::move(attributes);
        }

        return node;
    }

    PreprocessorNode* Parser::ParsePreprocessor() {
        // current token is '#'
        auto* node  = MakeNode<PreprocessorNode>(current_scope());
        node->begin = current_token().location;
        MatchAndConsume(TokenType::kSharp);

        if (current_token().type == TokenType::kEndOfFile) {
            return nullptr;
        }

        auto& directive_token = current_token();
        node->directive = directive_token.text;
        if (directive_token.type == TokenType::kKeyword) {
            // #if defined, #else
            if (directive_token.text == "if" || directive_token.text == "else") {
                expanded_tokens_[token_index_].type = TokenType::kPreprocessor;
            }
        }
        ConsumeToken();

        if (directive_token.text == "define") {
            node = ParseDefine(node, directive_token.location.filename(), directive_token.location.line());
        } else {
            node->tokens = CaptureDirectiveTokens(directive_token.location.filename(), directive_token.location.line());
            node->end    = GetPreviousTokenEnd();
        }

        pprefs_.push_back(node);
        return node;
    }

    PreprocessorNode* Parser::ParseDefine(
        PreprocessorNode* node,
        std::string_view target_file,
        std::uint32_t directive_physical_line)
    {
        // current token is macro name after "define"
        const auto& macro_token = current_token();

        if (macro_token.type != TokenType::kIdentifier ||
            macro_token.location.filename() != target_file ||
            macro_token.location.line() != directive_physical_line)
        {
            node->end = GetPreviousTokenEnd();
            return node;
        }

        node->symbol = document_.symbols.AddMacroSymbol(node, macro_token.text, macro_token.location);
        ConsumeToken();

        auto IsAdjacent = [](const Token& first, const Token& second) -> bool {
            return first.location.line() == second.location.line() &&
                (first.location.column() + first.text.length() == second.location.column());
        };

        // macro function like #define MACRO(x)
        if (current_token().type == TokenType::kOpenParen && IsAdjacent(macro_token, current_token())) {
            node->is_function = true;
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

    ArenaVector<StatementNode*> Parser::ParseMacroBody(
        std::span<const Token> body_tokens,
        const SymbolInfo* host_symbol)
    {
        ArenaVector<StatementNode*> statements{ ArenaAllocator<StatementNode*>(*document_.arena) };
        if (body_tokens.empty()) {
            return statements;
        }

        auto saved_tokens            = std::move(expanded_tokens_);
        const auto saved_index       = token_index_;
        const auto saved_scope_depth = scope_stack_.size();

        std::vector<Token> local_tokens = std::ranges::to<std::vector<Token>>(body_tokens);
        local_tokens.push_back({
            .text     = {},
            .location = local_tokens.back().location,
            .type     = TokenType::kEndOfFile
        });

        expanded_tokens_ = std::move(local_tokens);
        token_index_     = 0;

        EnterScope(body_tokens.front().location, host_symbol, ScopeKind::kMacroBody);

        while (current_token().type != TokenType::kEndOfFile) {
            const auto& token = current_token();

            if (token.type == TokenType::kBackslash) {
                ConsumeToken();
                continue;
            }

            const auto before = token_index_;
            auto* statement = ParseStatement();
            if (statement != nullptr) {
                statements.push_back(statement);
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

    CompoundStatementNode* Parser::ParseScope(const SymbolInfo* host_symbol, ScopeKind kind) {
        auto* node  = MakeNode<CompoundStatementNode>(current_scope());
        node->begin = current_token().location;
        MatchAndConsume(TokenType::kOpenBrace);

        node->internal_scope = EnterScope(node->begin, host_symbol, kind);
        node->children = ParseSequence<StatementNode>(TokenType::kCloseBrace, [this]() -> StatementNode* {
            return ParseStatement();
        }, false);

        node->end = GetCurrentTokenEnd();
        MatchAndConsume(TokenType::kCloseBrace);

        LeaveScope(node->end);
        return node;
    }

    ArenaVector<AttributeNode*> Parser::ParseAttributeList() {
        ArenaVector<AttributeNode*> attributes{ ArenaAllocator<AttributeNode*>(*document_.arena) };

        if (current_token().type != TokenType::kOpenBracket || PeekToken().type != TokenType::kOpenBracket) {
            return attributes;
        }

        do {
            // current token is [[ if in the first loop, or , in the following loops
            MatchAndConsume(TokenType::kOpenBracket);
            MatchAndConsume(TokenType::kOpenBracket);
            MatchAndConsume(TokenType::kComma);
            auto* node  = MakeNode<AttributeNode>(current_scope());
            node->begin = current_token().location;

            if (PeekToken().type == TokenType::kColonColon) {
                node->namespace_ = current_token();
                ConsumeToken();
            }

            const auto& name_token = current_token();
            if (name_token.type == TokenType::kIdentifier) {
                node->name = name_token;

                const auto* attribute_symbol =
                    document_.symbols.root_scope()->AddSymbol(node, name_token.text, name_token.location, SymbolKind::kAttribute);
                document_.bindings.try_emplace(name_token.location, attribute_symbol);
            }

            ConsumeToken();

            if (MatchAndConsume(TokenType::kOpenParen)) {
                // current token is attribute argument
                node->argument = ParseExpression(Precedence::kLowest);
                MatchAndConsume(TokenType::kCloseParen);
            }

            node->end = GetCurrentTokenEnd();
            attributes.push_back(node);
        } while (current_token().type == TokenType::kComma);

        MatchAndConsume(TokenType::kCloseBracket);
        MatchAndConsume(TokenType::kCloseBracket);
        return attributes;
    }

    StatementNode* Parser::ParseCodeStatement() {
        // current token is qualifier, type or identifier
        const auto statement_begin_index = token_index_;
        auto type_spec = ParseTypeSpec();

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
        const bool common_calling = type_spec.empty() &&
                                   (token.type == TokenType::kIdentifier ||
                                    token.type == TokenType::kBuiltInFunction ||
                                    token.type == TokenType::kSpirvIntrinsic);

        const bool constructor       = !type_spec.empty() && token.type == TokenType::kOpenParen;
        const bool is_expr_primitive = token.text == "true" || token.text == "false";

        if (common_calling || constructor || is_expr_primitive) {
            if (constructor) {
                const bool is_complex = !type_spec.template_args.empty()
                                     ||  type_spec.spirv_type != nullptr
                                     || !type_spec.array_sizes.empty();
                if (is_complex) {
                    token_index_ = statement_begin_index;
                } else {
                    const auto& type_token = type_spec.typename_token();

                    auto* callee  = MakeNode<VariableExpressionNode>(current_scope());
                    callee->begin = type_token.location;
                    callee->end   = GetCurrentTokenEnd();

                    callee->original_token = type_token;
                    callee->name           = type_token.text;
                    callee->node_type      = VariableExpressionNode::NodeType::kFunctionCallee;

                    ConsumeToken(); // consume '('

                    auto* call   = MakeNode<CallExpressionNode>(current_scope());
                    call->begin  = callee->begin;
                    call->callee = callee;

                    if (current_token().type != TokenType::kCloseParen) {
                        while (true) {
                            call->args.push_back(ParseExpression(Precedence::kAssignment));
                            if (!MatchAndConsume(TokenType::kComma)) {
                                break;
                            }
                        }
                    }

                    MatchAndConsume(TokenType::kCloseParen);
                    call->end = GetPreviousTokenEnd();

                    auto* statement  = MakeNode<ExpressionStatementNode>(current_scope());
                    statement->begin = call->begin;
                    statement->expr  = call;

                    if (current_token().type == TokenType::kSemicolon) {
                        statement->end = GetCurrentTokenEnd();
                        ConsumeToken();
                    } else {
                        statement->end = statement->expr->end;
                    }

                    return statement;
                }
            }

            return ParseExpressionStatement();
        }

        return nullptr;
    }

    FunctionDeclarationNode* Parser::ParseFunction(TypeSpec type_spec) {
        // current token is function name
        auto* node      = MakeNode<FunctionDeclarationNode>(current_scope());
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
        const auto param_typenames = MangleParameterNames(node);
        auto function_name = MangleFunctionName(name_token.text, param_typenames);

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

            auto* body_node           = MakeNode<CompoundStatementNode>(located_scope);
            body_node->internal_scope = current_scope();
            body_node->begin          = current_token().location;

            MatchAndConsume(TokenType::kOpenBrace);

            body_node->children = ParseSequence<StatementNode>(TokenType::kCloseBrace, [this]() -> StatementNode* {
                return ParseStatement();
            }, false);

            body_node->end = GetCurrentTokenEnd();
            node->body     = body_node;
            MatchAndConsume(TokenType::kCloseBrace);
        } else {
            function_name = std::format("__Decl_{}", function_name);
            kind = SymbolKind::kFunctionDecl;
            MatchAndConsume(TokenType::kSemicolon);
        }

        node->end = GetPreviousTokenEnd();
        LeaveScope(node->end);
        // current scope is function located scope
        node->declared_symbol = current_scope()->AddSymbol(node, function_name, name_token.location, kind);
        node->declared_symbol->internal_scope = internal_scope;
        node->declared_symbol->internal_scope->host_symbol_ = node->declared_symbol;
        node->internal_scope = internal_scope;
        node->internal_scope->host_symbol_ = node->declared_symbol;

        document_.symbols.AddFunctionBaseName(name_token.text, node->declared_symbol);

        return node;
    }

    ArenaVector<VariableDeclarationNode*> Parser::ParseParameterList() {
        // current token is first parameter or "void"
        ArenaVector<VariableDeclarationNode*> param_list{ ArenaAllocator<VariableDeclarationNode*>(*document_.arena) };
        param_list.reserve(6);

        // Function(void)
        if (current_token().type == TokenType::kPrimitive &&
            current_token().text == "void" &&
            PeekToken().type == TokenType::kCloseParen)
        {
            const auto& token = current_token();
            auto* node = MakeNode<VariableDeclarationNode>(current_scope());

            node->type_spec.specifiers.push_back(token);
            node->begin = token.location;
            node->end   = GetCurrentTokenEnd();

            ConsumeToken();
            param_list.push_back(node);
            return param_list;
        }

        while (current_token().type != TokenType::kEndOfFile && current_token().type != TokenType::kCloseParen) {
            // variadic parameter: ...
            if (current_token().type == TokenType::kEllipsis) {
                auto* ellipsis = MakeNode<VariableDeclarationNode>(current_scope());

                ellipsis->begin = current_token().location;
                ellipsis->is_variadic = true;
                ellipsis->type_spec.specifiers.push_back({
                    .text     = "...",
                    .location = current_token().location,
                    .type     = TokenType::kEllipsis
                });

                ConsumeToken();
                ellipsis->end = GetPreviousTokenEnd();

                MatchAndConsume(TokenType::kComma);
                param_list.push_back(ellipsis);

                continue;
            }

            auto type_spec  = ParseTypeSpec();
            auto* node      = MakeNode<VariableDeclarationNode>(current_scope());
            node->begin     = type_spec.begin_location();
            node->type_spec = std::move(type_spec);

            if (current_token().type == TokenType::kIdentifier) {
                const auto& name_token = current_token();
                node->declared_symbol  = current_scope()->AddSymbol(node, name_token.text, name_token.location, SymbolKind::kParameter);

                ConsumeToken();

                while (MatchAndConsume(TokenType::kOpenBracket)) {
                    node->type_spec.array_sizes.push_back(ParseExpression(Precedence::kLowest));
                    MatchAndConsume(TokenType::kCloseBracket);
                }
            }

            node->end = GetPreviousTokenEnd();
            param_list.push_back(node);

            if (!MatchAndConsume(TokenType::kComma)) {
                break;
            }
        }

        if (param_list.empty()) { // void main() -> void main(void)
            auto* virtual_void_node = MakeNode<VariableDeclarationNode>(current_scope());
            const auto& token       = current_token();

            virtual_void_node->type_spec.specifiers.emplace_back("void", token.location, TokenType::kPrimitive);
            virtual_void_node->begin = token.location;
            virtual_void_node->end   = token.location;

            param_list.push_back(virtual_void_node);
        }

        return param_list;
    }

    TypeSpec Parser::ParseTypeSpec() {
        // current token is specifier such as: Func("const int" input)
        TypeSpec type_spec(document_.arena.get());

        while (true) {
            const auto& token = current_token();

            if (token.type == TokenType::kPrimitive && TryParseLayoutQualifier(type_spec))
                continue;
            if (token.type == TokenType::kSpirvIntrinsic && TryParseSpirvIntrinsics(type_spec))
                continue;

            if (token.type == TokenType::kPrimitive || token.type == TokenType::kBuiltInType) {
                // (in, out, uniform, const, struct, ...)
                // (vec3, mat4, float, ...)
                if (token.text == "true" || token.text == "false") {
                    break;
                }

                type_spec.specifiers.push_back(token);
                ConsumeToken();

                if (token.text == "_Func" && MatchAndConsume(TokenType::kLessThan)) {
                    type_spec.function_type = ParseFunctionTypeSpec();
                    continue;
                }

                if (MatchAndConsume(TokenType::kLessThan)) { // coopmat<float16_t, gl_ScopeSubgroup, M, N, gl_MatrixUseA>;
                    while (current_token().type != TokenType::kEndOfFile &&
                           current_token().type != TokenType::kGreaterThan)
                    {
                        type_spec.template_args.push_back(ParseTemplateArgument());
                        if (!MatchAndConsume(TokenType::kComma)) {
                            break;
                        }
                    }

                    MatchAndConsume(TokenType::kGreaterThan);
                }

                continue;
            }

            if (token.type == TokenType::kIdentifier) {
                const auto* symbol_info = current_scope()->FindVisibleType(token.text);
                if (symbol_info == nullptr) {
                    break; // 不是类型标识符
                }

                const auto& next_token = PeekToken();
                if (next_token.type != TokenType::kIdentifier &&
                    next_token.type != TokenType::kOpenBracket)
                {
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
            auto* node = ParseExpression(Precedence::kLowest);
            type_spec.array_sizes.push_back(node);
            MatchAndConsume(TokenType::kCloseBracket);
        }

        return type_spec;
    }

    FunctionTypeSpec* Parser::ParseFunctionTypeSpec() {
        auto* function_type = document_.arena->Construct<FunctionTypeSpec>(document_.arena.get());

        function_type->return_type = ParseTypeSpec();
        MatchAndConsume(TokenType::kOpenParen);
        if (current_token().type != TokenType::kCloseParen) {
            while (true) {
                function_type->param_types.push_back(ParseTypeSpec());
                if (!MatchAndConsume(TokenType::kComma)) {
                    break;
                }
            }
        }

        MatchAndConsume(TokenType::kCloseParen);
        MatchAndConsume(TokenType::kGreaterThan);

        return function_type;
    }

    ArenaVector<Token> Parser::CaptureBalancedTokens(TokenType open, TokenType close) {
        ArenaVector<Token> captured{ ArenaAllocator<Token>(*document_.arena) };
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
                token.type == TokenType::kSpirvIntrinsic)
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
                auto type = slice[i].type;
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
                auto type = slice[i].type;
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

    QualifierArgumentNode* Parser::ParseQualifierArguments(std::span<const Token> tokens) {
        auto ComputeTokenEnd = [](const Token& token) -> SourceLocation {
            return SourceLocation(
                token.location.source_file(),
                token.location.line(),
                token.location.column() + static_cast<std::uint32_t>(token.text.length())
            );
        };

        auto MakeLeaf = [this, ComputeTokenEnd](const Token& token) -> QualifierArgumentNode* {
            auto* node = MakeNode<QualifierArgumentNode>(current_scope());

            node->begin         = token.location;
            node->arg_kind      = ResolveQualifierArgumentKind(token);
            node->token         = token;
            node->end           = ComputeTokenEnd(token);
            node->located_scope = current_scope();

            return node;
        };

        auto FinalizeRangeFromChildren = [this, ComputeTokenEnd](QualifierArgumentNode* node) -> void {
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

        auto Build = [this, MakeLeaf, FinalizeRangeFromChildren](this auto&& self, std::span<const Token> raw_slice)
            -> QualifierArgumentNode*
        {
            const auto slice = Trim(raw_slice);
            if (slice.empty()) {
                return nullptr;
            }

            if (slice.size() == 1) {
                return MakeLeaf(slice.front());
            }

            if (IsWrappedBy(slice, TokenType::kOpenBracket, TokenType::kCloseBracket)) {
                auto* node     = MakeNode<QualifierArgumentNode>(current_scope());
                node->arg_kind = QualifierArgumentKind::kArray;
                node->token    = slice.front();

                const auto inner = slice.subspan(1, slice.size() - 2);
                const auto parts = SplitTopLevel(inner, TokenType::kComma);
                for (auto part : parts) {
                    auto* child = self(part);
                    if (child != nullptr) {
                        node->children.push_back(child);
                    }
                }

                FinalizeRangeFromChildren(node);
                return node;
            }

            if (IsWrappedBy(slice, TokenType::kOpenParen, TokenType::kCloseParen)) {
                auto* node     = MakeNode<QualifierArgumentNode>(current_scope());
                node->arg_kind = QualifierArgumentKind::kGroup;
                node->token    = slice.front();

                const auto inner = slice.subspan(1, slice.size() - 2);
                auto* child = self(inner);
                if (child != nullptr) {
                    node->children.push_back(child);
                }

                FinalizeRangeFromChildren(node);
                return node;
            }

            if (auto equal_pos = FindTopLevel(slice, TokenType::kEqual)) {
                auto* node     = MakeNode<QualifierArgumentNode>(current_scope());
                node->arg_kind = QualifierArgumentKind::kAssignment;
                node->token    = slice[*equal_pos];

                auto* lhs = self(slice.first(*equal_pos));
                if (lhs != nullptr) {
                    node->children.push_back(lhs);
                }

                // layout(heap_offset = pc.value) xxx
                const auto slice_rhs   = slice.subspan(*equal_pos + 1);
                auto saved_tokens      = std::move(expanded_tokens_);
                const auto saved_index = token_index_;

                std::vector<Token> local(slice_rhs.begin(), slice_rhs.end());
                local.push_back({ .type = TokenType::kSemicolon });

                expanded_tokens_ = std::move(local);
                token_index_     = 0;

                node->rhs_expr = ParseExpression(Precedence::kLowest);

                expanded_tokens_ = std::move(saved_tokens);
                token_index_     = saved_index;

                auto rhs = self(slice_rhs);
                if (rhs != nullptr) {
                    node->children.push_back(rhs);
                }

                FinalizeRangeFromChildren(node);
                return node;
            }

            auto* node     = MakeNode<QualifierArgumentNode>(current_scope());
            node->arg_kind = QualifierArgumentKind::kSequence;
            node->token    = slice.front();

            for (const auto& token : slice) {
                node->children.push_back(MakeLeaf(token));
            }

            FinalizeRangeFromChildren(node);
            return node;
        };

        return Build(tokens);
    }

    LayoutQualifierNode* Parser::ParseLayoutQualifier() {
        // current token is "layout"
        const auto& token = current_token();
        if (token.type != TokenType::kPrimitive || token.text != "layout") {
            return nullptr;
        }

        auto* node  = MakeNode<LayoutQualifierNode>(current_scope());
        node->begin = token.location;
        node->end   = GetCurrentTokenEnd();
        ConsumeToken();

        if (current_token().type != TokenType::kOpenParen) {
            return node;
        }

        node->raw_tokens = CaptureBalancedTokens(TokenType::kOpenParen, TokenType::kCloseParen);

        const auto parts = SplitTopLevel(node->raw_tokens, TokenType::kComma);
        for (auto part : parts) {
            auto* param = ParseQualifierArguments(part);
            if (param != nullptr) {
                node->params.push_back(param);
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

    SpirvIntrinsicNode* Parser::ParseSpirvIntrinsics() {
        // current token is SPIR-V intrinsic keyword
        if (current_token().type != TokenType::kSpirvIntrinsic) {
            return nullptr;
        }

        const auto& keyword = current_token();

        auto* node           = MakeNode<SpirvIntrinsicNode>(current_scope());
        node->keyword        = keyword;
        node->intrinsic_kind = ResolveSpirvIntrinsicKind(keyword.text);
        node->begin          = keyword.location;
        node->end            = GetCurrentTokenEnd();

        ConsumeToken();

        if (current_token().type != TokenType::kOpenParen) {
            return node;
        }

        node->raw_tokens = CaptureBalancedTokens(TokenType::kOpenParen, TokenType::kCloseParen);

        const auto parts = SplitTopLevel(node->raw_tokens, TokenType::kComma);
        for (auto part : parts) {
            auto* param = ParseQualifierArguments(part);
            if (param != nullptr) {
                node->params.push_back(param);
            }
        }

        node->end = GetPreviousTokenEnd(); // CaptureBalancedTokens has consumed ')'
        return node;
    }

    ExpressionNode* Parser::ParseTemplateArgument() {
        auto end = token_index_;
        int paren_level   = 0;
        int bracket_level = 0;
        int brace_level   = 0;


        while (end < expanded_tokens_.size()) {
            const auto type = expanded_tokens_[end].type;

            if (paren_level == 0 && bracket_level == 0 && brace_level == 0 &&
                (type == TokenType::kComma || type == TokenType::kGreaterThan))
            {
                break;
            }

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

            ++end;
        }

        const auto previous_stop = expr_stop_index_;
        expr_stop_index_ = end;

        auto* result = ParseExpression(Precedence::kLowest);

        expr_stop_index_ = previous_stop;
        return result;
    }

    bool Parser::TryParseLayoutQualifier(TypeSpec& type_spec) {
        // layout(...)
        const auto& token = current_token();
        if (token.text != "layout") {
            return false;
        }

        type_spec.specifiers.push_back(token);

        auto* node = ParseLayoutQualifier();
        if (node == nullptr) {
            return false;
        }

        type_spec.layouts.push_back(node);
        return true;
    }

    bool Parser::TryParseSpirvIntrinsics(TypeSpec& type_spec) {
        auto* node = ParseSpirvIntrinsics();
        if (node == nullptr) {
            return false;
        }

        type_spec.specifiers.push_back(node->keyword);
        if (node->intrinsic_kind == SpirvIntrinsicKind::kTypeOverride) {
            type_spec.spirv_type = node;
        }

        type_spec.spirv_intrinsics.push_back(node);
        return true;
    }

    namespace {
        // const int kConstant = 0;
        // layout(constant_id = 0) kConstant;
        bool IsLayoutAttachment(const TypeSpec& type_spec) {
            if (type_spec.layouts.empty()       ||
               !type_spec.template_args.empty() ||
               !type_spec.array_sizes.empty()   ||
               !type_spec.spirv_intrinsics.empty())
            {
                return false;
            }

            if (!std::ranges::all_of(type_spec.specifiers, [](const Token& token) -> bool {
                return token.text == "layout";
            })) {
                return false;
            }

            return std::ranges::any_of(type_spec.layouts, [](const auto& layout) -> bool {
                return std::ranges::any_of(layout->params, [](const auto& param) -> bool {
                    return  param->arg_kind == QualifierArgumentKind::kAssignment
                        && !param->children.empty()
                        &&  param->children.front()->arg_kind   == QualifierArgumentKind::kIdentifier
                        &&  param->children.front()->token.text == "constant_id";
                });
            });
        }

        void MergeAttachedLayouts(TypeSpec& target, TypeSpec&& attachment) {
            const auto spec_pos = std::ranges::find_if(target.specifiers, [](const Token& token) -> bool {
                return token.text != "layout";
            });

            target.specifiers.insert_range(spec_pos, attachment.specifiers | std::views::as_rvalue);
            target.layouts.append_range(attachment.layouts | std::views::as_rvalue);
        }
    }

    DeclarationGroupNode* Parser::ParseVariableDeclarationList(TypeSpec type_spec) {
        // current token is variable name or semicolon
        if (current_token().type == TokenType::kSemicolon) {
            auto* node      = MakeNode<DeclarationGroupNode>(current_scope());
            node->begin     = type_spec.begin_location();
            node->end       = GetCurrentTokenEnd();
            node->type_spec = std::move(type_spec);

            ConsumeToken();
            return node;
        }

        auto ParseSingleDeclarer = [&]() -> VariableDeclarationNode* {
            if (current_token().type != TokenType::kIdentifier) {
                return nullptr;
            }

            const auto& name_token = current_token();
            auto* node      = MakeNode<VariableDeclarationNode>(current_scope());
            node->begin     = type_spec.begin_location();
            node->type_spec = type_spec;

            if (IsLayoutAttachment(node->type_spec) && PeekToken().type == TokenType::kSemicolon) {
                const auto& existing_symbol = current_scope()->FindSymbol(name_token.text);
                if (existing_symbol != nullptr && existing_symbol->kind == SymbolKind::kVariable) {
                    document_.bindings.try_emplace(name_token.location, existing_symbol);

                    auto it = variable_declaration_cache_.find(existing_symbol);
                    if (it != variable_declaration_cache_.end()) {
                        auto* original_decl = it->second;
                        MergeAttachedLayouts(original_decl->type_spec, std::move(node->type_spec));
                    }
                }

                ConsumeToken(); // kConstant
                return nullptr;
            }

            node->declared_symbol = current_scope()->AddSymbol(
                node, name_token.text, name_token.location, SymbolKind::kVariable);

            if (node->declared_symbol != nullptr && node->declared_symbol->node == node) {
                variable_declaration_cache_.try_emplace(node->declared_symbol, node);
            }

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

        auto* node  = MakeNode<DeclarationGroupNode>(current_scope());
        node->begin = type_spec.begin_location();

        if (auto* first_node = ParseSingleDeclarer()) {
            node->declarations.push_back(first_node);
        }

        while (current_token().type == TokenType::kComma) {
            ConsumeToken();
            if (auto next_node = ParseSingleDeclarer()) {
                node->declarations.push_back(next_node);
            } else {
                break;
            }
        }

        // terminate with semicolon
        if (current_token().type == TokenType::kSemicolon) {
            const auto end_location = GetCurrentTokenEnd();
            if (!node->declarations.empty()) {
                node->declarations.back()->end = end_location;
            }

            node->end = end_location;
            ConsumeToken();
        } else if (!node->declarations.empty()) {
            node->end = node->declarations.back()->end;
        }

        if (node->declarations.empty()) {
            node->type_spec = std::move(type_spec);
        }

        return node;
    }

    ExpressionStatementNode* Parser::ParseExpressionStatement() {
        auto* node  = MakeNode<ExpressionStatementNode>(current_scope());
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

    bool Parser::IsCastExpression() {
        if (current_token().type != TokenType::kOpenParen) {
            return false;
        }

        const auto& type_token  = PeekToken(1);
        const auto& close_token = PeekToken(2);

        if (close_token.type != TokenType::kCloseParen) {
            return false;
        }

        switch (type_token.type) {
        case TokenType::kPrimitive:
        case TokenType::kBuiltInType:
            return true;

        case TokenType::kIdentifier:
            return current_scope()->FindVisibleType(type_token.text) != nullptr;

        default:
            return false;
        }
    }

    ExpressionNode* Parser::ParsePrefixExpression() {
        const auto& token = current_token();

        // GL_NV_explicit_typecast: (type)expr
        if (token.type == TokenType::kOpenParen && IsCastExpression()) {
            return ParseCastExpression();
        }

        // ( expr )
        if (token.type == TokenType::kOpenParen) {
            ConsumeToken();
            auto* expr_node = ParseExpression(Precedence::kLowest);

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

        // 标识符/内置类型/内置函数（.length）/Primitive/SpirvIntrinsics (函数本来应该算标识符，但内置函数无需声明)
        case TokenType::kIdentifier:
        case TokenType::kBuiltInType:
        case TokenType::kBuiltInFunction:
        case TokenType::kPrimitive:
        case TokenType::kSpirvIntrinsic:
            return ParseVariableReference();

        // 前缀一元运算符 (!b, -x, ++i, --j, ~mask)
        case TokenType::kExclamation:
        case TokenType::kMinus:
        case TokenType::kPlus:
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

    CastExpressionNode* Parser::ParseCastExpression() {
        // current token is (
        auto* node  = MakeNode<CastExpressionNode>(current_scope());
        node->begin = current_token().location;

        ConsumeToken();

        node->target_type.specifiers.push_back(current_token());
        ConsumeToken();

        MatchAndConsume(TokenType::kCloseParen);

        node->operand = ParseExpression(Precedence::kTypecast);

        if (node->operand != nullptr) {
            node->end = node->operand->end;
        } else {
            node->end = GetPreviousTokenEnd();
        }

        return node;
    }

    RawExpressionNode* Parser::ParseLiteral() {
        // current token is raw literal
        auto* node = MakeNode<RawExpressionNode>(current_scope());
        const auto& token = current_token();

        node->begin = token.location;
        node->tokens.push_back(token);
        ConsumeToken();
        node->end = GetPreviousTokenEnd();

        return node;
    }

    VariableExpressionNode* Parser::ParseVariableReference() {
        auto* node = MakeNode<VariableExpressionNode>(current_scope());
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

    UnaryExpressionNode* Parser::ParsePrefixUnary() {
        auto* node = MakeNode<UnaryExpressionNode>(current_scope());
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

    InitializerListExpressionNode* Parser::ParseInitializerList() {
        // current token is {
        auto* node  = MakeNode<InitializerListExpressionNode>(current_scope());
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

    ExpressionNode*
    Parser::ParseInfixExpression(ExpressionNode* left, TokenType op_type, Precedence precedence) {
        switch (op_type) {
        // 成员访问 (object.number)
        case TokenType::kDot:
            return ParseMemberAccess(left);

        // 数组下标 (array[index])
        case TokenType::kOpenBracket:
            return ParseArrayIndex(left);

        // 函数调用 (Func(args...))
        case TokenType::kOpenParen:
            return ParseFunctionCall(left);

        // 后缀运算符
        case TokenType::kPlusPlus:
        case TokenType::kMinusMinus:
            return ParsePostfixUnary(left, op_type);

        // 三元运算符
        case TokenType::kQuestion:
            return ParseTernary(left);

        // 标准二元运算符 (+, -, *, /, &&, =, etc.)
        default:
            return ParseStandardBinary(left, op_type, precedence);
        }
    }

    MemberAccessExpressionNode* Parser::ParseMemberAccess(ExpressionNode* object) {
        auto* node   = MakeNode<MemberAccessExpressionNode>(current_scope());
        node->begin  = object->begin;
        node->object = object;

        const auto& member_token = current_token();
        if (member_token.type == TokenType::kIdentifier) {
            if (PeekToken().type != TokenType::kOpenParen) [[likely]] {
                auto member_node            = MakeNode<VariableExpressionNode>(current_scope());
                member_node->begin          = member_token.location;
                member_node->original_token = member_token;
                member_node->node_type      = VariableExpressionNode::NodeType::kBlockMember;
                member_node->name           = member_token.text;
                member_node->end            = GetCurrentTokenEnd();

                node->member = member_node;
                ConsumeToken();
            } else { // array.length();, the only member func that GLSL supports for built-in arrays
                expanded_tokens_[token_index_].type = TokenType::kBuiltInFunction;
                auto* callee_expr = ParseVariableReference();
                MatchAndConsume(TokenType::kOpenParen); // simulate pratt parsing comsume '(' after callee
                auto* callee_node = ParseFunctionCall(callee_expr);

                node->member = callee_node;

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

    IndexExpressionNode* Parser::ParseArrayIndex(ExpressionNode* base) {
        auto* node  = MakeNode<IndexExpressionNode>(current_scope());
        node->begin = base->begin;
        node->base  = base;
        node->index = ParseExpression(Precedence::kLowest);

        if (current_token().type == TokenType::kCloseBracket) {
            node->end = GetCurrentTokenEnd();
            ConsumeToken();
        } else {
            node->end = GetPreviousTokenEnd();
        }

        return node;
    }

    CallExpressionNode* Parser::ParseFunctionCall(ExpressionNode* callee) {
        auto* node   = MakeNode<CallExpressionNode>(current_scope());
        node->begin  = callee->begin;
        node->callee = callee;

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

    TernaryExpressionNode* Parser::ParseTernary(ExpressionNode* condition) {
        auto* node      = MakeNode<TernaryExpressionNode>(current_scope());
        node->begin     = condition->begin;
        node->condition = condition;
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

    UnaryExpressionNode* Parser::ParsePostfixUnary(ExpressionNode* operand, TokenType op_type) {
        auto* node       = MakeNode<UnaryExpressionNode>(current_scope());
        node->begin      = operand->begin;
        node->operand    = operand;
        node->op         = op_type;
        node->is_postfix = true;
        node->end        = GetPreviousTokenEnd();

        return node;
    }

    BinaryExpressionNode*
    Parser::ParseStandardBinary(ExpressionNode* left, TokenType op_type, Precedence precedence) {
        auto* node  = MakeNode<BinaryExpressionNode>(current_scope());
        node->begin = left->begin;
        node->left  = left;
        node->op    = op_type;

        const auto next_min_prec =
            IsRightAssociative(op_type) ? precedence : static_cast<Precedence>(static_cast<int>(precedence) + 1);

        node->right = ParseExpression(next_min_prec);
        if (node->right != nullptr) {
            node->end = node->right->end;
        } else {
            node->end = GetPreviousTokenEnd();
        }

        return node;
    }

    ExpressionNode* Parser::ParseExpression(Precedence min_prec) {
        auto* left = ParsePrefixExpression();
        if (left == nullptr) {
            return nullptr;
        }

        while (true) {
            if (expr_stop_index_.has_value() && *expr_stop_index_ == token_index_) {
                break;
            }

            const auto op_type = current_token().type;
            const auto op_prec = GetInfixPrecedence(op_type);

            if (op_prec == Precedence::kLowest || op_prec < min_prec) {
                break;
            }

            ConsumeToken();
            left = ParseInfixExpression(left, op_type, op_prec);
        }

        return left;
    }

    DeclarationNode* Parser::ParseBlockBody(TypeSpec type_spec) {
        std::string_view name;
        SourceLocation   name_location;

        if (current_token().type == TokenType::kIdentifier) {
            // current token is block name
            const auto& block_name = current_token();
            name = block_name.text;
            name_location = block_name.location;
            ConsumeToken();
        } else { // anonymous
            const auto generated_name = std::format("__AnonymousStruct_{}", GetNextAnonymousId());
            name = document_.StoreTokenText(generated_name);
            name_location = current_token().location;
        }

        const bool is_struct = type_spec.has_keyword("struct");

        auto ParseBody = [&](auto& node) -> void {
            node->begin           = type_spec.begin_location();
            const auto block_kind = is_struct ? SymbolKind::kStruct : SymbolKind::kInterface;
            node->declared_symbol = current_scope()->AddSymbol(node, name, name_location, block_kind);
            node->body            = ParseScope(node->declared_symbol, ScopeKind::kBlock);

            if (current_token().type != TokenType::kSemicolon) {
                // struct MyStruct { ... } instance;
                auto instance_spec = type_spec;
                Token specifier{
                    .text     = name,
                    .location = name_location,
                    .type     = TokenType::kIdentifier
                };

                instance_spec.specifiers.push_back(std::move(specifier));
                node->instances = ParseVariableDeclarationList(std::move(instance_spec));
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

            if (node->body != nullptr) {
                if (node->declared_symbol != nullptr) {
                    node->declared_symbol->internal_scope = node->body->internal_scope;
                }
                node->internal_scope = node->body->internal_scope;
            }

            node->end = GetPreviousTokenEnd();
        };

        if (is_struct) {
            auto* node = MakeNode<StructDeclarationNode>(current_scope());
            ParseBody(node);
            return node;
        } else {
            auto* node = MakeNode<InterfaceDeclarationNode>(current_scope());
            ParseBody(node);
            node->type_spec = std::move(type_spec);
            return node;
        }
    }

    StatementNode* Parser::ParseControlFlowStatement() {
        const auto& keyword = current_token().text;

        switch (keyword.front()) {
        case 'i':
            if (keyword == "if")
                return ParseIfStatement();
            break;
        case 'f':
            if (keyword == "for")
                return ParseForStatement();
            break;
        case 'd':
            if (keyword == "do")
                return ParseDoStatement();
            if (keyword == "discard" || keyword == "demote" || keyword == "terminateInvocation")
                return ParseJumpStatement();
            break;
        case 'w':
            if (keyword == "while")
                return ParseWhileStatement();
            break;
        case 's':
            if (keyword == "switch")
                return ParseSwitchStatement();
            break;
        case 'r':
            if (keyword == "return")
                return ParseJumpStatement();
            break;
        case 'b':
            if (keyword == "break")
                return ParseJumpStatement();
            break;
        case 'c':
            if (keyword == "continue")
                return ParseJumpStatement();
            break;
        }

        // error
        ConsumeToken();
        return nullptr;
    }

    IfStatementNode* Parser::ParseIfStatement() {
        // current token is "if"
        auto* node  = MakeNode<IfStatementNode>(current_scope());
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

    ForStatementNode* Parser::ParseForStatement() {
        // current token is "for"
        auto* node  = MakeNode<ForStatementNode>(current_scope());
        node->begin = current_token().location;
        ConsumeToken();

        EnterScope(node->begin);
        MatchAndConsume(TokenType::kOpenParen);

        if (current_token().type == TokenType::kSemicolon) {
            // for (; ...
            node->init = MakeNode<NullStatementNode>(current_scope());
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

    DoStatementNode* Parser::ParseDoStatement() {
        // current token is "do"
        auto* node  = MakeNode<DoStatementNode>(current_scope());
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

    WhileStatementNode* Parser::ParseWhileStatement() {
        // current token is "while"
        auto* node  = MakeNode<WhileStatementNode>(current_scope());
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

    SwitchStatementNode* Parser::ParseSwitchStatement() {
        // current token is "switch"
        auto* node  = MakeNode<SwitchStatementNode>(current_scope());
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

    CaseStatementNode* Parser::ParseCaseLabel() {
        // current token is "case" or "default"
        auto* node  = MakeNode<CaseStatementNode>(current_scope());
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
            auto* statement = ParseStatement();
            if (statement != nullptr) {
                node->body.push_back(statement);
            }
        }

        if (!node->body.empty()) {
            node->end = node->body.back()->end;
        } else {
            node->end = GetPreviousTokenEnd();
        }

        return node;
    }

    StatementNode* Parser::ParseJumpStatement() {
        // current token is "return", "break", "continue" or "discard"
        const auto& keyword_token = current_token();

        if (keyword_token.text == "return") {
            auto* node  = MakeNode<ReturnStatementNode>(current_scope());
            node->begin = keyword_token.location;
            ConsumeToken();

            if (current_token().type != TokenType::kSemicolon) {
                node->return_value = ParseExpression(Precedence::kLowest);
            }

            node->end = GetCurrentTokenEnd();
            MatchAndConsume(TokenType::kSemicolon);
            return node;
        }

        StatementNode* node = nullptr;
        if (keyword_token.text == "break") {
            node = MakeNode<BreakStatementNode>(current_scope());
        } else if (keyword_token.text == "continue") {
            node = MakeNode<ContinueStatementNode>(current_scope());
        } else if (keyword_token.text == "discard") {
            node = MakeNode<DiscardStatementNode>(current_scope());
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

    ArenaVector<Token> Parser::CaptureDirectiveTokens(std::string_view target_file, std::uint32_t directive_physical_line) {
        ArenaVector<Token> collected{ ArenaAllocator<Token>(*document_.arena) };
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
                    if (token.location.line() - collected.back().location.line() > 1) {
                        break;
                    }
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
    } && std::same_as<Ty, ArenaVector<typename Ty::value_type>>;

    template <typename Ty>
    ArenaVector<Ty*> Parser::ParseSequence(TokenType terminator, auto parse_func, bool consume_terminator) {
        ArenaVector<Ty*> nodes{ ArenaAllocator<Ty*>(*document_.arena) };

        while (current_token().type != TokenType::kEndOfFile && current_token().type != terminator) {
            auto result = parse_func();

            if constexpr (IsVector<decltype(result)>) {
                for (auto& node : result) {
                    if (node != nullptr) {
                        nodes.push_back(node);
                    }
                }
            } else {
                if (result != nullptr) {
                    nodes.push_back(result);
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

    namespace {
        std::string BuildFunctionTypeSpecName(const FunctionTypeSpec* function_type);

        std::string BuildCommonTypeSpecName(const TypeSpec& type_spec) {
            std::string result;

            for (const auto& specifier : type_spec.specifiers) {
                if (!result.empty()) {
                    result += " ";
                }

                result += specifier.text;
            }

            if (type_spec.function_type != nullptr) {
                result += std::format("<{}>", BuildFunctionTypeSpecName(type_spec.function_type));
            }

            for (const auto* array_size : type_spec.array_sizes) {
                result += "[";
                if (array_size->kind() == AstNodeKind::kVariableExpression) {
                    result += static_cast<const VariableExpressionNode*>(array_size)->name;
                } else if (array_size->kind() == AstNodeKind::kRawExpression) {
                    auto* raw = static_cast<const RawExpressionNode*>(array_size);
                    for (const auto& token : raw->tokens) {
                        result += token.text;
                    }
                }

                result += "]";
            }

            return result;
        }

        std::string BuildFunctionTypeSpecName(const FunctionTypeSpec* function_type) {
            std::string result = BuildCommonTypeSpecName(function_type->return_type);
            result += "(";

            for (const auto& [i, param_type] : function_type->param_types | std::views::enumerate) {
                if (!std::cmp_equal(i, 0)) {
                    result += ", ";
                }

                result += BuildCommonTypeSpecName(param_type);
            }

            result += ")";
            return result;
        }
    }

    std::vector<std::string> Parser::MangleParameterNames(const FunctionDeclarationNode* node) {
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
                    const auto parameters = Utils::BuildQualifierParameterList(param->type_spec.spirv_type);
                    param_typename += std::format("spirv_type({})", parameters);
                }
            }

            if (param->type_spec.function_type != nullptr) {
                param_typename += std::format("<{}>", BuildFunctionTypeSpecName(param->type_spec.function_type));
            } else if (!param->type_spec.template_args.empty()) {
                param_typename += "<";
                for (auto i = 0uz; i != param->type_spec.template_args.size(); ++i) {
                    if (param->type_spec.template_args[i]->kind() == AstNodeKind::kVariableExpression) {
                        auto* var_expr = static_cast<const VariableExpressionNode*>(param->type_spec.template_args[i]);
                        param_typename += var_expr->name;
                    } else if (param->type_spec.template_args[i]->kind() == AstNodeKind::kRawExpression) {
                        auto* raw_node = static_cast<const RawExpressionNode*>(param->type_spec.template_args[i]);
                        if (!raw_node->tokens.empty()) {
                            param_typename += raw_node->tokens.front().text;
                        }
                    }

                    if (i + 1 != param->type_spec.template_args.size()) {
                        param_typename += ", ";
                    }
                }

                param_typename += ">";
            }

            for (const auto& array_size : param->type_spec.array_sizes) {
                if (array_size == nullptr) {
                    param_typename += "[]";
                    continue;
                }

                std::string array_dimension;

                if (array_size->kind() == AstNodeKind::kRawExpression) {
                    auto* raw_node = static_cast<const RawExpressionNode*>(array_size);
                    for (const auto& token : raw_node->tokens) {
                        array_dimension += token.text;
                    }
                } else if (array_size->kind() == AstNodeKind::kVariableExpression) {
                    const auto* var_expr = static_cast<const VariableExpressionNode*>(array_size);
                    array_dimension = var_expr->name;
                }

                param_typename = std::format("{}[{}]", param_typename, array_dimension);
            }

            param_typenames.push_back(std::move(param_typename));
        }

        return param_typenames;
    }

    std::string Parser::MangleFunctionName(std::string_view base_name, std::span<const std::string> param_typenames) {
        std::string mangled_name(base_name);
        mangled_name += "(";

        const auto typename_size = param_typenames.size();
        for (const auto& [i, param_typename] : param_typenames | std::views::enumerate) {
            mangled_name += param_typename;
            mangled_name += std::cmp_equal(i, typename_size - 1) ? ")" : ", ";
        }

        return mangled_name;
    }
}
