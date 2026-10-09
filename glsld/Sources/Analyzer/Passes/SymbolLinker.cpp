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

    void SymbolLinker::BindResolvedNames(std::span<const ResolvedNamePart> names) {
        for (const auto& [token, symbol] : names) {
            if (symbol != nullptr) {
                document_.bindings.insert_or_assign(token.location, symbol);
            }
        }
    }

    void SymbolLinker::TraverseTypeSpec(TypeSpec& type_spec) {
        BindResolvedNames(type_spec.name_prefixes);
        AstVisitor::TraverseTypeSpec(type_spec);
    }

    void SymbolLinker::VisitPreprocessor(PreprocessorNode* node) {
        if (node->directive == "define" && node->symbol != nullptr) {
            document_.bindings.try_emplace(node->symbol->location, node->symbol);
        }

        AstVisitor::VisitPreprocessor(node);
    }

    void SymbolLinker::VisitAttribute(AttributeNode* node) {
        const auto& symbols = document_.symbols.root_scope()->symbols();

        const auto it = symbols.find(node->name.text);
        if (it != symbols.end()) {
            document_.bindings.try_emplace(node->name.location, it->second.get());
        }

        AstVisitor::VisitAttribute(node);
    }

    void SymbolLinker::VisitNamespaceDeclaration(NamespaceDeclarationNode* node) {
        BindResolvedNames(node->resolved_names);

        if (!node->alias_target.empty() && node->declared_symbol != nullptr) {
            if (const auto* name = node->name()) {
                document_.bindings.insert_or_assign(name->location, node->declared_symbol);
            }
        }

        AstVisitor::VisitNamespaceDeclaration(node);
    }

    void SymbolLinker::VisitVariableExpression(VariableExpressionNode* node) {
        BindResolvedNames(node->name_prefixes);

        const Scope* scope = nullptr;
        if (node->internal_scope != nullptr) {
            scope = node->internal_scope;
        } else {
            scope = node->located_scope;
        }

        if (node->original_token.type != TokenType::kIdentifier || scope == nullptr) {
            return;
        }

        if (node->named_type_symbol != nullptr) {
            node->linked_symbols = node->named_type_symbol;
        } else {
            SymbolReference found;
            if (node->qualifier_space != nullptr) {
                found = node->qualifier_space->Lookup(node->name);
            } else {
                found = document_.LookupUnqualified(scope, node->name);
            }

            node->linked_symbols = document_.ReferenceSymbol(found);
        }

        const auto binding_location = node->original_token.location;

        std::visit(Overloaded{
            [&](const SymbolInfo* symbol) -> void {
                if (symbol != nullptr && !document_.macro_traces.contains(binding_location)) {
                    document_.bindings.insert_or_assign(binding_location, symbol);
                }
            },
            [&](SymbolListView list) -> void {
                if (!document_.macro_traces.contains(binding_location)) {
                    document_.bindings.insert_or_assign(binding_location, list);
                }
            },
            [](std::monostate) -> void {}
        }, node->linked_symbols);
    }
}
