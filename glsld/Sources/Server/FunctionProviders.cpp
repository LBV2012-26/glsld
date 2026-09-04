#include "pch.hpp"
#include "FunctionProviders.hpp"

#include <cstddef>
#include <algorithm>
#include <filesystem>
#include <format>
#include <iterator>
#include <ranges>
#include <span>
#include <system_error>
#include <type_traits>
#include <utility>
#include <variant>

#include <ankerl/unordered_dense.h>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Passes/ConstantEvaluator.hpp"
#include "Analyzer/Passes/InlayHintCollector.hpp"
#include "Analyzer/Passes/NodeLocator.hpp"
#include "Analyzer/Passes/TypeResolver.hpp"
#include "Analyzer/Syntax/MetadataManager.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Base/Hash.hpp"
#include "Utils/Utils.hpp"

namespace glsld::Providers {
    namespace {
        bool IsPositionInToken(const Token& token, const SourceLocation& position) {
            if (*token.location.source_file() != *position.source_file()) {
                return false;
            }

            if (token.location.line() != position.line()) {
                return false;
            }

            const auto start_column = token.location.column();
            const auto end_column   = start_column + static_cast<std::uint32_t>(token.text.length());

            // [start_column, end_column]
            return position.column() >= start_column && position.column() <= end_column;
        }

        std::optional<std::size_t> FindCursorTokenIndex(
            std::span<const Token> tokens,
            const SourceLocation& location)
        {
            auto it = std::ranges::upper_bound(tokens, location, std::ranges::less{}, &Token::location);
            if (it == tokens.begin()) {
                return std::nullopt;
            }

            const auto index = static_cast<std::size_t>(std::distance(tokens.begin(), std::prev(it)));
            if (!IsPositionInToken(tokens[index], location)) {
                return std::nullopt;
            }

            return index;
        }

        const SymbolInfo* FindFunctionCounterpart(const SymbolList& symbol_list, const SymbolInfo* symbol) {
            if (symbol == nullptr) {
                return nullptr;
            }

            const auto target_kind = symbol->kind == SymbolKind::kFunctionImpl
                                   ? SymbolKind::kFunctionDecl
                                   : SymbolKind::kFunctionImpl;

            for (const auto& counterpart : symbol_list) {
                if (counterpart->kind != target_kind ||
                    counterpart->param_typeinfos.size() != symbol->param_typeinfos.size()) {
                    continue;
                }

                bool signature_matched = true;
                for (auto i = 0uz; i != symbol->param_typeinfos.size(); ++i) {
                    if (!symbol->param_typeinfos[i].CompareWithoutQualifiers(counterpart->param_typeinfos[i])) {
                        signature_matched = false;
                        break;
                    }
                }

                if (signature_matched) {
                    return counterpart;
                }
            }

            return nullptr;
        };

        void PushUniquePath(std::vector<std::filesystem::path>& paths, const std::filesystem::path& path) {
            const auto normalized = Utils::NormalizePath(path);
            const auto filename   = normalized.generic_string();

            auto Predicate = [&](const auto& path) -> bool {
                return Utils::NormalizePath(path).generic_string() == filename;
            };

            if (std::ranges::any_of(paths, Predicate)) {
                return;
            }

            paths.push_back(path);
        }

        nlohmann::json ConvertScopeToDocumentSymbols(
            Context& context,
            std::string_view uri,
            const Scope* const scope,
            const Unicode::PositionMapper& mapper)
        {
            nlohmann::json symbols = nlohmann::json::array();

            if (scope == nullptr) {
                return symbols;
            }

            auto ConvertToLspRange = [&mapper](const auto& begin, const auto& end) -> nlohmann::json {
                return {
                    { "start", { { "line", begin.line() - 1 }, { "character", mapper.ToUtf16Character(begin.line(), begin.column()) } } },
                    { "end",   { { "line", end.line()   - 1 }, { "character", mapper.ToUtf16Character(end.line(),   end.column()) } } }
                };
            };

            auto ConvertToSelectionRange = [&mapper](const auto& location, std::string_view name) -> nlohmann::json {
                return {
                    { "start", { { "line", location.line() - 1 }, { "character", mapper.ToUtf16Character(location.line(), location.column()) } } },
                    { "end",   { { "line", location.line() - 1 }, { "character", mapper.ToUtf16Character(location.line(), location.column() + static_cast<std::uint32_t>(name.length())) } } }
                };
            };

            // split __Decl_ or __Impl_ on function
            auto SplitSymbolName = [](std::string_view mangled_name) -> std::string_view {
                if (mangled_name.starts_with("__Decl_")) {
                    return mangled_name.substr(7);
                } else if (mangled_name.starts_with("__Impl_")) {
                    return mangled_name.substr(7);
                } else {
                    return mangled_name; // common token
                }
            };

            ankerl::unordered_dense::set<const Scope*> handled_scopes;

            for (const auto& [name, info] : scope->symbols()) {
                ABORT_IF_CANCELLED();

                const auto& location = info->location;
                auto symbol_name = SplitSymbolName(info->name);
                if (symbol_name.empty() ||
                    location.source_file() == nullptr ||
                    location.uri() != uri ||
                    location.line() == 0 ||
                    location.column() == 0)
                {
                    continue;
                }

                nlohmann::json symbol_node;

                int symbol_kind = ConvertSymbolKind(info->kind);
                if (symbol_kind == 13) {
                    if (info->located_scope != nullptr &&
                        (info->located_scope->kind() == ScopeKind::kBlock || info->located_scope->kind() == ScopeKind::kBlockTransparent))
                    {
                        symbol_kind = 8;
                    }
                }

                symbol_node["name"] = std::move(symbol_name);
                symbol_node["kind"] = symbol_kind;
                symbol_node["selectionRange"] = ConvertToSelectionRange(info->location, info->name);

                const Scope* child_scope = nullptr;
                if (info->kind == SymbolKind::kInterface || info->kind == SymbolKind::kStruct) {
                    for (const auto& child : scope->children()) {
                        ABORT_IF_CANCELLED();

                        if (child->host_symbol() == info.get()) {
                            child_scope = child.get();
                            handled_scopes.insert(child_scope);
                            break;
                        }
                    }

                    if (info->kind == SymbolKind::kFunctionDecl) {
                        symbol_node["detail"] = "(decl)";
                    }
                }

                if (child_scope != nullptr) {
                    symbol_node["range"] = ConvertToLspRange(info->location, child_scope->interval().second);
                    if (info->kind == SymbolKind::kInterface || info->kind == SymbolKind::kStruct) {
                        const auto children = ConvertScopeToDocumentSymbols(context, uri, child_scope, mapper);
                        if (!children.empty()) {
                            symbol_node["children"] = children;
                        }
                    }
                } else {
                    symbol_node["range"] = symbol_node["selectionRange"];
                }

                auto& selection_range = symbol_node["selectionRange"];
                auto& range           = symbol_node["range"];
                if (range["end"]["line"]      <  selection_range["end"]["line"] ||
                   (range["end"]["line"]      == selection_range["end"]["line"] &&
                    range["end"]["character"] <  selection_range["end"]["character"]))
                {
                    range["end"] = selection_range["end"];
                }

                symbols.push_back(symbol_node);
            }

            // Transparent scope
            for (const auto& child_scope : scope->children()) {
                ABORT_IF_CANCELLED();

                if ((child_scope->kind() == ScopeKind::kGlobalTransparent ||
                     child_scope->kind() == ScopeKind::kBlockTransparent) &&
                    !handled_scopes.contains(child_scope.get()))
                {
                    const auto transparent_children = ConvertScopeToDocumentSymbols(context, uri, child_scope.get(), mapper);
                    for (const auto& child : transparent_children) {
                        symbols.push_back(child);
                    }
                }
            }

            return symbols;
        }
    }

    nlohmann::json GetDocumentSymbols(
        Context& context,
        Snapshot snapshot,
        std::string_view uri,
        const Unicode::PositionMapper& mapper)
    {
        auto result = ConvertScopeToDocumentSymbols(context, uri, snapshot->symbols.root_scope(), mapper);

        for (const auto& macro : snapshot->symbols.macro_symbols()) {
            const auto* symbol = macro.get();
            if (symbol->location.source_file() == nullptr ||
                symbol->location.uri() != uri ||
                symbol->location.line() == 0)
            {
                continue;
            }

            auto* node = static_cast<const PreprocessorNode*>(symbol->node);
            nlohmann::json macro_symbol{
                { "name", symbol->name },
                { "kind", ConvertSymbolKind(SymbolKind::kMacro) },
                { "selectionRange", {
                    {
                        "start", {
                            { "line", symbol->location.line() - 1 },
                            { "character", mapper.ToUtf16Character(symbol->location.line(), symbol->location.column()) }
                        }
                    },
                    {
                        "end", {
                            { "line", symbol->location.line() - 1 },
                            { "character", mapper.ToUtf16Character(symbol->location.line(), symbol->location.column() + static_cast<std::uint32_t>(symbol->name.length())) }
                        }
                    }
                } },
                { "range", {
                    {
                        "start", {
                            { "line", node->begin.line() - 1 },
                            { "character", mapper.ToUtf16Character(node->begin.line(), node->begin.column()) }
                        }
                    },
                    {
                        "end", {
                            { "line", node->end.line() - 1 },
                            { "character", mapper.ToUtf16Character(node->end.line(),   node->end.column()) }
                        }
                    }
                } }
            };

            result.push_back(macro_symbol);
        }

        return result;
    }

    namespace {
        int GetSymbolSemanticHighlight(SymbolKind kind) {
            int type_index = -1;

            switch (kind) {
            case SymbolKind::kAttribute:    type_index = 1; break;
            case SymbolKind::kInterface:    type_index = 4; break;
            case SymbolKind::kStruct:       type_index = 5; break;
            case SymbolKind::kParameter:    type_index = 7; break;
            case SymbolKind::kVariable:     type_index = 8; break;

            case SymbolKind::kFunctionDecl:
            case SymbolKind::kFunctionImpl:
                type_index = 12;
                break;

            case SymbolKind::kMacro:        type_index = 14; break;
            case SymbolKind::kPreprocessor: type_index = 15; break;
            default:
                break;
            }

            return type_index;
        };

