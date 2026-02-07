#include "stdafx.h"
#include "Workspace.hpp"

#include <variant>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Ast/NodeLocator.hpp"
#include "Analyzer/Ast/SymbolLinker.hpp"
#include "Analyzer/Ast/TypeResolver.hpp"
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

        TypeResolver resolver(document->symbols, document->bindings);
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

        if (node->kind() == AstNodeKind::kVariableExpression) {
            const auto* var_expr = static_cast<const VariableExpressionNode*>(node);

            if (std::holds_alternative<const SymbolInfo*>(var_expr->linked_symbols)) {
                const auto* linked_symbol = std::get<const SymbolInfo*>(var_expr->linked_symbols);
                if (linked_symbol != nullptr) {
                    results.push_back(linked_symbol);
                }
            } else if (std::holds_alternative<std::vector<const SymbolInfo*>>(var_expr->linked_symbols)) {
                const auto& linked_symbols = std::get<std::vector<const SymbolInfo*>>(var_expr->linked_symbols);
                if (!linked_symbols.empty()) {
                    results.append_range(linked_symbols);
                }
            }

            return results;
        }

        if (node->kind() == AstNodeKind::kVariableDeclaration) {
            const auto* var_decl = static_cast<const VariableDeclarationNode*>(node);

            const auto& typename_token = var_decl->type_spec.typename_token();
            if (typename_token.type != TokenType::kIdentifier) {
                return results;
            }

            if (utils::IsPositionInToken(typename_token, location)) {
                const auto& type_symbol = document->bindings.at(typename_token.location);
                if (std::holds_alternative<const SymbolInfo*>(type_symbol)) {
                    const auto* linked_symbol = std::get<const SymbolInfo*>(type_symbol);
                    if (linked_symbol != nullptr) {
                        results.push_back(linked_symbol);
                    }
                } else if (std::holds_alternative<std::vector<const SymbolInfo*>>(type_symbol)) {
                    const auto& linked_symbols = std::get<std::vector<const SymbolInfo*>>(type_symbol);
                    if (!linked_symbols.empty()) {
                        results.append_range(linked_symbols);
                    }
                }
            }

            return results;
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
}
