#include "stdafx.h"
#include "SymbolLinker.hpp"

#include <utility>

namespace glsld {
    SymbolLinker::SymbolLinker(const DocumentSymbols& symbols, BindingMap& bindings)
        : symbols_{ symbols }
        , bindings_{ bindings }
    {}

    void SymbolLinker::VisitPreprocessor(PreprocessorNode* node) {
        if (node->directive == "define" && node->symbol != nullptr) {
            bindings_.try_emplace(node->symbol->location, node->symbol);
        }
    }

    void SymbolLinker::VisitVariableExpression(VariableExpressionNode* node) {
        const Scope* scope = nullptr;
        if (node->internal_scope != nullptr) {
            scope = node->internal_scope;
        } else {
            scope = node->located_scope;
        }

        if (node->token_type != TokenType::kIdentifier || scope == nullptr) {
            return;
        }

        switch (node->node_type) {
            using enum VariableExpressionNode::NodeType;
        case kCommonVariable:
            node->linked_symbols = scope->FindSymbol(node->name);
            break;
        case kFunctionCallee: {
            auto function_result = symbols_.FindFunctionsByOriginalName(node->name);

            if (function_result.empty()) { // constructor calling, like "BufferReference ref = BufferReference(device_address);"
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

        if (!std::holds_alternative<std::monostate>(node->linked_symbols)) {
            bindings_.try_emplace(node->begin, node->linked_symbols);
        }
    }
}