        int GetTokenSemanticHighlight(TokenType type) {
            int type_index = -1;

            switch (type) {
            case TokenType::kPrimitive:
            case TokenType::kSpirvIntrinsic:
                type_index = 23; break;
            case TokenType::kBuiltInType:     type_index = 1;  break;
            case TokenType::kBuiltInFunction: type_index = 12; break;

            case TokenType::kKeyword:
            case TokenType::kPreprocessor:
            case TokenType::kSharp:
                type_index = 15;
                break;

            case TokenType::kNumberLiteral:   type_index = 19; break;
            default:
                break;
            }

            return type_index;
        };
    }

    std::vector<std::uint32_t> GetSemanticData(
        Context& context,
        Snapshot snapshot,
        const SourceFile* source_file,
        const Unicode::PositionMapper& mapper)
    {
        if (snapshot == nullptr) {
            return {};
        }

        std::vector<std::uint32_t> data;
        std::uint32_t last_line = 0;
        std::uint32_t last_char = 0;

        auto EmitSemanticData = [&](int type_index, int modifiers, const Token& token) -> void {
            if (type_index == -1 && modifiers == 0) {
                return;
            }

            const auto line       = static_cast<std::size_t>(token.location.line() - 1);
            const auto character  = mapper.ToUtf16Character(token.location.line(), token.location.column());
            const auto length     = Unicode::Utf16Length(token.text);
            const auto delta_line = line - last_line;
            const auto delta_char = (delta_line == 0) ? (character - last_char) : character;

            data.push_back(static_cast<std::uint32_t>(delta_line));
            data.push_back(static_cast<std::uint32_t>(delta_char));
            data.push_back(static_cast<std::uint32_t>(length));
            data.push_back(static_cast<std::uint32_t>(type_index));
            data.push_back(modifiers);

            last_line = static_cast<std::uint32_t>(line);
            last_char = static_cast<std::uint32_t>(character);
        };

        auto IsInactive = [&snapshot, source_file](std::uint32_t line) -> bool {
            if (snapshot->inactive_regions.empty()) {
                return false;
            }

            if (snapshot->inactive_regions.find(source_file) == snapshot->inactive_regions.end()) {
                return false;
            }

            const auto& regions = snapshot->inactive_regions.at(source_file);
            auto it = std::ranges::upper_bound(regions, line, std::ranges::less{}, [](InactiveRegion region) -> std::uint32_t {
                return region.begin_line;
            });

            if (it == regions.begin()) {
                return false;
            }

            --it;

            return it->begin_line <= line && line <= it->end_line;
        };

        auto IsReadonlySymbol = [](const SymbolInfo* symbol) -> bool {
            if (symbol->type_info.is_const()) {
                return true;
            }

            return std::ranges::any_of(symbol->type_info.qualifiers, [](const Token& qualifier) -> bool {
                return qualifier.text == "readonly";
            });
        };

        for (const auto& token : snapshot->raw_tokens) {
            ABORT_IF_CANCELLED();

            std::uint32_t modifiers = 0;
            if (IsInactive(token.location.line())) {
                modifiers |= (1 << 10); // inactive
            }

            int type_index = -1;

            auto it = snapshot->bindings.find(token.location);
            if (it == snapshot->bindings.end()) {
                type_index = GetTokenSemanticHighlight(token.type);
                EmitSemanticData(type_index, modifiers, token);
                continue;
            }

            const SymbolInfo* symbol = nullptr;

            std::visit(Overloaded{
                [&](const SymbolInfo* symbol_) -> void {
                    symbol = symbol_;
                },
                [&](const SymbolListView list) -> void {
                    if (!list.empty()) {
                        symbol = list.front();
                    }
                },
                [](std::monostate) -> void {}
            }, it->second);

            if (symbol == nullptr) {
                type_index = GetTokenSemanticHighlight(token.type);
                EmitSemanticData(type_index, modifiers, token);
                continue;
            }

            type_index = GetSymbolSemanticHighlight(symbol->kind);

            if ((symbol->kind == SymbolKind::kVariable ||
                 symbol->kind == SymbolKind::kParameter) &&
                IsReadonlySymbol(symbol))
            {
                modifiers |= (1 << 2); // readonly
            }

            if (token.location.line()   == symbol->location.line() &&
                token.location.column() == symbol->location.column())
            {
                modifiers |= (1 << 0); // declaration
            }

            if (token.type == TokenType::kIdentifier &&
                symbol->located_scope->kind() == ScopeKind::kGlobalTransparent)
            {
                modifiers |= (1 << 3); // static
            }

            EmitSemanticData(type_index, modifiers, token);
        }

        return data;
    }

    namespace {
        std::string_view TrimIncludeRootSeparators(std::string_view path) {
            while (!path.empty() && (path.front() == '/' || path.front() == '\\')) {
                path.remove_prefix(1);
            }

            return path;
        }

        std::optional<std::string_view> ExtractIncludeExpr(const PreprocessorNode* node, const SourceLocation& location) {
            if (node == nullptr || node->directive != "include") {
                return std::nullopt;
            }

            for (const auto& token : node->tokens) {
                if (!IsPositionInToken(token, location)) {
                    continue;
                }

                if (token.type == TokenType::kStringLiteral && token.text.length() > 2) {
                    if ((token.text.front() == '"' && token.text.back() == '"') ||
                        (token.text.front() == '<' && token.text.back() == '>'))
                    {
                        return token.text;
                    }
                }
            }

            return std::nullopt;
        }

        std::optional<std::filesystem::path> ResolveIncludeFilename(
            std::string_view includer_uri,
            std::string_view include_expr,
            IncludeDirectoryHandle include_dirs)
        {
            // #include "/path/to/include.glsl"
            const auto filename = TrimIncludeRootSeparators(include_expr.substr(1, include_expr.length() - 2));
            if (filename.empty()) {
                return std::nullopt;
            }

            if (!include_expr.starts_with('<')) {
                const auto includer = Utils::UriToPath(includer_uri);

                auto result = Utils::NormalizePath(includer.parent_path() / filename);
                if (std::filesystem::exists(result)) {
                    return result;
                }
            }

            for (const auto& dir : *include_dirs) {
                auto result = Utils::NormalizePath(dir / filename);
                if (std::filesystem::exists(result)) {
                    return result;
                }
            }

            return std::nullopt;
        }
    }

    std::optional<std::string> GotoInclude(
        Context& context,
        Snapshot snapshot,
        const SourceLocation& location,
        IncludeDirectoryHandle include_dirs)
    {
        if (snapshot == nullptr) {
            return std::nullopt;
        }

        const auto index = FindCursorTokenIndex(snapshot->raw_tokens, location);
        if (!index.has_value()) {
            return std::nullopt;
        }

        const auto& cursor_token = snapshot->raw_tokens[*index];
        if (cursor_token.type != TokenType::kStringLiteral) {
            return std::nullopt;
        }

        for (const auto* node : snapshot->ast->pprefs) {
            ABORT_IF_CANCELLED();

            const auto include_expr = ExtractIncludeExpr(node, location);
            if (!include_expr.has_value()) {
                continue;
            }

            const auto result = ResolveIncludeFilename(location.uri(), *include_expr, include_dirs);
            if (!result.has_value()) {
                return std::nullopt;
            }

            return Utils::PathToUri(*result);
        }

        return std::nullopt;
    }

    namespace {
        const SymbolInfo* ResolveFunctionJump(const Document* snapshot, const SymbolInfo* symbol) {
            if (symbol == nullptr) {
                return nullptr;
            }

            const auto  base_name  = Utils::UnmangleFunctionName(symbol->name);
            const auto& candidates = snapshot->symbols.FindFunctionsByOriginalName(base_name);

            if (std::holds_alternative<std::monostate>(candidates) ||
                std::holds_alternative<const SymbolInfo*>(candidates))
            {
                return nullptr;
            }

            return FindFunctionCounterpart(std::get<SymbolList>(candidates), symbol);
        };

        void GetDefinitionSymbolsFromCursor(Snapshot snapshot, const Token* cursor_token, bool toggle_function, SymbolList& results) {
            auto it = snapshot->bindings.find(cursor_token->location);
            if (it == snapshot->bindings.end()) {
                return;
            }

            auto AddSymbol = [&](const SymbolInfo* linked_symbol) -> void {
                if (linked_symbol == nullptr) {
                    return;
                }

                if (linked_symbol->kind == SymbolKind::kFunctionDecl ||
                    linked_symbol->kind == SymbolKind::kFunctionImpl)
                {
                    if (!toggle_function) {
                        results.push_back(linked_symbol);
                        return;
                    }

                    const bool clicked_on_definition = (cursor_token->location == linked_symbol->location);
                    if (clicked_on_definition) {
                        const auto* toggled = ResolveFunctionJump(snapshot.get(), linked_symbol);
                        results.push_back(toggled != nullptr ? toggled : linked_symbol);
                    } else {
                        if (linked_symbol->kind == SymbolKind::kFunctionDecl) {
                            const auto* impl = ResolveFunctionJump(snapshot.get(), linked_symbol);
                            results.push_back(impl != nullptr ? impl : linked_symbol);
                        } else {
                            results.push_back(linked_symbol);
                        }
                    }

                    return;
                }

                results.push_back(linked_symbol);
            };

            std::visit(Overloaded{
                [&](const SymbolInfo* symbol) -> void {
                    AddSymbol(symbol);
                },
                [&](SymbolListView list) -> void {
                    for (const auto* symbol : list) {
                        AddSymbol(symbol);
                    }
                },
                [](std::monostate) -> void {}
            }, it->second);
        }
    }

