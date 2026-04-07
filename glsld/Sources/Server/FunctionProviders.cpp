#include "stdafx.h"
#include "FunctionProviders.hpp"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <string_view>
#include <unordered_set>
#include <variant>

#include "Analyzer/Passes/InlayHintVisitor.hpp"
#include "Analyzer/Passes/NodeLocator.hpp"
#include "Base/Hash.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    namespace {
        const SymbolInfo* FindFunctionCounterpart(const SymbolList& symbol_list, const SymbolInfo* symbol) {
            if (symbol == nullptr) {
                return nullptr;
            }

            auto target_kind = symbol->kind == SymbolKind::kFunctionImpl ? SymbolKind::kFunctionDecl : SymbolKind::kFunctionImpl;

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
    }

    nlohmann::json ConvertScopeToDocumentSymbols(const Scope* const scope) {
        nlohmann::json symbols = nlohmann::json::array();

        if (scope == nullptr) {
            return symbols;
        }

        auto ConvertToLspRange = [](const auto& begin, const auto& end) -> nlohmann::json {
            return {
                { "start", { { "line", begin.line - 1 }, { "character", begin.column - 1 } } },
                { "end",   { { "line", end.line   - 1 }, { "character", end.column   - 1 } } }
            };
        };

        auto ConvertToSelectionRange = [](const auto& location, std::string_view name) -> nlohmann::json {
            return {
                { "start", { { "line", location.line - 1 }, { "character", location.column                 - 1 } } },
                { "end",   { { "line", location.line - 1 }, { "character", location.column + name.length() - 1 } } }
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

        std::unordered_set<const Scope*> handled_scopes;

        for (const auto& [name, info] : scope->symbols()) {
            nlohmann::json symbol_node;

            int symbol_kind = ConvertSymbolKind(info.kind);
            if (symbol_kind == 13) {
                if (info.located_scope != nullptr &&
                    (info.located_scope->kind() == ScopeKind::kBlock || info.located_scope->kind() == ScopeKind::kBlockTransparent))
                {
                    symbol_kind = 8;
                }
            }

            symbol_node["name"] = SplitSymbolName(info.name);
            symbol_node["kind"] = symbol_kind;
            symbol_node["selectionRange"] = ConvertToSelectionRange(info.location, info.name);

            const Scope* child_scope = nullptr;
            if (info.kind == SymbolKind::kInterface || info.kind == SymbolKind::kStruct) {
                for (const auto& child : scope->children()) {
                    if (child->interval().first.line == info.location.line) {
                        child_scope = child.get();
                        handled_scopes.insert(child_scope);
                        break;
                    }
                }

                if (info.kind == SymbolKind::kFunctionDecl) {
                    symbol_node["detail"] = "(decl)";
                }
            }

            if (child_scope != nullptr) {
                symbol_node["range"] = ConvertToLspRange(info.location, child_scope->interval().second);
                if (info.kind == SymbolKind::kInterface || info.kind == SymbolKind::kStruct) {
                    auto children = ConvertScopeToDocumentSymbols(child_scope);
                    if (!children.empty()) {
                        symbol_node["children"] = children;
                    }
                }
            } else {
                symbol_node["range"] = symbol_node["selectionRange"];
            }

            symbols.push_back(symbol_node);
        }

        // Transparent scope
        for (const auto& child_scope : scope->children()) {
            if ((child_scope->kind() == ScopeKind::kGlobalTransparent ||
                 child_scope->kind() == ScopeKind::kBlockTransparent) &&
                !handled_scopes.contains(child_scope.get()))
            {
                auto transparent_children = ConvertScopeToDocumentSymbols(child_scope.get());
                for (const auto& child : transparent_children) {
                    symbols.push_back(child);
                }
            }
        }

        return symbols;
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
            case TokenType::kPrimitive:       type_index = 23; break;
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

    std::vector<std::uint32_t> GetSemanticData(std::shared_ptr<const Document> snapshot) {
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

            std::size_t line       = token.location.line   - 1;
            std::size_t character  = token.location.column - 1;
            std::size_t length     = token.text.length();
            std::size_t delta_line = line - last_line;
            std::size_t delta_char = (delta_line == 0) ? (character - last_char) : character;

            data.push_back(static_cast<std::uint32_t>(delta_line));
            data.push_back(static_cast<std::uint32_t>(delta_char));
            data.push_back(static_cast<std::uint32_t>(length));
            data.push_back(static_cast<std::uint32_t>(type_index));
            data.push_back(modifiers);

            last_line = static_cast<std::uint32_t>(line);
            last_char = static_cast<std::uint32_t>(character);
        };

        auto IsInactive = [&](std::size_t line) -> bool {
            if (snapshot->inactive_regions.empty()) {
                return false;
            }

            const auto& regions = snapshot->inactive_regions;
            auto it = std::ranges::upper_bound(regions, line, std::ranges::less{}, [](InactiveRegion region) -> std::size_t {
                return region.begin_line;
            });

            if (it == regions.begin()) {
                return false;
            }

            --it;
            return it->begin_line <= line && line <= it->end_line;
        };

        for (const auto& token : snapshot->raw_tokens) {
            std::uint32_t modifiers = 0;
            if (IsInactive(token.location.line)) {
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
            if (std::holds_alternative<SymbolList>(it->second)) {
                const auto& symbols = std::get<SymbolList>(it->second);
                if (!symbols.empty()) {
                    symbol = symbols.front();
                }
            } else if (std::holds_alternative<const SymbolInfo*>(it->second)) {
                symbol = std::get<const SymbolInfo*>(it->second);
            }

            if (symbol == nullptr) {
                type_index = GetTokenSemanticHighlight(token.type);
                EmitSemanticData(type_index, modifiers, token);
                continue;
            }

            type_index = GetSymbolSemanticHighlight(symbol->kind);

            if (token.location.line   == symbol->location.line &&
                token.location.column == symbol->location.column)
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
        const SymbolInfo* ResolveFunctionJump(const Document* snapshot, const SymbolInfo* symbol) {
            if (symbol == nullptr) {
                return nullptr;
            }

            auto base_name = utils::UnmangleFunctionName(symbol->name);
            const auto& candidates = snapshot->symbols.FindFunctionsByOriginalName(base_name);

            return FindFunctionCounterpart(candidates, symbol);
        };
    }

    SymbolList GetDefinitionSymbols(std::shared_ptr<const Document> snapshot, SourceLocation location, bool toggle_function) {
        if (snapshot == nullptr) {
            return {};
        }

        SymbolList results;
        const Token* cursor_token = nullptr;

        auto it = std::ranges::upper_bound(snapshot->raw_tokens, location, std::ranges::less{}, &Token::location);
        if (it != snapshot->raw_tokens.begin()) {
            cursor_token = &*std::prev(it);
            if (!utils::IsPositionInToken(*cursor_token, location)) {
                cursor_token = nullptr;
            }
        }

        if (cursor_token != nullptr) {
            auto it = snapshot->bindings.find(cursor_token->location);
            if (it != snapshot->bindings.end() && std::holds_alternative<const SymbolInfo*>(it->second)) {
                const auto* linked_symbol = std::get<const SymbolInfo*>(it->second);
                if (linked_symbol != nullptr) {
                    if (linked_symbol->kind == SymbolKind::kFunctionDecl ||
                        linked_symbol->kind == SymbolKind::kFunctionImpl)
                    {
                        if (!toggle_function) {
                            results.push_back(linked_symbol);
                            return results;
                        }

                        bool clicked_on_defination = (cursor_token->location == linked_symbol->location);
                        if (clicked_on_defination) {
                            const auto* toggled = ResolveFunctionJump(snapshot.get(), linked_symbol);
                            results.push_back(toggled ? toggled : linked_symbol);
                        } else {
                            if (linked_symbol->kind == SymbolKind::kFunctionDecl) {
                                const auto* impl = ResolveFunctionJump(snapshot.get(), linked_symbol);
                                results.push_back(impl ? impl : linked_symbol);
                            } else {
                                results.push_back(linked_symbol);
                            }
                        }
                    } else {
                        results.push_back(linked_symbol);
                    }
                }
            }
        }

        if (!results.empty()) {
            return results;
        }

        LeafLocator locator(location);
        locator.Traverse(snapshot->ast.get());
        const auto* node = locator.result();

        if (node == nullptr || node->kind() != AstNodeKind::kVariableExpression) {
            return {};
        }

        const auto& linked_symbols = static_cast<const VariableExpressionNode*>(node)->linked_symbols;
        if (std::holds_alternative<const SymbolInfo*>(linked_symbols)) {
            const auto* symbol = std::get<const SymbolInfo*>(linked_symbols);
            if (symbol != nullptr) {
                results.push_back(symbol);
            }
        } else if (std::holds_alternative<SymbolList>(linked_symbols)) {
            const auto& symbols = std::get<SymbolList>(linked_symbols);
            if (!symbols.empty()) {
                results.append_range(symbols);
            }
        }

        return results;
    }

    std::vector<InlayHint> GetInlayHints(std::shared_ptr<const Document> snapshot) {
        if (snapshot == nullptr || snapshot->ast == nullptr) {
            return {};
        }

        InlayHintVisitor visitor;
        visitor.Traverse(snapshot->ast.get());
        return visitor.hints();
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
    }

    std::optional<SignatureHelpResult> GetSignatureHelp(std::shared_ptr<const Document> snapshot, SourceLocation location) {
        if (snapshot == nullptr) {
            return std::nullopt;
        }

        SignatureLocator locator(location);
        locator.Traverse(snapshot->ast.get());
        const auto* node = locator.result();

        if (node == nullptr) {
            return std::nullopt;
        }

        const auto* callee = static_cast<const VariableExpressionNode*>(node->callee.get());
        auto candidates = snapshot->symbols.FindFunctionsByOriginalName(callee->name);
        if (candidates.empty()) {
            return std::nullopt;
        }

        auto open_paren_loc = callee->end;
        auto it = std::ranges::lower_bound(snapshot->raw_tokens, open_paren_loc, std::ranges::less{}, &Token::location);
        int active_param_index = 0;
        int paren_level        = 0;
        while (it != snapshot->raw_tokens.end() && it->location < location) {
            if (it->type == TokenType::kOpenParen) {
                ++paren_level;
            } else if (it->type == TokenType::kCloseParen) {
                --paren_level;
            } else if (paren_level == 1 && it->type == TokenType::kComma) {
                ++active_param_index;
            }

            ++it;
        }

        std::vector<TypeInfo> current_arg_types;
        for (auto i = 0uz; i <= active_param_index && i < node->args.size(); ++i) {
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

        auto unique_candidates = DeduplicateSignatures(candidates);
        int active_signature_index = TypeResolver::RankSignatureCandidates(unique_candidates, current_arg_types);

        return SignatureHelpResult{
            .candidates             = std::move(unique_candidates),
            .active_signature_index = active_signature_index,
            .active_param_index     = active_param_index
        };
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
    }

    nlohmann::json GetCompletionItems(std::shared_ptr<const Document> snapshot, SourceLocation location) {
        if (snapshot == nullptr) {
            return {};
        }

        const auto* located_scope = snapshot->symbols.FindScopeAt(location);

        std::vector<const SymbolInfo*> visible_symbols;
        located_scope->GetVisibleSymbols(visible_symbols);

        nlohmann::json items = nlohmann::json::array();
        std::unordered_set<std::string, StringViewHeteroHash, StringViewHeteroEqual> existing_labels;

        for (const auto* symbol : visible_symbols) {
            nlohmann::json item;
            std::string_view symbol_name;

            if (symbol->kind == SymbolKind::kFunctionDecl || symbol->kind == SymbolKind::kFunctionImpl) {
                symbol_name = utils::UnmangleFunctionName(symbol->name);
            } else {
                symbol_name = symbol->name;
            }

            if (existing_labels.contains(symbol_name)) {
                continue;
            }

            existing_labels.emplace(symbol_name);
            item["label"] = symbol_name;
            item["kind"]  = MapSymbolKindToLspCompletion(symbol->kind, symbol->type_info.is_const());

            // TODO: document, detail, etc
            items.push_back(item);
        }

        return items;
    }

    nlohmann::json GetFieldCompletionItems(std::shared_ptr<const Document> snapshot, SourceLocation location) {
        if (snapshot == nullptr) {
            return {};
        }

        SourceLocation dot_location;
        auto it = std::ranges::upper_bound(snapshot->expanded_tokens, location, std::ranges::less{}, &Token::location);
        if (it != snapshot->expanded_tokens.begin()) {
            dot_location = std::prev(it)->location;
        }

        ContextLocator locator(dot_location);
        locator.Traverse(snapshot->ast.get());
        const auto* node = locator.result();

        nlohmann::json items = nlohmann::json::array();

        if (const auto* expr_node = dynamic_cast<const ExpressionNode*>(node)) {
            const auto& type_info = expr_node->evaluated_type;

            if (type_info.block_symbol != nullptr && type_info.block_symbol->internal_scope != nullptr) {
                const auto* scope = type_info.block_symbol->internal_scope;
                for (const auto& symbol : scope->symbols()) {
                    nlohmann::json item;
                    item["label"] = symbol.first;
                    item["kind"]  = 5;

                    items.push_back(item);
                }
            }
        }

        return items;
    }

    namespace {
        std::string GetVariableSpecifiers(const auto* node) {
            std::string specifiers;

            for (auto i = 0uz; i < node->type_spec.specifiers.size(); ++i) {
                const auto& specifier = node->type_spec.specifiers[i];
                if (specifier.text == "layout") {
                    specifiers += specifier.text + "(";
                    for (auto j = 0uz; j != node->type_spec.layout_params.size(); ++j) {
                        const auto& layout_param = node->type_spec.layout_params[j];

                        if (layout_param.text == "," && specifiers.back() == ' ') {
                            specifiers.pop_back();
                        }

                        specifiers += layout_param.text;

                        if (j + 1 != node->type_spec.layout_params.size()) {
                            specifiers += " ";
                        }
                    }

                    specifiers += ")";
                } else {
                    std::string specifier_text = specifier.text.contains("__AnonymousStruct_")
                                               ? "<anonymous>" : specifier.text;
                    if (specifiers.empty()) {
                        specifiers = specifier_text;
                    } else {
                        specifiers += " " + specifier_text;
                    }
                }
            }

            if (specifiers.contains("layout")) {
                if (node->type_spec.specifiers.back().type == TokenType::kIdentifier) {
                    specifiers += " { ... }";
                }

                if (!specifiers.contains("set")) {
                    auto binding_pos = specifiers.find("binding");
                    if (binding_pos != std::string::npos) {
                        specifiers.insert(binding_pos, "set = 0, ");
                    }
                }
            }

            return specifiers;
        };
    }

    std::string FormatSymbol(const SymbolInfo* symbol) {
        if (symbol == nullptr) {
            return "";
        }

        std::string result;
        switch (symbol->kind) {
        case SymbolKind::kAttribute: {
            result = std::format("(attribute) {}", symbol->name);
            break;
        }

        case SymbolKind::kParameter: {
            const auto* node = static_cast<const VariableDeclarationNode*>(symbol->node);
            result = std::format("(parameter) {} {}", GetVariableSpecifiers(node), symbol->name);

            for (auto array_size : symbol->type_info.array_sizes) {
                std::format_to(std::back_inserter(result), "[{}]", array_size);
            }

            break;
        }

        case SymbolKind::kVariable: {
            std::string prefix;
            bool is_field = false;

            if (symbol->located_scope->kind() == ScopeKind::kGlobalTransparent) {
                prefix = "(global variable)";
            } else if (symbol->located_scope->kind() == ScopeKind::kCommon) {
                prefix = "(local variable)";
            } else {
                prefix = "(field)";
                is_field = true;
            }

            const auto* node = static_cast<const VariableDeclarationNode*>(symbol->node);
            auto specifiers = GetVariableSpecifiers(node);
            std::string name;
            if (is_field && node->located_scope->host_symbol() != nullptr) {
                name = std::format("{} {}::{}", specifiers, node->located_scope->host_symbol()->name, symbol->name);
            } else {
                name = std::format("{} {}", specifiers, symbol->name);
            }

            result = std::format("{} {}", prefix, name);

            for (auto array_size : symbol->type_info.array_sizes) {
                std::format_to(std::back_inserter(result), "[{}]", array_size);
            }

            break;
        }

        case SymbolKind::kFunctionDecl:
        case SymbolKind::kFunctionImpl: {
            std::string return_typename = symbol->type_info.typename_token.text;
            for (auto array_size : symbol->type_info.array_sizes) {
                std::format_to(std::back_inserter(return_typename), "[{}]", array_size);
            }

            auto raw_name = utils::UnmangleFunctionName(symbol->name);
            result = std::format("{} {}(", return_typename, raw_name);

            const auto* node = static_cast<const FunctionDeclarationNode*>(symbol->node);
            for (auto i = 0uz; i != node->params.size(); ++i) {
                const auto& param = node->params[i];
                result += GetVariableSpecifiers(param.get());

                const auto* param_symbol = param->declared_symbol;
                if (param_symbol != nullptr && param_symbol->name != "") {
                    result += " " + param_symbol->name;

                    for (auto array_size : param_symbol->type_info.array_sizes) {
                        std::format_to(std::back_inserter(result), "[{}]", array_size);
                    }
                }

                if (i + 1 != node->params.size()) {
                    result += ", ";
                }
            }

            result += ")";
            break;
        }

        case SymbolKind::kInterface: {
            const auto* node = static_cast<const InterfaceDeclarationNode*>(symbol->node);
            result = std::format("{} {}", GetVariableSpecifiers(node), symbol->name);
            break;
        }

        case SymbolKind::kStruct: {
            const auto* node = static_cast<const StructDeclarationNode*>(symbol->node);
            result = std::format("struct {}", symbol->name);
            break;
        }

        case SymbolKind::kMacro: {
            const auto* node = static_cast<const PreprocessorNode*>(symbol->node);
            result = std::format("#define {}", node->symbol->name);

            if (node->params.size() != 0) {
                result += "(";
                for (auto i = 0uz; i != node->params.size(); ++i) {
                    const auto& param = node->params[i];
                    result += param;
                    if (i + 1 != node->params.size()) {
                        result += ", ";
                    }
                }

                result += ")";
            }

            for (const auto& token : node->tokens) {
                if (token.type == TokenType::kBackslash) {
                    continue;
                }

                result += " " + token.text;
            }

            break;
        }

        default:
            break;
        }

        return result;
    }
}
