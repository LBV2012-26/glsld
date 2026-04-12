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
            break;
        case kFunctionCallee: {
            auto function_result = document_.symbols.FindFunctionsByOriginalName(node->name);

            if (function_result.empty()) {
                // constructor calling, like "BufferReference ref = BufferReference(device_address);"
                const auto* symbol_result = scope->FindSymbol(node->name);
                node->linked_symbols = symbol_result;
                break;
            }

            node->linked_symbols = std::move(function_result);
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