    SymbolList GetDefinitionSymbols(
        Context& context,
        Snapshot snapshot,
        const SourceLocation& location,
        bool toggle_function)
    {
        if (snapshot == nullptr) {
            return {};
        }

        const Token* cursor_token = nullptr;

        const auto index = FindCursorTokenIndex(snapshot->raw_tokens, location);
        if (index.has_value()) {
            cursor_token = &snapshot->raw_tokens[*index];
        }

        SymbolList results;
        if (cursor_token != nullptr) {
            ABORT_IF_CANCELLED();
            GetDefinitionSymbolsFromCursor(snapshot, cursor_token, toggle_function, results);
        }

        if (!results.empty()) {
            return results;
        }

        ABORT_IF_CANCELLED();
        LeafLocator locator(*snapshot, location);
        const auto* node = locator.result();

        if (node == nullptr || node->kind() != AstNodeKind::kVariableExpression) {
            return {};
        }

        auto* var_expr = static_cast<const VariableExpressionNode*>(node);

        std::visit(Overloaded{
            [&](const SymbolInfo* symbol) -> void {
                if (symbol != nullptr) {
                    results.push_back(symbol);
                }
            },
            [&](SymbolListView list) -> void {
                if (!list.empty()) {
                    results.append_range(list);
                }
            },
            [](std::monostate) -> void {}
        }, var_expr->linked_symbols);

        return results;
    }

    ReferenceResult GetReferences(
        Context& context,
        Snapshot snapshot,
        const SourceLocation& location,
        const ReferenceQuery& query)
    {
        if (snapshot == nullptr) {
            return {};
        }

        const auto index = FindCursorTokenIndex(snapshot->raw_tokens, location);
        if (!index.has_value()) {
            return {};
        }

        const auto& cursor_token = snapshot->raw_tokens[*index];
        auto it = snapshot->bindings.find(cursor_token.location);
        if (it == snapshot->bindings.end()) {
            return {};
        }

        const SymbolInfo* symbol = nullptr;
        if (std::holds_alternative<const SymbolInfo*>(it->second)) {
            symbol = std::get<const SymbolInfo*>(it->second);
        } else if (std::holds_alternative<SymbolListView>(it->second)) {
            const auto& symbols = std::get<SymbolListView>(it->second);
            if (!symbols.empty()) {
                symbol = symbols.front();
            }
        }

        if (symbol == nullptr) {
            return {};
        }

        ABORT_IF_CANCELLED();
        auto locations = query(symbol->location);

        return {
            .locations = std::move(locations),
            .symbol    = symbol
        };
    }

    std::vector<InlayHint> GetInlayHints(Context& context, Snapshot snapshot) {
        if (snapshot == nullptr || snapshot->ast == nullptr) {
            return {};
        }

        ABORT_IF_CANCELLED();
        InlayHintCollector collector(*snapshot);
        return collector.hints();
    }

    namespace {
        SymbolList DeduplicateSignatures(const SymbolList& condidates) {
            SymbolList unique_signatures;

            for (const auto* condidate : condidates) {
                if (condidate->kind == SymbolKind::kFunctionDecl &&
                    FindFunctionCounterpart(condidates, condidate) != nullptr)
                {
                    continue;
                }

                unique_signatures.push_back(condidate);
            }

            return unique_signatures;
        }

        void ClampToVariadic(int& index, const SymbolInfo* symbol) {
            auto* func = static_cast<const FunctionDeclarationNode*>(symbol->node);
            if (func != nullptr && !func->params.empty() && func->params.back()->is_variadic) {
                index = std::min(index, static_cast<int>(func->params.size() - 1));
            }
        }

        std::pair<std::vector<std::string>, int> BuildBufferReferenceSignatures(
            const CallExpressionNode* call,
            const VariableExpressionNode* callee,
            const SymbolInfo* target)
        {
            std::vector<std::string> signatures;
            int active_signature = 0;

            const TypeInfo* argument_type = nullptr;
            if (!call->args.empty() && call->args.front() != nullptr) {
                argument_type = &call->args.front()->evaluated_type;
            }

            auto AddReferenceSignature = [&](const SymbolInfo* source) -> void {
                const int index = static_cast<int>(signatures.size());
                signatures.push_back(std::format("{}({} _Ref)", target->name, source->name));

                if (argument_type != nullptr && argument_type->block_symbol == source) {
                    active_signature = index;
                }
            };

            AddReferenceSignature(target);

            SymbolList visible_symbols;
            callee->located_scope->GetVisibleSymbols(visible_symbols);

            for (const auto* symbol : visible_symbols) {
                if (symbol != target && Utils::HasInterfaceLayoutQualifier(symbol, "buffer_reference")) {
                    AddReferenceSignature(symbol);
                }
            }

            int index = static_cast<int>(signatures.size());
            signatures.push_back(std::format("{}(uint64_t _Address)", target->name));

            if (argument_type != nullptr &&
                argument_type->block_symbol == nullptr &&
                argument_type->type_desc.family == BaseFamily::kUint &&
                argument_type->type_desc.bits == 64 &&
                argument_type->type_desc.vector_count == 1 &&
                argument_type->type_desc.vector_length == 1)
            {
                active_signature = index;
            }

            index = static_cast<int>(signatures.size());
            signatures.push_back(std::format("{}(uvec2 _Address)", target->name));

            if (argument_type != nullptr &&
                argument_type->block_symbol == nullptr &&
                argument_type->type_desc.family == BaseFamily::kUint &&
                argument_type->type_desc.bits == 32 &&
                argument_type->type_desc.vector_count == 1 &&
                argument_type->type_desc.vector_length == 2)
            {
                active_signature = index;
            }

            return { std::move(signatures), active_signature };
        }

        struct MacroSignatureContext {
            const PreprocessorNode* definition{ nullptr };
            int active_param_index{};
        };

        std::optional<MacroSignatureContext> FindMacroSignatureContext(
            const Document& document,
            const SourceLocation& location)
        {
            struct DelimiterFrame {
                TokenType    open{};
                const Token* callee{};
                int          active_param_index{};
            };

            std::vector<DelimiterFrame> stack;
            const auto& tokens = document.raw_tokens;

            for (auto i = 0uz; i != tokens.size() && tokens[i].location < location; ++i) {
                const auto& token = tokens[i];

                if (token.type == TokenType::kOpenParen) {
                    stack.push_back(DelimiterFrame{
                        .open               = token.type,
                        .callee             = i > 0 ? &tokens[i - 1] : nullptr,
                        .active_param_index = 0
                    });
                    continue;
                }

                if (token.type == TokenType::kOpenBracket || token.type == TokenType::kOpenBrace) {
                    stack.push_back(DelimiterFrame{
                        .open = token.type
                    });
                    continue;
                }

                if (token.type == TokenType::kCloseParen ||
                    token.type == TokenType::kCloseBracket ||
                    token.type == TokenType::kCloseBrace)
                {
                    if (!stack.empty()) {
                        stack.pop_back();
                    }
                    continue;
                }

                if (token.type == TokenType::kComma && !stack.empty() &&
                    stack.back().open == TokenType::kOpenParen)
                {
                    ++stack.back().active_param_index;
                }
            }

            for (const auto& frame : stack | std::views::reverse) {
                if (frame.callee == nullptr) {
                    continue;
                }

                const Token* definition_token = nullptr;

                if (const auto trace = document.macro_traces.find(frame.callee->location);
                    trace != document.macro_traces.end())
                {
                    definition_token = &trace->second;
                }
                else if (const auto arg_trace = document.macro_args_traces.find(frame.callee->location);
                         arg_trace != document.macro_args_traces.end() &&
                         arg_trace->second.definition.has_value())
                {
                    definition_token = &*arg_trace->second.definition;
                }

                if (definition_token == nullptr) {
                    return std::nullopt;
                }

                const auto* symbol = document.symbols.FindMacroSymbol(*definition_token);
                if (symbol == nullptr || symbol->node == nullptr ||
                    symbol->node->kind() != AstNodeKind::kPreprocessor)
                {
                    return std::nullopt;
                }

                auto* definition = static_cast<const PreprocessorNode*>(symbol->node);
                if (!definition->is_function) {
                    return std::nullopt;
                }

                return MacroSignatureContext{
                    .definition         = definition,
                    .active_param_index = frame.active_param_index
                };
            }

            return std::nullopt;
        }

        std::string FormatMacroSignature(const PreprocessorNode* node) {
            auto signature = node->symbol->name;
            signature += "(";

            for (const auto [i, param] : node->params | std::views::enumerate) {
                signature += param;

                if (!std::cmp_equal(i + 1, node->params.size())) {
                    signature += ", ";
                }
            }

            signature += ")";
            return signature;
        }
    }

