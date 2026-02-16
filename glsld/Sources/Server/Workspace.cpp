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

    SymbolList Workspace::GetDefinitionSymbols(std::string_view uri, SourceLocation location) const {
        const auto* document = GetDocument(uri);
        if (document == nullptr) {
            return {};
        }

        SymbolList results;
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

    std::vector<InlayHint> Workspace::GetInlayHints(std::string_view uri) const {
        const auto* document = GetDocument(uri);
        if (document == nullptr || document->ast == nullptr) {
            return {};
        }

        InlayHintVisitor visitor;
        visitor.Traverse(document->ast.get());
        return visitor.hints();
    }

    const Document* Workspace::GetDocument(std::string_view uri) const {
        auto it = documents_.find(uri);
        if (it != documents_.end()) {
            return it->second.get();
        }

        return nullptr;
    }

    const DocumentSymbols* Workspace::GetDocumentSymbols(std::string_view uri) const {
        const auto* document = GetDocument(uri);
        if (document == nullptr) {
            return nullptr;
        }

        return &document->symbols;
    }

    std::span<const Token> Workspace::GetDocumentTokens(std::string_view uri) const {
        const auto* document = GetDocument(uri);
        if (document == nullptr) {
            return {};
        }

        return document->tokens;
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
}
