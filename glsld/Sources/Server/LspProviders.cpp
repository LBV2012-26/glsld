#include "stdafx.h"
#include "LspProviders.hpp"

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
        int GetSymbolSemanticHighlight(SymbolKind kind) {
            int type_index = -1;

            switch (kind) {
            case SymbolKind::kInterface:
                type_index = 4;
                break;
            case SymbolKind::kStruct:
                type_index = 5;
                break;
            case SymbolKind::kParameter:
                type_index = 7;
                break;
            case SymbolKind::kVariable:
                type_index = 8;
                break;
            case SymbolKind::kFunctionDecl:
            case SymbolKind::kFunctionImpl:
                type_index = 12;
                break;
            case SymbolKind::kMacro:
                type_index = 14;
                break;
            case SymbolKind::kPreprocessor:
                type_index = 15;
                break;
            default:
                break;
            }

            return type_index;
        }

        int GetTokenSemanticHighlight(TokenType type) {
            int type_index = -1;

            switch (type) {
            case TokenType::kPrimitive:
                type_index = 23;
                break;
            case TokenType::kBuiltInType:
                type_index = 1;
                break;
            case TokenType::kBuiltInFunction:
                type_index = 12;
                break;
            case TokenType::kKeyword:
            case TokenType::kPreprocessor:
            case TokenType::kSharp:
                type_index = 15;
                break;
            case TokenType::kNumberLiteral:
                type_index = 19;
                break;
            default:
                break;
            }

            return type_index;
        }

        const SymbolInfo* ResolveFunctionJump(const Document* snapshot, const SymbolInfo* symbol) {
            std::string name = symbol->name;
            if (symbol->kind == SymbolKind::kFunctionImpl) {
                if (auto pos = name.find("__Impl_"); pos != std::string::npos) {
                    name.replace(pos, 7, "__Decl_");
                }
            } else if (symbol->kind == SymbolKind::kFunctionDecl) {
                if (auto pos = name.find("__Decl_"); pos != std::string::npos) {
                    name.replace(pos, 7, "__Impl_");
                }
            }

            if (snapshot != nullptr) {
                const auto* resolved_symbol = snapshot->symbols.root_scope()->FindSymbol(name);
                return resolved_symbol;
            }

            return nullptr;
        }

        int MapSymbolKindToLspCompletion(SymbolKind kind, bool is_const = false) {
            switch (kind) {
            case SymbolKind::kVariable:
                return is_const ? 21 : 6; // 常量用 Constant(21)，普通变量用 Variable(6)
            case SymbolKind::kParameter:
                return 6;  // Variable
            case SymbolKind::kStruct:
                return 22; // Struct
            case SymbolKind::kInterface:
                return 8;  // Interface
            case SymbolKind::kFunctionDecl:
            case SymbolKind::kFunctionImpl:
                return 3;  // Function
            case SymbolKind::kMacro:
                return 9;  // Module (或者 Constant)
            default:
                return 1;  // Text
            }
        }
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
                { "start", { { "line", location.line - 1 }, { "character", location.column - 1 } } },
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

            symbol_node["name"] = SplitSymbolName(info.name);
            symbol_node["kind"] = ConvertSymbolKind(info.kind);
            symbol_node["selectionRange"] = ConvertToSelectionRange(info.location, info.name);

            const Scope* child_scope = nullptr;
            if (info.kind == SymbolKind::kInterface || info.kind == SymbolKind::kStruct) {
                for (const auto& child : scope->children()) {
                    if (child->interval().first.line == info.location.line) {
                        child_scope = child.get();
                        handled_scopes.emplace(child_scope);
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
            if (child_scope->kind() == ScopeKind::kTransparent && !handled_scopes.contains(child_scope.get())) {
                auto transparent_children = ConvertScopeToDocumentSymbols(child_scope.get());
                for (const auto& child : transparent_children) {
                    symbols.push_back(child);
                }
            }
        }

        return symbols;
    }

    std::vector<std::uint32_t> GetSemanticData(std::shared_ptr<const Document> snapshot) {
        if (snapshot == nullptr) {
            return {};
        }

        std::vector<std::uint32_t> data;
        std::uint32_t last_line = 0;
        std::uint32_t last_char = 0;

        for (const auto& token : snapshot->tokens) {
            std::uint32_t modifiers = 0;
            int type_index = -1;

            if (token.type == TokenType::kIdentifier) {
                auto it = snapshot->bindings.find(token.location);
                if (it != snapshot->bindings.end()) {
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
                        continue;
                    }

                    type_index = GetSymbolSemanticHighlight(symbol->kind);

                    if (token.location.line   == symbol->location.line &&
                        token.location.column == symbol->location.column)
                    {
                        modifiers |= (1 << 0); // declaration
                    }

                    if (token.type == TokenType::kIdentifier &&
                        symbol->located_scope->kind() == ScopeKind::kTransparent)
                    {
                        modifiers |= (1 << 3); // static
                    }
                }
            } else {
                type_index = GetTokenSemanticHighlight(token.type);
            }

            if (type_index != -1) {
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
            }
        }

        return data;
    }

    SymbolList GetDefinitionSymbols(std::shared_ptr<const Document> snapshot, SourceLocation location) {
        if (snapshot == nullptr) {
            return {};
        }

        NodeLocator locator(location);
        locator.Traverse(snapshot->ast.get());
        const AstNode* node = locator.result();
        if (node == nullptr) {
            return {};
        }

        SymbolList results;

        auto ExtractSymbols = [&results](const auto& symbols) mutable -> void {
            if (std::holds_alternative<const SymbolInfo*>(symbols)) {
                const auto* linked_symbol = std::get<const SymbolInfo*>(symbols);
                if (linked_symbol != nullptr) {
                    results.push_back(linked_symbol);
                }
            } else if (std::holds_alternative<SymbolList>(symbols)) {
                const auto& linked_symbols = std::get<SymbolList>(symbols);
                if (!linked_symbols.empty()) {
                    results.append_range(linked_symbols);
                }
            }
        };

        switch (node->kind()) {
        case AstNodeKind::kVariableExpression: {
            const auto* var_expr = static_cast<const VariableExpressionNode*>(node);
            ExtractSymbols(var_expr->linked_symbols);
            return results;
        }

        case AstNodeKind::kVariableDeclaration: {
            const auto* var_decl = static_cast<const VariableDeclarationNode*>(node);
            const auto& typename_token = var_decl->type_spec.typename_token();

            if (utils::IsPositionInToken(typename_token, location)) {
                if (typename_token.type != TokenType::kIdentifier) {
                    return results;
                }

                auto it = snapshot->bindings.find(typename_token.location);
                if (it == snapshot->bindings.end()) {
                    return results;
                }

                const auto& type_symbol = it->second;
                ExtractSymbols(type_symbol);
            }

            return results;
        }

        case AstNodeKind::kFunctionDeclaration: {
            const auto* func_decl = static_cast<const FunctionDeclarationNode*>(node);
            const auto& typename_token = func_decl->type_spec.typename_token();

            if (utils::IsPositionInToken(typename_token, location)) {
                if (typename_token.type != TokenType::kIdentifier) {
                    return results;
                }

                auto it = snapshot->bindings.find(typename_token.location);
                if (it == snapshot->bindings.end()) {
                    return results;
                }

                const auto& type_symbol = it->second;
                ExtractSymbols(type_symbol);
            } else if (func_decl->declared_symbol != nullptr && utils::IsPositionInFunctionName(func_decl->declared_symbol, location)) {
                const auto* result = ResolveFunctionJump(snapshot.get(), func_decl->declared_symbol);
                if (result != nullptr) {
                    results.push_back(result);
                }
            }

            return results;
        }

        default:
            break;
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

    nlohmann::json GetCompletionItems(std::shared_ptr<const Document> snapshot, SourceLocation location) {
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
        SourceLocation dot_location;
        auto it = std::ranges::upper_bound(snapshot->tokens, location, std::ranges::less{}, &Token::location);
        if (it != snapshot->tokens.begin()) {
            dot_location = std::prev(it)->location;
        }

        NodeLocator locator(dot_location);
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

    std::string FormatSymbol(const SymbolInfo* symbol) {
        if (symbol == nullptr) {
            return "";
        }

        auto GetVariableSpecifiers = [](const auto* node) -> std::string {
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
                    if (specifiers.empty()) {
                        specifiers = specifier.text;
                    } else {
                        specifiers += " " + specifier.text;
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

        std::string result;
        switch (symbol->kind) {
        case SymbolKind::kParameter: {
            const auto* node = static_cast<const VariableDeclarationNode*>(symbol->node);
            result = std::format("(parameter) {} {}", GetVariableSpecifiers(node), symbol->name);
            break;
        }

        case SymbolKind::kVariable: {
            std::string prefix;

            if (symbol->located_scope->kind() == ScopeKind::kTransparent) {
                prefix = "(global variable)";
            } else if (symbol->located_scope->kind() == ScopeKind::kCommon) {
                prefix = "(local variable)";
            } else {
                prefix = "(field)";
            }

            const auto* node = static_cast<const VariableDeclarationNode*>(symbol->node);
            result = std::format("{} {} {}", prefix, GetVariableSpecifiers(node), symbol->name);

            for (const auto& array_size : symbol->type_info.array_sizes) {
                result += std::format("[{}]", array_size.text);
            }

            break;
        }

        case SymbolKind::kFunctionDecl:
        case SymbolKind::kFunctionImpl: {
            std::string return_typename = symbol->type_info.typename_token.text;
            for (const auto& array_size : symbol->type_info.array_sizes) {
                return_typename += std::format("[{}]", array_size.text);
            }

            auto raw_name = utils::UnmangleFunctionName(symbol->name);
            result = std::format("{} {}(", return_typename, raw_name);

            const auto* node = static_cast<const FunctionDeclarationNode*>(symbol->node);
            for (auto i = 0uz; i != node->params.size(); ++i) {
                const auto& param = node->params[i];
                result += GetVariableSpecifiers(param.get());
                if (param->declared_symbol != nullptr && param->declared_symbol->name != "") {
                    result += " " + param->declared_symbol->name;
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