    std::optional<SignatureHelpResult> GetSignatureHelp(
        Context& context,
        Snapshot snapshot,
        const SourceLocation& location)
    {
        if (snapshot == nullptr) {
            return std::nullopt;
        }

        if (const auto macro = FindMacroSignatureContext(*snapshot, location)) {
            return SignatureHelpResult{
                .signatures             = { FormatMacroSignature(macro->definition) },
                .active_signature_index = 0,
                .active_param_index     = macro->active_param_index
            };
        }

        ABORT_IF_CANCELLED();
        SignatureLocator locator(*snapshot, location);
        const auto* node = locator.result();

        if (node == nullptr || node->callee == nullptr ||
            node->callee->kind() != AstNodeKind::kVariableExpression)
        {
            return std::nullopt;
        }

        auto* callee = static_cast<const VariableExpressionNode*>(node->callee);

        const auto open_paren_loc = callee->end;
        auto it = std::ranges::lower_bound(snapshot->raw_tokens, open_paren_loc, std::ranges::less{}, &Token::location);

        int active_param_index = 0;
        int paren_level        = 0;
        while (it != snapshot->raw_tokens.end() && it->location < location) {
            ABORT_IF_CANCELLED();

            if (it->type == TokenType::kOpenParen) {
                ++paren_level;
            } else if (it->type == TokenType::kCloseParen) {
                --paren_level;
            } else if (paren_level == 1 && it->type == TokenType::kComma) {
                ++active_param_index;
            }

            ++it;
        }

        if (paren_level <= 0) {
            return std::nullopt;
        }

        std::vector<TypeInfo> current_arg_types;
        for (auto i = 0uz; i <= active_param_index && i < node->args.size(); ++i) {
            ABORT_IF_CANCELLED();

            if (node->args[i] != nullptr) {
                current_arg_types.push_back(node->args[i]->evaluated_type);
            } else {
                current_arg_types.push_back({
                    .typename_token = {
                        .text = "unknown",
                        .type = TokenType::kUnknown
                    }
                });
            }
        }

        ABORT_IF_CANCELLED();

        if (auto* linked = std::get_if<const SymbolInfo*>(&callee->linked_symbols);
            linked != nullptr && *linked != nullptr)
        {
            const auto* symbol = *linked;

            if (symbol->kind == SymbolKind::kStruct) {
                return SignatureHelpResult{
                    .signatures             = { FormatStructConstructor(symbol, snapshot).full_spec },
                    .active_signature_index = 0,
                    .active_param_index     = active_param_index
                };
            }

            if (Utils::HasInterfaceLayoutQualifier(symbol, "buffer_reference")) {
                auto [signatures, active_signature] = BuildBufferReferenceSignatures(node, callee, symbol);

                return SignatureHelpResult{
                    .signatures             = std::move(signatures),
                    .active_signature_index = active_signature,
                    .active_param_index     = active_param_index
                };
            }

            if (symbol->kind == SymbolKind::kInterface) {
                return std::nullopt;
            }
        }

        const auto candidates = snapshot->symbols.FindFunctionsByOriginalName(callee->name);
        if (std::holds_alternative<std::monostate>(candidates)) {
            return std::nullopt;
        }

        if (auto* symbol = std::get_if<const SymbolInfo*>(&candidates)) {
            ClampToVariadic(active_param_index, *symbol);
            auto format_result = FormatFunctionSymbol(*symbol, snapshot);

            return SignatureHelpResult{
                .signatures             = { std::move(format_result.full_spec) },
                .active_signature_index = 0,
                .active_param_index     = active_param_index
            };
        }

        auto unique_candidates = DeduplicateSignatures(std::get<SymbolList>(candidates));
        const auto ranked = TypeResolver::RankSignatureCandidates(unique_candidates, current_arg_types);

        if (ranked.candidates.empty()) {
            return std::nullopt;
        }

        ClampToVariadic(active_param_index, ranked.candidates[ranked.active_index]);

        std::vector<std::string> signatures;
        for (const auto* symbol : ranked.candidates) {
            signatures.push_back(FormatFunctionSymbol(symbol, snapshot).full_spec);
        }

        return SignatureHelpResult{
            .signatures             = std::move(signatures),
            .active_signature_index = ranked.active_index,
            .active_param_index     = active_param_index
        };
    }

    namespace {
        struct IncludeCompletionContext {
            std::string_view prefix;
            std::string_view uri;
            std::uint32_t    line{};
            std::uint32_t    replace_start_column{};
            std::uint32_t    replace_end_column{};
            bool             system_include{};
        };

        std::optional<IncludeCompletionContext> TryBuildIncludeCompletionContext(
            const PreprocessorNode* node,
            const SourceLocation& location)
        {
            if (node == nullptr || node->directive != "include") {
                return std::nullopt;
            }

            for (const auto& token : node->tokens) {
                if (!IsPositionInToken(token, location))
                    continue;
                if (token.type != TokenType::kStringLiteral || token.text.length() < 2)
                    continue;

                if ((token.text.front() != '"' && token.text.back() != '"') &&
                    (token.text.front() != '<' && token.text.back() != '>'))
                {
                    continue;
                }

                const auto literal_start = token.location.column(); // " or <
                const auto content_start = literal_start + 1;
                const auto content_end   = literal_start + static_cast<std::uint32_t>(token.text.length()) - 1;
                const auto clamped       = std::clamp(location.column(), content_start, content_end);
                const auto prefix_length = clamped > content_start ? (clamped - content_start) : 0;

                IncludeCompletionContext context{
                    .prefix               = token.text.substr(1, prefix_length),
                    .uri                  = location.uri(),
                    .line                 = location.line(),
                    .replace_start_column = content_start,
                    .replace_end_column   = content_end,
                    .system_include       = (token.text.front() == '<' && token.text.back() == '>')
                };

                return context;
            }

            return std::nullopt;
        }

        std::vector<std::filesystem::path> BuildSearchRoots(
            const IncludeCompletionContext& context,
            IncludeDirectoryHandle include_dirs)
        {
            std::vector<std::filesystem::path> roots;

            if (!context.system_include) {
                const auto includer = Utils::UriToPath(context.uri);
                PushUniquePath(roots, includer.parent_path());
            }

            for (const auto& dir : *include_dirs) {
                PushUniquePath(roots, dir);
            }

            return roots;
        }

        std::pair<std::string, std::string> SplitPrefix(std::string_view prefix) {
            std::string dir_prefix;
            std::string file_prefix;

            const auto position = prefix.find_last_of("/\\");
            if (position == std::string_view::npos) {
                dir_prefix.clear();
                file_prefix = prefix;
            } else {
                dir_prefix  = prefix.substr(0, position + 1); // keep "/"
                file_prefix = prefix.substr(position + 1);
            }

            return { dir_prefix, file_prefix };
        }
    }

    nlohmann::json GetIncludeCompletionItems(
        Context& context,
        Snapshot snapshot,
        const SourceLocation& location,
        IncludeDirectoryHandle include_dirs,
        Unicode::PositionMapper& mapper)
    {
        if (snapshot == nullptr) {
            return {};
        }

        const auto index = FindCursorTokenIndex(snapshot->raw_tokens, location);
        if (!index.has_value()) {
            return {};
        }

        const auto& cursor_token = snapshot->raw_tokens[*index];
        if (cursor_token.type != TokenType::kStringLiteral) {
            return {};
        }

        std::optional<IncludeCompletionContext> include_context;
        for (const auto& node : snapshot->ast->pprefs) {
            ABORT_IF_CANCELLED();

            include_context = TryBuildIncludeCompletionContext(node, location);
            if (include_context.has_value()) {
                break;
            }
        }

        if (!include_context.has_value()) {
            return {};
        }

        ABORT_IF_CANCELLED();

        const auto roots = BuildSearchRoots(*include_context, include_dirs);
        if (roots.empty()) {
            return {};
        }

        const auto [dir_prefix, file_prefix] = SplitPrefix(include_context->prefix);
        const auto search_dir_prefix         = TrimIncludeRootSeparators(dir_prefix);

        StringHeteroHashSet unique_labels;
        nlohmann::json items = nlohmann::json::array();

        for (const auto& root : roots) {
            ABORT_IF_CANCELLED();

            const auto base = Utils::NormalizePath(root / search_dir_prefix);
            if (!std::filesystem::exists(base) || !std::filesystem::is_directory(base)) {
                continue;
            }

            std::error_code ec;
            for (const auto& entry : std::filesystem::directory_iterator(base, ec)) {
                ABORT_IF_CANCELLED();

                if (ec) {
                    break;
                }

                const auto name = entry.path().filename().generic_string();
                if (!file_prefix.empty() && !name.starts_with(file_prefix)) {
                    continue;
                }

                auto relative_path = dir_prefix + name;
                if (entry.is_directory()) {
                    relative_path += "/";
                }

                if (!unique_labels.insert(relative_path).second) {
                    continue;
                }

                const auto kind = entry.is_directory() ? 19 : 17;  // Folder : File

                nlohmann::json item;
                item["label"]    = relative_path;
                item["kind"]     = kind;
                item["sortText"] = kind == 19 ? "0" : "1";
                item["textEdit"] = {
                    { "range", {
                        { "start", {
                            { "line", include_context->line - 1 },
                            { "character", mapper.ToUtf16Character(include_context->line, include_context->replace_start_column) }
                        } },
                        { "end", {
                            { "line", include_context->line - 1 },
                            { "character", mapper.ToUtf16Character(include_context->line, include_context->replace_end_column) }
                        } }
                    } },
                    { "newText", relative_path }
                };

                if (entry.is_directory()) {
                    item["command"] = {
                        { "title",   "Trigger include completion" },
                        { "command", "editor.action.triggerSuggest" }
                    };
                }

                items.push_back(item);
            }
        }

        return items;
    }

    namespace {
        int MapSymbolKindToLspCompletion(SymbolKind kind, bool is_const = false) {
            switch (kind) {
            case SymbolKind::kVariable:  return is_const ? 21 : 6; // 常量用 Constant(21)，普通变量用 Variable(6)
            case SymbolKind::kParameter: return 6;  // Variable
            case SymbolKind::kStruct:    return 22; // Struct
            case SymbolKind::kInterface: return 8;  // Interface

            case SymbolKind::kFunctionDecl:
            case SymbolKind::kFunctionImpl:
                return 3;  // Function

            case SymbolKind::kMacro:     return 9;  // Module (或者 Constant)
            default:                     return 1;  // Text
            }
        };

        int SubtypeToKind(std::string_view subtype) {
            if (subtype == "Builtins.Types")
                return 22; // Struct
            if (subtype == "Primitives.Types" || subtype == "Keywords.Meta")
                return 14; // Keyword
            if (subtype == "Preprocessors.Meta")
                return 21; // Constant
            if (subtype == "Builtins.Functions")
                return 3;  // Function
            if (subtype == "Builtins.Variables")
                return 6;  // Variable
            if (subtype == "Primitives.Qualifiers" || subtype.contains("Layout"))
                return 10; // Enum
            return 1; // Text
        };

        StringHeteroHashMap<const SymbolInfo*> GetActiveMacrosAt(const Document* snapshot, const SourceLocation& location) {
            if (snapshot->ast == nullptr) {
                return {};
            }

            StringHeteroHashMap<const SymbolInfo*> active_macros;
            const auto& references = snapshot->ast->pprefs;

            for (const auto* node : references) {
                if (node != nullptr && node->directive == "define" &&
                    node->symbol != nullptr && node->begin.line() == 0)
                {
                    active_macros.insert_or_assign(node->symbol->name, node->symbol);
                }
            }

            for (const auto* node : references) {
                if (node == nullptr || node->begin.line() == 0) {
                    continue;
                }

                if (node->begin.source_file() == location.source_file() && location <= node->end) {
                    break;
                }

                if (node->directive == "define") {
                    if (node->symbol != nullptr) {
                        active_macros.insert_or_assign(node->symbol->name, node->symbol);
                    }
                    continue;
                }

                if (node->directive == "undef" && !node->tokens.empty() &&
                    node->tokens.front().type == TokenType::kIdentifier)
                {
                    active_macros.erase(node->tokens.front().text);
                }
            }

            return active_macros;
        }
    }

