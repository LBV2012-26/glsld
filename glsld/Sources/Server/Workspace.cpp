#include "stdafx.h"
#include "Workspace.hpp"

#include <variant>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Passes/NodeLocator.hpp"
#include "Analyzer/Passes/OverloadResolver.hpp"
#include "Analyzer/Passes/SymbolLinker.hpp"
#include "Analyzer/Passes/TypeResolver.hpp"
#include "Analyzer/Syntax/Parser.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    void Workspace::UpdateDocument(std::string_view uri, std::string_view context) {
        auto document = std::make_unique<Document>();

        Parser parser(context, document->symbols);
        document->ast = parser.Parse();
        document->tokens = parser.tokens();

        SymbolLinker linker(document->symbols, document->bindings);
        linker.Traverse(document->ast.get());

        TypeResolver collector(document->symbols, document->bindings);
        collector.Traverse(document->ast.get());

        OverloadResolver resolver(document->symbols, document->bindings);
        resolver.Traverse(document->ast.get());

        documents_[std::string(uri)] = std::move(document);
    }

    void Workspace::RemoveDocument(std::string_view uri) {
        documents_.erase(uri);
    }

    std::vector<const SymbolInfo*> Workspace::GetDefinitionSymbols(std::string_view uri, SourceLocation location) const {
        auto it = documents_.find(uri);
        if (it == documents_.end()) {
            return {};
        }

        const auto& document = it->second;

        std::vector<const SymbolInfo*> results;
        NodeLocator locator(location);
        locator.Traverse(document->ast.get());
        const AstNode* node = locator.result();
        if (node == nullptr) {
            return {};
        }

        switch (node->kind()) {
        case AstNodeKind::kVariableExpression: {
            const auto* var_expr = static_cast<const VariableExpressionNode*>(node);

            if (std::holds_alternative<const SymbolInfo*>(var_expr->linked_symbols)) {
                const auto* linked_symbol = std::get<const SymbolInfo*>(var_expr->linked_symbols);
                if (linked_symbol != nullptr) {
                    results.push_back(linked_symbol);
                }
            } else if (std::holds_alternative<SymbolList>(var_expr->linked_symbols)) {
                const auto& linked_symbols = std::get<SymbolList>(var_expr->linked_symbols);
                if (!linked_symbols.empty()) {
                    results.append_range(linked_symbols);
                }
            }

            return results;
        }

        case AstNodeKind::kVariableDeclaration: {
            const auto* var_decl = static_cast<const VariableDeclarationNode*>(node);
            const auto& typename_token = var_decl->type_spec.typename_token();

            if (utils::IsPositionInToken(typename_token, location)) {
                if (typename_token.type != TokenType::kIdentifier) {
                    return results;
                }

                const auto& type_symbol = document->bindings.at(typename_token.location);
                if (std::holds_alternative<const SymbolInfo*>(type_symbol)) {
                    const auto* linked_symbol = std::get<const SymbolInfo*>(type_symbol);
                    if (linked_symbol != nullptr) {
                        results.push_back(linked_symbol);
                    }
                } else if (std::holds_alternative<SymbolList>(type_symbol)) {
                    const auto& linked_symbols = std::get<SymbolList>(type_symbol);
                    if (!linked_symbols.empty()) {
                        results.append_range(linked_symbols);
                    }
                }
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

                const auto& type_symbol = document->bindings.at(typename_token.location);
                if (std::holds_alternative<const SymbolInfo*>(type_symbol)) {
                    const auto* linked_symbol = std::get<const SymbolInfo*>(type_symbol);
                    if (linked_symbol != nullptr) {
                        results.push_back(linked_symbol);
                    }
                } else if (std::holds_alternative<SymbolList>(type_symbol)) {
                    const auto& linked_symbols = std::get<SymbolList>(type_symbol);
                    if (!linked_symbols.empty()) {
                        results.append_range(linked_symbols);
                    }
                }
            } else if (func_decl->declared_symbol != nullptr && utils::IsPositionInFunctionName(func_decl->declared_symbol, location)) {
                const auto* result = ResolveFunctionJump(func_decl->declared_symbol, uri);
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

    const Document* Workspace::GetDocument(std::string_view uri) const {
        auto it = documents_.find(uri);
        if (it != documents_.end()) {
            return it->second.get();
        }

        return nullptr;
    }

    const DocumentSymbols* Workspace::GetDocumentSymbols(std::string_view uri) const {
        auto it = documents_.find(uri);
        if (it != documents_.end()) {
            return &it->second->symbols;
        }

        return nullptr;
    }

    std::span<const Token> Workspace::GetDocumentTokens(std::string_view uri) const {
        auto it = documents_.find(uri);
        if (it != documents_.end()) {
            return it->second->tokens;
        }

        return {};
    }

    const SymbolInfo* Workspace::ResolveFunctionJump(const SymbolInfo* symbol, std::string_view uri) const {
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

        auto it = documents_.find(uri);
        if (it != documents_.end()) {
            const auto* resolved_symbol = it->second->symbols.root_scope()->FindSymbol(name);
            return resolved_symbol;
        }

        return nullptr;
    }

    std::string Workspace::FormatSymbol(const SymbolInfo* symbol) const {
        if (symbol == nullptr) {
            return "";
        }

        std::string result;
        switch (symbol->kind) {
        case SymbolKind::kParameter:
            result = std::format("(parameter) {} {}", symbol->type_info.typename_token.text, symbol->name);
            break;
        case SymbolKind::kVariable: {
            std::string prefix;

            if (symbol->located_scope->kind() == ScopeKind::kTransparent) {
                prefix = "(global variable)";
            } else if (symbol->located_scope->kind() == ScopeKind::kCommon) {
                prefix = "(local variable)";
            } else {
                prefix = "(field)";
            }

            result = std::format("{} {} {}", prefix, symbol->type_info.typename_token.text, symbol->name);
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
        }
        }
    }
}
