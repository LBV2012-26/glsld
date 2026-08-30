#include "pch.hpp"
#include "SymbolLinker.hpp"

#include <utility>
#include <variant>

namespace glsld {
    SymbolLinker::SymbolLinker(Document& document, int version_replica, VersionPointer vesion_pointer)
        : AstVisitor(version_replica, vesion_pointer)
        , document_{ document }
    {
        Traverse(document_.ast);
    }

    void SymbolLinker::VisitPreprocessor(PreprocessorNode* node) {
        if (node->directive == "define" && node->symbol != nullptr) {
            document_.bindings.try_emplace(node->symbol->location, node->symbol);
        }

        for (auto& statement : node->body) {
            Traverse(statement);
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
        case kCommonVariable: {
            node->linked_symbols = scope->FindSymbol(node->name);

            const bool nothing_linked = std::holds_alternative<std::monostate>(node->linked_symbols);
            const bool linked_nullptr = std::holds_alternative<const SymbolInfo*>(node->linked_symbols)
                                     && std::get<const SymbolInfo*>(node->linked_symbols) == nullptr;

            if (nothing_linked || linked_nullptr) {
                const auto functions = document_.symbols.FindFunctionsByOriginalName(node->name);
                if (!std::holds_alternative<std::monostate>(functions)) {
                    node->linked_symbols = document_.ReferenceSymbol(functions);
                    node->node_type      = VariableExpressionNode::NodeType::kFunctionCallee;
                }
            }

            break;
        }
        case kFunctionCallee: {
            auto FindConstructor = [node, scope]() -> void {
                node->linked_symbols = scope->FindVisibleType(node->name);
            };

            auto it = function_cache_.find(node->name);
            if (it != function_cache_.end()) {
                if (std::holds_alternative<std::monostate>(it->second)) {
                    FindConstructor();
                } else {
                    node->linked_symbols = document_.ReferenceSymbol(it->second);
                }

                break;
            }

            auto functions = document_.symbols.FindFunctionsByOriginalName(node->name);
            auto [inserted_it, _] = function_cache_.try_emplace(node->name, std::move(functions));

            if (std::holds_alternative<std::monostate>(inserted_it->second)) {
                // constructor calling, like "BufferReference ref = BufferReference(device_address);"
                FindConstructor();
                break;
            }

            node->linked_symbols = document_.ReferenceSymbol(inserted_it->second);
            break;
        }
        default:
            break;
        }

        std::visit(Overloaded{
            [&](const SymbolInfo* symbol) -> void {
                if (!document_.macro_traces.contains(node->original_token.location)) {
                    document_.bindings.try_emplace(node->original_token.location, symbol);
                }
            },
            [&](SymbolListView list) -> void {
                document_.bindings.try_emplace(node->begin, list);
            },
            [](std::monostate) -> void {}
        }, node->linked_symbols);
    }
}