    nlohmann::json GetCompletionItems(
        Context& context,
        Snapshot snapshot,
        const SourceLocation& location)
    {
        if (snapshot == nullptr) {
            return {};
        }

        const auto* located_scope = snapshot->symbols.FindScopeAt(location);

        ABORT_IF_CANCELLED();
        std::vector<const SymbolInfo*> visible_symbols;

        const auto active_macros = GetActiveMacrosAt(snapshot.get(), location);
        for (const auto& [_, symbol] : active_macros) {
            visible_symbols.push_back(symbol);
        }

        located_scope->GetVisibleSymbols(visible_symbols);

        nlohmann::json items = nlohmann::json::array();
        StringHeteroHashSet existing_labels;

        for (const auto* symbol : visible_symbols) {
            ABORT_IF_CANCELLED();

            nlohmann::json item;
            std::string_view symbol_name;

            if (symbol->kind == SymbolKind::kFunctionDecl || symbol->kind == SymbolKind::kFunctionImpl) {
                symbol_name = Utils::UnmangleFunctionName(symbol->name);
            } else {
                symbol_name = symbol->name;
            }

            if (symbol_name.empty() || existing_labels.contains(symbol_name)) {
                continue;
            }

            existing_labels.emplace(symbol_name);
            item["label"] = symbol_name;
            item["kind"]  = MapSymbolKindToLspCompletion(symbol->kind, symbol->type_info.is_const());

            // TODO: document, detail, etc
            items.push_back(item);
        }

        const auto& meta = MetadataManager::GetInstance().GetMeta();
        for (const auto& [subtype, name] : meta) {
            if (existing_labels.contains(name)) {
                continue;
            }

            existing_labels.emplace(name);

            nlohmann::json item;
            item["label"] = name;
            item["kind"]  = SubtypeToKind(subtype);
            items.push_back(item);
        }

        return items;
    }

    nlohmann::json GetFieldCompletionItems(
        Context& context,
        Snapshot snapshot,
        const SourceLocation& location,
        const TypeMemberIndex& type_member_index)
    {
        if (snapshot == nullptr) {
            return {};
        }

        SourceLocation dot_location;
        auto it = std::ranges::upper_bound(snapshot->raw_tokens, location, std::ranges::less{}, &Token::location);
        if (it != snapshot->raw_tokens.begin()) {
            dot_location = std::prev(it)->location;
        }

        ABORT_IF_CANCELLED();
        ContextLocator locator(*snapshot, dot_location);
        const auto* node = locator.result();

        auto* expr_node = dynamic_cast<const ExpressionNode*>(node);
        if (expr_node == nullptr) {
            return {};
        }

        const auto& type_info = expr_node->evaluated_type;
        if (!type_info.is_valid()) {
            return {};
        }

        SymbolList fields;
        if (type_info.block_symbol != nullptr && !type_info.block_symbol->name.empty()) {
            fields = type_member_index.GetMembers(type_info.block_symbol->name);
        }

        if (fields.empty() && type_info.block_symbol != nullptr &&
            type_info.block_symbol->internal_scope != nullptr)
        {
            const auto* scope = type_info.block_symbol->internal_scope;
            for (const auto& symbol : scope->symbols()) {
                ABORT_IF_CANCELLED();
                fields.push_back(symbol.second.get());
            }
        }

        StringHeteroHashSet existing_labels;
        nlohmann::json items = nlohmann::json::array();

        for (const auto* field : fields) {
            ABORT_IF_CANCELLED();

            if (existing_labels.contains(field->name)) {
                continue;
            }

            existing_labels.emplace(field->name);

            nlohmann::json item;
            item["label"] = field->name;
            item["kind"]  = 5;

            items.push_back(item);
        }

        return items;
    }

    nlohmann::json GetExtensionCompletionItems(
        Context& context,
        Snapshot snapshot,
        const SourceLocation& location)
    {
        if (snapshot == nullptr) {
            return {};
        }

        const auto cursor = std::ranges::upper_bound(snapshot->raw_tokens, location, std::ranges::less{}, &Token::location);
        if (cursor == snapshot->raw_tokens.begin()) {
            return {};
        }

        auto cursor_index = static_cast<std::size_t>(std::distance(snapshot->raw_tokens.begin(), std::prev(cursor)));
        if (snapshot->raw_tokens[cursor_index].type == TokenType::kEndOfFile) {
            if (cursor_index == 0) {
                return {};
            }

            --cursor_index;
        }

        if (snapshot->raw_tokens[cursor_index].location.line() != location.line()) {
            return {};
        }

        const auto start = cursor_index > 2 ? cursor_index - 2 : 0;
        bool is_extension = false;
        for (auto i = cursor_index;; --i) {
            ABORT_IF_CANCELLED();
            if (snapshot->raw_tokens[i].type == TokenType::kSharp) {
                if (i + 1 < snapshot->raw_tokens.size() &&
                    snapshot->raw_tokens[i + 1].text == "extension")
                {
                    is_extension = true;
                }

                break;
            }

            if (i == start) {
                break;
            }
        }

        if (!is_extension) {
            return {};
        }

        const auto root = Utils::GetFilePath("Database/Meta/Extensions/Main");
        std::error_code ec;
        nlohmann::json items = nlohmann::json::array();

        for (const auto& entry : std::filesystem::recursive_directory_iterator(root, ec)) {
            ABORT_IF_CANCELLED();

            if (ec || !entry.is_regular_file()) {
                continue;
            }

            const auto name = entry.path().stem().generic_string();
            if (name.empty()) {
                continue;
            }

            nlohmann::json item;
            item["label"] = name;
            item["kind"]  = 9; // Module
            items.push_back(item);
        }

        return items;
    }

    namespace {
        bool IsAssignmentWithKey(const QualifierArgumentNode* node, std::string_view key) {
            if (node == nullptr || node->children.size() != 2 ||
                node->arg_kind != QualifierArgumentKind::kAssignment)
            {
                return false;
            }

            const auto& lhs = node->children.front();
            const auto& rhs = node->children.back();
            if (lhs == nullptr || rhs == nullptr || lhs->arg_kind != QualifierArgumentKind::kIdentifier) {
                return false;
            }

            if (lhs->token.text != key) {
                return false;
            }

            return true;
        }

        template <typename Ty>
        std::string SerializeArray(std::span<const Ty> array, auto&& pred) {
            std::string result = "[";
            for (auto i = 0uz; i != array.size(); ++i) {
                result += pred(array[i]);
                if (i + 1 != array.size()) {
                    result += ", ";
                }
            }

            result += "]";
            return result;
        }

        std::string SerializeStringArray(std::span<const std::string> array) {
            return SerializeArray<std::string>(array, [](std::string_view sv) -> std::string {
                return std::format("\"{}\"", sv);
            });
        }

        std::string SerializeIntegerArray(std::span<const std::int64_t> array) {
            return SerializeArray<std::int64_t>(array, [](std::int64_t value) -> std::string {
                return std::to_string(value);
            });
        }

        std::string SerializeCanonicalLayoutParameters(std::span<const QualifierArgumentNode*> params) {
            auto& metadata = MetadataManager::GetInstance();
            StringHeteroHashMap<std::vector<std::string>> param_map;

            for (const auto* param : params) {
                if (param->arg_kind == QualifierArgumentKind::kIdentifier) {
                    const auto subtype = metadata.GetLexicalSubtype(param->token.text);
                    if (!subtype.has_value()) {
                        continue;
                    }

                    param_map[*subtype].push_back(std::string(param->token.text));
                    continue;
                }

                if (param->arg_kind == QualifierArgumentKind::kAssignment &&
                    !param->children.empty() &&
                    param->children.front()->arg_kind == QualifierArgumentKind::kIdentifier)
                {
                    const auto& identifier_token = param->children.front()->token;
                    const auto subtype = metadata.GetLexicalSubtype(identifier_token.text);
                    if (!subtype.has_value()) {
                        continue;
                    }

                    param_map[*subtype].push_back(Utils::SerializeQualifierArguments(param));
                }
            }

            std::string result;
            auto AppendBucket = [&result](std::span<const std::string> bucket) -> void {
                if (bucket.empty()) {
                    return;
                }

                if (!result.empty()) {
                    result += ", ";
                }

                for (auto i = 0uz; i != bucket.size(); ++i) {
                    result += bucket[i];
                    if (i + 1 != bucket.size()) {
                        result += ", ";
                    }
                }
            };

            for (const auto& bucket : param_map) {
                if (bucket.first.contains("LayoutDeclarers")) {
                    AppendBucket(bucket.second);
                }
            }

            for (const auto& bucket : param_map) {
                if (bucket.first.contains("LayoutArguments")) {
                    AppendBucket(bucket.second);
                }
            }

            return result;
        }

        std::string RenderMergedLayout(std::span<const LayoutQualifierNode* const> layouts) {
            std::vector<const QualifierArgumentNode*> canonical_params;
            for (const auto& layout : layouts) {
                if (layout == nullptr) {
                    continue;
                }

                for (const auto& param : layout->params) {
                    if (param != nullptr) {
                        canonical_params.push_back(param);
                    }
                }
            }

            if (canonical_params.empty()) {
                return {};
            }

            return std::format("layout({})", SerializeCanonicalLayoutParameters(canonical_params));
        }

        std::optional<std::vector<std::string>> CollectStringArray(const QualifierArgumentNode* rhs) {
            return Utils::CollectArgumentArray<std::string>(rhs, QualifierArgumentKind::kStringLiteral, Utils::UnquoteStringLiteral);
        }

        std::optional<std::vector<std::int64_t>> CollectIntegerArray(const QualifierArgumentNode* rhs) {
            return Utils::CollectArgumentArray<std::int64_t>(rhs, QualifierArgumentKind::kNumberLiteral, Utils::ParseNumberLiteralToInteger);
        }

        template <typename Ty>
        std::string SerializeArgumentArray(std::span<const Ty> array, auto&& pred) {
            std::string result = "[";
            for (auto i = 0uz; i != array.size(); ++i) {
                result += pred(array[i]);
                if (i + 1 != array.size()) {
                    result += ", ";
                }
            }

            result += "]";
            return result;
        }

