#include "stdafx.h"
#include "SymbolLinker.hpp"

namespace glsld {
    SymbolLinker::SymbolLinker(const DocumentSymbols& symbols, BindingMap& bindings)
        : symbols_{ symbols }
        , bindings_{ bindings }
    {}

    void SymbolLinker::VisitVariableExpression(VariableExpressionNode * node) {
        Scope* scope = nullptr;
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
        case kFuncCallee:
            node->linked_symbols = symbols_.FindFunctionsByOriginalName(node->name);
            break;
        default:
            break;
        }

        if (!std::holds_alternative<std::monostate>(node->linked_symbols)) {
            bindings_.emplace(node->begin, node->linked_symbols);
        }
    }
}
