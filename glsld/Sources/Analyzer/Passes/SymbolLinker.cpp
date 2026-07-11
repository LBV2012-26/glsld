#include "stdafx.h"
#include "SymbolLinker.hpp"

#include <utility>
#include <variant>

namespace glsld {
    SymbolLinker::SymbolLinker(Document& document, int version_replica, std::shared_ptr<const std::atomic<int>> vesion_pointer)
        : AstVisitor(version_replica, vesion_pointer)
        , document_{ document }
    {
        Traverse(document_.ast.get());
    }

    void SymbolLinker::VisitPreprocessor(PreprocessorNode* node) {
        if (node->directive == "define" && node->symbol != nullptr) {
            document_.bindings.try_emplace(node->symbol->location, node->symbol);
        }

        for (auto& statement : node->body) {
            Traverse(statement.get());
        }
    }

    void SymbolLinker::VisitVariableExpression(VariableExpressionNode* node) {
        const Scope* scope = nullptr;
        if (node->internal_scope != nullptr) {
            scope = node->internal_scope;
        } else {
            scope = node->located_scope;
        }

        if (node->original_token.type != TokenType::kIdentifier || scope == nullptr) {
            return;
        }

        switch (node->node_type) {
            using enum VariableExpressionNode::NodeType;
        case kCommonVariable:
            node->linked_symbols = scope->FindSymbol(node->name);

            if (std::holds_alternative<std::monostate>(node->linked_symbols) ||
                (std::holds_alternative<const SymbolInfo*>(node->linked_symbols) &&
                 std::get<const SymbolInfo*>(node->linked_symbols) == nullptr))
            {
                auto function = document_.symbols.FindFunctionsByOriginalName(node->name);
                if (!std::holds_alternative<std::monostate>(function)) {
                    node->linked_symbols = std::move(function);
                    node->node_type = VariableExpressionNode::NodeType::kFunctionCallee;
                }
            }

            break;
        case kFunctionCallee: {
            auto FindConstructor = [node, scope]() -> void {
                const auto* symbol_result = scope->FindSymbol(node->name);
                node->linked_symbols = symbol_result;
            };

            auto it = function_cache_.find(node->name);
            if (it != function_cache_.end()) {
                if (std::holds_alternative<std::monostate>(it->second)) {
                    FindConstructor();
                } else {
                    node->linked_symbols = it->second;
                }

                break;
            }

            auto function_result = document_.symbols.FindFunctionsByOriginalName(node->name);
            auto [inserted_it, _] = function_cache_.try_emplace(node->name, std::move(function_result));

            if (std::holds_alternative<std::monostate>(inserted_it->second)) {
                // constructor calling, like "BufferReference ref = BufferReference(device_address);"
                FindConstructor();
                break;
            }

            node->linked_symbols = inserted_it->second;
            break;
        }
        default:
            break;
        }

        if (std::holds_alternative<SymbolList>(node->linked_symbols)) {
            document_.bindings.try_emplace(node->begin, node->linked_symbols);
        } else if (std::holds_alternative<const SymbolInfo*>(node->linked_symbols)) {
            const auto* symbol = std::get<const SymbolInfo*>(node->linked_symbols);
            if (!document_.macro_traces.contains(node->original_token.location)) {
                document_.bindings.try_emplace(node->original_token.location, symbol);
            }
        }
    }
}