        std::string RenderCanonicalSpirvCall(const SpirvIntrinsicNode* node) {
            if (node == nullptr) {
                return {};
            }

            const auto& params = node->params;
            const auto& name   = node->keyword.text;
            std::vector<std::string> ordered;

            std::vector<std::string>  exts_union;
            std::vector<std::int64_t> caps_union;
            std::vector<std::string>  positional;

            if (name == "spirv_type" || name == "spirv_instruction") {
                std::vector<std::string> sets;
                std::vector<std::string> ids;

                for (const auto& param : params) {
                    if (IsAssignmentWithKey(param, "extensions")) {
                        auto extensions = CollectStringArray(param->children.back());
                        if (!extensions.has_value()) {
                            continue;
                        }

                        exts_union.insert_range(exts_union.end(), *extensions | std::views::as_rvalue);
                        continue;
                    }

                    if (IsAssignmentWithKey(param, "capabilities")) {
                        auto capabilities = CollectIntegerArray(param->children.back());
                        if (!capabilities.has_value()) {
                            continue;
                        }

                        caps_union.insert_range(caps_union.end(), *capabilities | std::views::as_rvalue);
                        continue;
                    }

                    if (IsAssignmentWithKey(param, "sets")) {
                        sets.push_back(std::format("set = {}", Utils::SerializeQualifierArguments(param->children.back())));
                        continue;
                    }

                    if (IsAssignmentWithKey(param, "ids")) {
                        ids.push_back(std::format("ids = {}", Utils::SerializeQualifierArguments(param->children.back())));
                        continue;
                    }

                    positional.push_back(Utils::SerializeQualifierArguments(param));
                }

                std::ranges::sort(exts_union);
                auto [ext_first, ext_last] = std::ranges::unique(exts_union);
                exts_union.erase(ext_first, ext_last);

                std::ranges::sort(caps_union);
                auto [caps_first, caps_last] = std::ranges::unique(caps_union);
                caps_union.erase(caps_first, caps_last);

                if (!exts_union.empty())
                    ordered.push_back(std::format("extensions = {}", SerializeStringArray(exts_union)));
                if (!caps_union.empty())
                    ordered.push_back(std::format("capabilities = {}", SerializeIntegerArray(caps_union)));

                ordered.append_range(sets | std::views::as_rvalue);
                ordered.append_range(ids | std::views::as_rvalue);
                ordered.append_range(positional | std::views::as_rvalue);
            } else if (name == "spirv_decorate" || name == "spirv_decorate_id" || name == "spirv_decorate_string") {
                for (const auto& param : params) {
                    if (IsAssignmentWithKey(param, "extensions")) {
                        auto extensions = CollectStringArray(param->children.back());
                        if (!extensions.has_value()) {
                            continue;
                        }

                        exts_union.insert_range(exts_union.end(), *extensions | std::views::as_rvalue);
                        continue;
                    }

                    if (IsAssignmentWithKey(param, "capabilities")) {
                        auto capabilities = CollectIntegerArray(param->children.back());
                        if (!capabilities.has_value()) {
                            continue;
                        }

                        caps_union.insert_range(caps_union.end(), *capabilities | std::views::as_rvalue);
                        continue;
                    }

                    positional.push_back(Utils::SerializeQualifierArguments(param));
                }

                std::ranges::sort(exts_union);
                auto [ext_first, ext_last] = std::ranges::unique(exts_union);
                exts_union.erase(ext_first, ext_last);

                std::ranges::sort(caps_union);
                auto [caps_first, caps_last] = std::ranges::unique(caps_union);
                caps_union.erase(caps_first, caps_last);

                if (!exts_union.empty())
                    ordered.push_back(std::format("extensions = {}", SerializeStringArray(exts_union)));
                if (!caps_union.empty())
                    ordered.push_back(std::format("capabilities = {}", SerializeIntegerArray(caps_union)));

                ordered.append_range(positional | std::views::as_rvalue);
            } else {
                for (const auto& param : params) {
                    ordered.push_back(Utils::SerializeQualifierArguments(param));
                }
            }

            std::string inside;
            for (auto i = 0uz; i != ordered.size(); ++i) {
                inside += ordered[i];
                if (i + 1 != ordered.size()) {
                    inside += ", ";
                }
            }

            return std::format("{}({})", name, inside);
        }

        std::string ResolveAlias(const Document* snapshot, const SourceLocation& location) {
            if (snapshot == nullptr) {
                return {};
            }

            const auto& metadata = MetadataManager::GetInstance();

            auto it = snapshot->macro_traces.find(location);
            if (it != snapshot->macro_traces.end() && metadata.IsNoExpandHint(it->second.text)) {
                return std::string(it->second.text);
            }

            for (const auto& builtin : snapshot->builtins) {
#pragma warning(push)
#pragma warning(disable : 4456)
                auto it = builtin->macro_traces.find(location);
                if (it != builtin->macro_traces.end() && metadata.IsNoExpandHint(it->second.text)) {
                    return std::string(it->second.text);
                }
#pragma warning(pop)
            }

            return {};
        };

        std::string BuildTypeText(const auto* node, const Document* snapshot) {
            if (node->type_spec.spirv_type != nullptr) {
                auto alias = ResolveAlias(snapshot, node->type_spec.spirv_type->keyword.location);
                if (!alias.empty()) {
                    return alias;
                }

                return RenderCanonicalSpirvCall(node->type_spec.spirv_type);
            }

            auto& metadata = MetadataManager::GetInstance();

            const auto alias = ResolveAlias(snapshot, node->type_spec.typename_token().location);
            auto type_name = alias.empty() ? std::string(node->type_spec.typename_token().text) : alias;
            if (auto subtype = metadata.GetLexicalSubtype(type_name);
                subtype.has_value() && subtype->contains("Qualifiers"))
            {
                return {};
            }

            const auto* symbol = node->declared_symbol;
            if (symbol != nullptr) {
                return symbol->type_info.Format(type_name);
            }

            return type_name;
        }

        std::string BuildHoverSpecifierLine(const auto* node, const Document* snapshot) {
            std::vector<std::string> layer_exec_env;
            std::vector<std::string> layer_storage;
            std::vector<std::string> layer_decorate;
            const auto layer_layout = RenderMergedLayout(node->type_spec.layouts);

            auto& metadata = MetadataManager::GetInstance();

            for (const auto& spec : node->type_spec.specifiers) {
                if (spec.text == "layout" || spec.type == TokenType::kSpirvIntrinsic ||
                    spec.text == "true"   || spec.text == "false")
                {
                    continue;
                }

                const auto alias = ResolveAlias(snapshot, spec.location);
                auto text = alias.empty() ? (spec.text.contains("__AnonymousStruct_") ? "<anonymous>" : std::string(spec.text)) : std::string(alias);
                if (const auto subtype = metadata.GetLexicalSubtype(spec.text);
                    subtype.has_value() && subtype->contains("Qualifiers"))
                {
                    layer_storage.push_back(std::move(text));
                }
            }

            for (const auto& spirv : node->type_spec.spirv_intrinsics) {
                if (spirv == nullptr) {
                    continue;
                }

                const auto name  = spirv->keyword.text;
                const auto alias = ResolveAlias(snapshot, spirv->keyword.location);
                auto       call  = alias.empty() ? RenderCanonicalSpirvCall(spirv) : alias;

                if (name == "spirv_execution_mode" || name == "spirv_execution_mode_ide" || name == "spirv_instruction") {
                    layer_exec_env.push_back(std::move(call));
                } else if (name == "spirv_storage_class") {
                    layer_storage.push_back(std::move(call));
                } else if (name == "spirv_decorate" || name == "spirv_decorate_id" || name == "spirv_decorate_string") {
                    layer_decorate.push_back(std::move(call));
                } else if (name == "spirv_by_reference" || name == "spirv_literal") {
                    layer_decorate.push_back(std::string(name));
                }
            }

            std::string result;
            auto Append = [&result](std::string_view text, bool abbreviate_arguments) -> void {
                if (text.empty()) {
                    return;
                }

                std::string rendered;
                if (abbreviate_arguments) {
                    const auto open_paren = text.find('(');
                    rendered = open_paren == std::string_view::npos
                             ? std::string(text)
                             : std::string(text.substr(0, open_paren)) + "(...)";
                } else {
                    rendered = text;
                }

                if (!result.empty()) {
                    result += " ";
                }

                result += rendered;
            };

            for (const auto& layer : layer_exec_env)
                Append(layer, true);

            Append(layer_layout, true);

            for (const auto& layer : layer_decorate)
                Append(layer, true);
            for (const auto& layer : layer_storage)
                Append(layer, true);

            Append(BuildTypeText(node, snapshot), false);

            if (result.contains("layout") && !result.contains("set")) {
                if (auto pos = result.find("binding"); pos != std::string::npos) {
                    result.insert(pos, "set = 0, ");
                }
            }

            return result;
        }

        std::string FormatNamedDeclaration(const VariableDeclarationNode* node, const Document* snapshot) {
            auto result = BuildHoverSpecifierLine(node, snapshot);
            const auto* symbol = node->declared_symbol;
            if (symbol == nullptr) {
                return result;
            }

            const auto last_close_paren = result.find(')');
            const auto open_bracket = last_close_paren != std::string::npos
                                    ? result.find('[', last_close_paren)
                                    : result.find('[');

            if (open_bracket != std::string::npos) {
                result.insert(open_bracket, " " + symbol->name);
            } else {
                result += " " + symbol->name;
            }

            return result;
        }
    }

    FunctionFormatResult FormatFunctionSymbol(const SymbolInfo* symbol, Snapshot snapshot) {
        if (symbol == nullptr) {
            return {};
        }

        auto return_typename = symbol->type_info.spirv_type.empty()
                             ? symbol->type_info.Format()
                             : std::string(symbol->type_info.spirv_type);

        const auto raw_name = Utils::UnmangleFunctionName(symbol->name);
        auto result = std::format("{} {}(", return_typename, raw_name);

        std::vector<std::string> params;

        auto* node = static_cast<const FunctionDeclarationNode*>(symbol->node);
        for (auto i = 0uz; i != node->params.size(); ++i) {
            const auto& param = node->params[i];

            if (param->is_variadic) {
                result += "...";
                params.push_back("...");
                if (i + 1 != node->params.size()) {
                    result += ", ";
                }

                continue;
            }

            auto param_line = FormatNamedDeclaration(param, snapshot.get());

            result += param_line;
            params.push_back(std::move(param_line));

            if (i + 1 != node->params.size()) {
                result += ", ";
            }
        }

        result += ")";

        return {
            .return_typename = std::move(return_typename),
            .base_name       = std::string(raw_name),
            .full_spec       = std::move(result),
            .params          = std::move(params)
        };
    }

