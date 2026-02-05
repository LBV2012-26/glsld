#include "stdafx.h"
#include "SymbolLinker.hpp"

namespace glsld {
    SymbolLinker::SymbolLinker(const DocumentSymbols& symbols)
        : symbols_{ symbols }
    {}

    void SymbolLinker::VisitVariableExpression(VariableExpressionNode * node) {
        auto* scope = node->scope;

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
    }
}
