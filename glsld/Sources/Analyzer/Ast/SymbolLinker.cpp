#include "stdafx.h"
#include "SymbolLinker.hpp"

namespace glsld {
    SymbolLinker::SymbolLinker(const DocumentSymbols& symbols, BindingMap& bindings)
        : symbols_{ symbols }
        , bindings_{ bindings }
    {}

    void SymbolLinker::VisitVariableDeclaration(VariableDeclarationNode* node) {
        if (node->declared_symbol != nullptr) {
            auto location_pair = std::make_pair(node->begin, node->end);
            bindings_.emplace(location_pair, node->declared_symbol);
        }

        AstVisitor::VisitVariableDeclaration(node);
    }

    void SymbolLinker::VisitVariableExpression(VariableExpressionNode * node) {
        Scope* scope = nullptr;
        if (node->internal_scope != nullptr) {
            scope = node->internal_scope;
        } else {
            scope = node->located_scope;
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
            auto location_pair = std::make_pair(node->begin, node->end);
            bindings_.emplace(location_pair, node->linked_symbols);
        }
    }
}