    FunctionFormatResult FormatStructConstructor(const SymbolInfo* symbol, Snapshot snapshot) {
        const auto fields = Utils::CollectStructFieldsOrdered(symbol);
        if (!fields.has_value()) {
            return {};
        }

        auto result = symbol->name + "(";
        std::vector<std::string> params;

        for (const auto& [i, field] : *fields | std::views::enumerate) {
            auto* field_node = static_cast<const VariableDeclarationNode*>(field->node);
            auto  field_line = FormatNamedDeclaration(field_node, snapshot.get());

            result += field_line;
            params.push_back(std::move(field_line));

            if (!std::cmp_equal(i + 1, fields->size())) {
                result += ", ";
            }
        }

        result += ")";

        return {
            .return_typename = symbol->name,
            .base_name       = symbol->name,
            .full_spec       = std::move(result),
            .params          = std::move(params)
        };
    }

    namespace {
        std::string SerializeInitializer(const auto* const expr) {
            if (expr == nullptr) {
                return {};
            }

            switch (expr->kind()) {
            case AstNodeKind::kRawExpression: {
                auto* node = static_cast<const RawExpressionNode*>(expr);
                std::string result;
                for (const auto& token : node->tokens) {
                    result += token.text;
                }

                return result;
            }

            case AstNodeKind::kVariableExpression: {
                auto* node = static_cast<const VariableExpressionNode*>(expr);
                return std::string(node->name);
            }

            case AstNodeKind::kCastExpression: {
                auto* cast = static_cast<const CastExpressionNode*>(expr);
                if (cast->operand == nullptr) {
                    return {};
                }

                const auto type_name = cast->target_type.typename_token().text;
                if (type_name.empty()) {
                    return {};
                }

                auto operand = SerializeInitializer(cast->operand);
                if (operand.empty()) {
                    return {};
                }

                // 防止 (int)(a + b) 被序列化成 (int)a + b
                switch (cast->operand->kind()) {
                case AstNodeKind::kBinaryExpression:
                case AstNodeKind::kTernaryExpression:
                    operand = "(" + operand + ")";
                    break;

                default:
                    break;
                }

                return "(" + std::string(type_name) + ")" + operand;
            }

            case AstNodeKind::kUnaryExpression: {
                auto* unary = static_cast<const UnaryExpressionNode*>(expr);

                std::string op_text = [](TokenType op) -> std::string {
                    switch (op) {
                    case TokenType::kMinus:       return "-";
                    case TokenType::kExclamation: return "!";
                    case TokenType::kTilde:       return "~";
                    case TokenType::kPlusPlus:    return "++";
                    case TokenType::kMinusMinus:  return "--";
                    default:                      return "";
                    }
                }(unary->op);

                if (unary->is_postfix) {
                    return SerializeInitializer(unary->operand) + op_text;
                }

                return op_text + SerializeInitializer(unary->operand);
            }

            case AstNodeKind::kBinaryExpression: {
                auto* binary = static_cast<const BinaryExpressionNode*>(expr);

                std::string op_text = [](TokenType op) -> std::string {
                    switch (op) {
                    case TokenType::kPlus:                   return " + ";
                    case TokenType::kMinus:                  return " - ";
                    case TokenType::kStar:                   return " * ";
                    case TokenType::kSlash:                  return " / ";
                    case TokenType::kPercent:                return " % ";
                    case TokenType::kEqualEqual:             return " == ";
                    case TokenType::kNotEqual:               return " != ";
                    case TokenType::kLessThan:               return " < ";
                    case TokenType::kGreaterThan:            return " > ";
                    case TokenType::kLessEqual:              return " <= ";
                    case TokenType::kGreaterEqual:           return " >= ";
                    case TokenType::kAmpersand:              return " & ";
                    case TokenType::kVerticalBar:            return " | ";
                    case TokenType::kCaret:                  return " ^ ";
                    case TokenType::kLeftShift:              return " << ";
                    case TokenType::kRightShift:             return " >> ";
                    case TokenType::kAmpersandAmpersand:     return " && ";
                    case TokenType::kVerticalBarVerticalBar: return " || ";
                    case TokenType::kCaretCaret:             return " ^^ ";
                    default:                                 return " ? ";
                    }
                }(binary->op);

                return SerializeInitializer(binary->left) + op_text + SerializeInitializer(binary->right);
            }

            case AstNodeKind::kCallExpression: {
                auto* call = static_cast<const CallExpressionNode*>(expr);
                std::string result = SerializeInitializer(call->callee) + "(";
                for (auto i = 0uz; i != call->args.size(); ++i) {
                    result += SerializeInitializer(call->args[i]);
                    if (i + 1 != call->args.size()) {
                        result += ", ";
                    }
                }

                result += ")";
                return result;
            }

            case AstNodeKind::kIndexExpression: {
                auto* index = static_cast<const IndexExpressionNode*>(expr);
                return SerializeInitializer(index->base) + "[" + SerializeInitializer(index->index) + "]";
            }

            case AstNodeKind::kMemberAccessExpression: {
                auto* member = static_cast<const MemberAccessExpressionNode*>(expr);
                return SerializeInitializer(member->object) + "." + SerializeInitializer(member->member);
            }

            case AstNodeKind::kTernaryExpression: {
                auto* ternary = static_cast<const TernaryExpressionNode*>(expr);
                return SerializeInitializer(ternary->condition) + " ? "
                     + SerializeInitializer(ternary->true_expr) + " : "
                     + SerializeInitializer(ternary->false_expr);
            }

            default:
                return {};
            }
        };

        bool NeedSpace(const Token& left, const Token& right) {
            switch (right.type) {
            case TokenType::kCloseParen:
            case TokenType::kCloseBracket:
            case TokenType::kCloseBrace:
            case TokenType::kComma:
            case TokenType::kSemicolon:
            case TokenType::kSharpSharp:
            case TokenType::kColon:
            case TokenType::kColonColon:
            case TokenType::kDot:
                return false;
            default:
                break;
            }

            switch (left.type) {
            case TokenType::kOpenParen:
            case TokenType::kOpenBracket:
            case TokenType::kSharpSharp:
            case TokenType::kSharp:
            case TokenType::kDot:
            case TokenType::kExclamation:
            case TokenType::kTilde:
                return false;
            default:
                break;
            }

            if (right.type == TokenType::kOpenParen) {
                return left.type == TokenType::kKeyword &&
                      (left.text == "if" ||
                       left.text == "while" ||
                       left.text == "for" ||
                       left.text == "switch" ||
                       left.text == "do" ||
                       left.text == "else");
            }

            if (right.type == TokenType::kOpenBracket) {
                return left.type == TokenType::kEqual;
            }

            if (right.type == TokenType::kOpenBrace) {
                return false;
            }

            return true;
        }

        std::string RenderTokenSequence(std::span<const Token> tokens) {
            bool suppress_next = false;
            const Token* previous = nullptr;

            std::string result;
            for (auto i = 0uz; i != tokens.size(); ++i) {
                const auto& token = tokens[i];
                if (token.type == TokenType::kBackslash) {
                    suppress_next = true;
                    result += (!result.empty() && result.back() != ' ') ? " \\\n" : "\\\n";

                    if (i + 1 < tokens.size()) {
                        auto indent = tokens[i + 1].location.column() - 1;
                        for (auto j = 0uz; j != indent; ++j) {
                            result += ' ';
                        }
                    }

                    continue;
                }

                if (previous != nullptr && !suppress_next && NeedSpace(*previous, token)) {
                    result += ' ';
                }

                result += token.text;
                previous = &token;
                suppress_next = false;
            }

            return result;
        }

        std::string RenderMacroExpansion(std::span<const Token> tokens, const Formatter& formatter, const std::filesystem::path& filename) {
            std::string snippet;
            const Token* previous = nullptr;

            for (const auto& token : tokens) {
                if (token.type == TokenType::kBackslash) {
                    continue;
                }

                if (previous != nullptr && NeedSpace(*previous, token)) {
                    snippet += ' ';
                }

                snippet += token.text;
                previous = &token;
            }

            if (snippet.empty()) {
                return {};
            }

            auto formatted = formatter.FormatSnippet(snippet, filename);
            return formatted.empty() ? snippet : formatted;
        }

        const ExpressionNode* FollowConstantChain(const ExpressionNode* expr) {
            while (expr != nullptr && expr->kind() == AstNodeKind::kVariableExpression) {
                auto* variable = static_cast<const VariableExpressionNode*>(expr);
                const SymbolInfo* symbol = nullptr;

                if (std::holds_alternative<const SymbolInfo*>(variable->linked_symbols)) {
                    symbol = std::get<const SymbolInfo*>(variable->linked_symbols);
                }

                if (symbol == nullptr                     ||
                    symbol->kind != SymbolKind::kVariable ||
                    !symbol->type_info.is_const()         ||
                    symbol->node->kind() != AstNodeKind::kVariableDeclaration)
                {
                    break;
                }

                auto* declare = static_cast<const VariableDeclarationNode*>(symbol->node);
                if (declare->init == nullptr) {
                    break;
                }

                expr = declare->init;
            }

            return expr;
        }
    }

    std::string BuildHoverMarkdown(
        const SymbolInfo* symbol,
        Snapshot snapshot,
        const SourceLocation& location,
        std::string_view current_uri,
        const Formatter& formatter)
    {
        if (symbol == nullptr) {
            return {};
        }

        auto BuildDefinedAt = [current_uri](const SymbolInfo* symbol) -> std::string {
            if (symbol->location.uri() == current_uri) {
                return std::format("Defined in this file, line {}", symbol->location.line());
            }

            auto filename = Utils::UriToPath(symbol->location.uri()).filename().generic_string();
            return std::format("Defined in {}, line {}", filename, symbol->location.line());
        };

        std::string title;
        std::string type_arrow;
        std::string details;
        std::string declare;

        auto AppendLayoutOnDetails = [&details](std::string_view layout) -> void {
            if (!layout.empty()) {
                details += std::format("**Layout**: `{}`\n", layout);
            }
        };

        auto AppendHitsCommit = [&details](std::span<const std::string> hits, std::string_view header) -> void {
            if (hits.empty()) {
                return;
            }

            details += std::format("\n**{}**: `", header);
            for (auto i = 0uz; i != hits.size(); ++i) {
                details += hits[i];
                if (i + 1 != hits.size()) {
                    details += " ";
                }
            }

            details += "`\n";
        };

        auto CollectAndAppendStorageHits = [&details, AppendHitsCommit](const auto* node) -> void {
            std::vector<std::string> storage_hits;
            auto& metadata = MetadataManager::GetInstance();

            for (const auto& spec : node->type_spec.specifiers) {
                if (auto subtype = metadata.GetLexicalSubtype(spec.text);
                    subtype.has_value() && subtype->contains("Qualifiers")) {
                    if (spec.text != "layout") {
                        storage_hits.push_back(std::string(spec.text));
                    }
                }
            }

            for (const auto& spirv : node->type_spec.spirv_intrinsics) {
                if (spirv != nullptr && spirv->keyword.text == "spirv_storage_class") {
                    storage_hits.push_back(RenderCanonicalSpirvCall(spirv));
                }
            }

            AppendHitsCommit(storage_hits, "Storage");
        };

        auto CollectAndAppendDecorateHits = [&details, AppendHitsCommit](const auto* node) -> void {
            std::vector<std::string> decorate_hits;

            for (const auto& spirv : node->type_spec.spirv_intrinsics) {
                if (spirv == nullptr) {
                    continue;
                }

                const auto& name = spirv->keyword.text;
                if (name == "spirv_decorate" || name == "spirv_decorate_id" || name == "spirv_decorate_string") {
                    decorate_hits.push_back(RenderCanonicalSpirvCall(spirv));
                }
            }

            AppendHitsCommit(decorate_hits, "Decorate");
        };

        auto BuildDefinedDeclare = [&declare](const SymbolInfo* symbol, std::string_view type_spec) mutable -> void {
            const auto* host_symbol = symbol->located_scope->closest_host();

            std::string host_name;
            if (host_symbol != nullptr) {
                if (host_symbol->name.contains("__AnonymousStruct_")) {
                    host_name = "<anonymous>";
                } else if (host_symbol->name.contains("__Impl_") || host_symbol->name.contains("__Decl_")) {
                    host_name = Utils::UnmangleFunctionName(host_symbol->name);
                } else {
                    host_name = host_symbol->name;
                }
            } else {
                host_name = "global scope";
            }

            std::string full_spec(type_spec);
            const auto last_close_paren = full_spec.find(')');
            const auto open_bracket = (last_close_paren != std::string::npos) ? full_spec.find('[', last_close_paren) : full_spec.find('[');
            if (open_bracket != std::string::npos) {
                full_spec.insert(open_bracket, " " + symbol->name);
            } else {
                full_spec += " " + symbol->name;
            }

            declare = std::format("// In {}\n{};", host_name, full_spec);

            if (!declare.contains("const") || symbol->kind != SymbolKind::kVariable) {
                return;
            }

            auto* node = static_cast<const VariableDeclarationNode*>(symbol->node);
            if (node == nullptr || node->init == nullptr) {
                return;
            }

            const auto initializer = SerializeInitializer(node->init);
            if (!initializer.empty()) {
                declare.insert(declare.find_last_of(';'), " = " + initializer);
                if (node->init->kind() != AstNodeKind::kRawExpression) {
                    ConstantEvaluator evaluator;
                    if (auto value = evaluator.EvaluateAs<std::string>(node->init)) {
                        declare += "\n// Evaluates to\n" + *value;
                    } else if (auto* root = FollowConstantChain(node->init); root != nullptr && root != node->init) {
                        auto root_text = SerializeInitializer(root);
                        if (!root_text.empty()) {
                            declare += "\n// Evaluates to\n" + root_text;
                        }
                    }
                }
            }
        };

        switch (symbol->kind) {
        case SymbolKind::kAttribute: {
            std::string markdown = std::format("**Attribute** `{}`\n\n---\n", symbol->name);
            // TODO: insert example
            // markdown += "Example:\n\n---\n";
            markdown += std::format("```glsl\n[[{}]]\n```", symbol->name);
            return markdown;
        }

        case SymbolKind::kMacro: {
            auto* node = static_cast<const PreprocessorNode*>(symbol->node);

            const auto cursor_index = FindCursorTokenIndex(snapshot->raw_tokens, location);
            auto markdown = std::format("**Macro** `{}`\n\n{}\n\n---\n\n```glsl\n", node->symbol->name, BuildDefinedAt(symbol));
            declare.clear();
            details.clear();

            declare = std::format("#define {}", node->symbol->name);
            if (node->is_function) {
                declare += "(";
                for (auto i = 0uz; i != node->params.size(); ++i) {
                    declare += node->params[i];
                    if (i + 1 != node->params.size()) {
                        declare += ", ";
                    }
                }

                declare += ")";
            }
            declare += " " + RenderTokenSequence(node->tokens);

            if (cursor_index.has_value()) {
                const auto& cursor_token = snapshot->raw_tokens.at(*cursor_index);
                const auto  filename     = Utils::UriToPath(current_uri);

                auto it = snapshot->macro_expansions.find(cursor_token.location);
                if (it != snapshot->macro_expansions.end()) {
                    const auto expanded = RenderMacroExpansion(it->second, formatter, filename);
                    if (!expanded.empty()) {
                        details = "\n\n// Expands to\n" + expanded + "\n```\n";
                    }
                } else if (location == symbol->location) {
                    const auto body = RenderMacroExpansion(node->tokens, formatter, filename);
                    if (!body.empty()) {
                        details = "\n\n// Expands to\n" + body + "\n```\n";
                    }
                }
            }

            markdown = markdown + declare + details;
            return markdown;
        }

        case SymbolKind::kVariable:
        case SymbolKind::kParameter: {
            auto* node = static_cast<const VariableDeclarationNode*>(symbol->node);

            std::string scope_prefix = "**Field**";
            if (symbol->kind == SymbolKind::kParameter) {
                scope_prefix = "**Parameter**";
            } else if (symbol->located_scope->kind() == ScopeKind::kMacroBody) {
                scope_prefix = "";
            } else if (symbol->located_scope->kind() == ScopeKind::kGlobalTransparent) {
                scope_prefix = "**Global Variable**";
            } else if (symbol->located_scope->kind() == ScopeKind::kCommon) {
                scope_prefix = "**Local Variable**";
            }

            if (!scope_prefix.contains("Field")) {
                title = std::format("{} `{}`\n\n{}", scope_prefix, symbol->name, BuildDefinedAt(symbol));
            } else {
                title = std::format("{} `{}::{}`\n\n{}", scope_prefix, symbol->located_scope->host_symbol()->name, symbol->name, BuildDefinedAt(symbol));
            }

            const auto type_text = BuildTypeText(node, snapshot.get());
            type_arrow = std::format("**Type** -> `{}`", type_text);

            const auto full = BuildHoverSpecifierLine(node, snapshot.get());
            BuildDefinedDeclare(symbol, full);

            AppendLayoutOnDetails(RenderMergedLayout(node->type_spec.layouts));
            CollectAndAppendStorageHits(node);
            CollectAndAppendDecorateHits(node);

            break;
        }

        case SymbolKind::kFunctionDecl:
        case SymbolKind::kFunctionImpl: {
            const auto format_result = FormatFunctionSymbol(symbol, snapshot);

            title      = std::format("**Function** `{}`", format_result.base_name);
            type_arrow = std::format("**Returns** -> `{}`", format_result.return_typename);

            std::string params_block = "**Parameters:**\n";
            for (const auto& param : format_result.params) {
                if (param == "...") {
                    continue;
                }

                params_block += std::format("- `{}`\n", param);
            }

            details += params_block + "\n";

            auto* node = static_cast<const FunctionDeclarationNode*>(symbol->node);
            std::vector<std::string> decorate_calls;
            for (const auto& spirv : node->type_spec.spirv_intrinsics) {
                if (spirv != nullptr && spirv->keyword.text == "spirv_instruction") {
                    decorate_calls.push_back(RenderCanonicalSpirvCall(spirv));
                }
            }

            if (!decorate_calls.empty()) {
                details += "**Decorate:**\n\n";
                for (const auto& call : decorate_calls) {
                    details += std::format("`{}`\n", call);
                }
            }

            declare += std::format("// {}\n", BuildDefinedAt(symbol));
            declare += format_result.full_spec;
            break;
        }

        case SymbolKind::kStruct: {
            title = std::format("**Struct** `{}`\n\n{}", symbol->name, BuildDefinedAt(symbol));
            type_arrow.clear();

            BuildDefinedDeclare(symbol, "struct");

            break;
        }

        case SymbolKind::kInterface: {
            auto* node = static_cast<const InterfaceDeclarationNode*>(symbol->node);
            title = std::format("**Block** `{}`\n\n{}", symbol->name, BuildDefinedAt(symbol));
            type_arrow.clear();

            const auto full = BuildHoverSpecifierLine(node, snapshot.get());
            BuildDefinedDeclare(symbol, full);

            AppendLayoutOnDetails(RenderMergedLayout(node->type_spec.layouts));
            CollectAndAppendStorageHits(node);
            CollectAndAppendDecorateHits(node);

            break;
        }

        default:
            break;
        }

        std::string markdown;
        markdown += title + "\n";
        if (!type_arrow.empty()) {
            markdown += "\n---\n";
            markdown += type_arrow + "\n";
        }

        markdown += "\n---\n";
        if (details.size() > 11) {
            markdown += details + "\n";
        }

        markdown += "\n---\n";
        markdown += "```glsl\n" + declare + "\n```";
        return markdown;
    }
}
