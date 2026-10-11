#include "pch.hpp"
#include "Document.hpp"

#include <algorithm>
#include <ranges>
#include <utility>
#include <variant>

#include <Base/Logger.hpp>

namespace glsld {
    SymbolReference glsld::NamespaceInfo::Lookup(std::string_view name) const {
        auto it = members.find(name);
        return it != members.end() ? it->second : std::monostate{};
    }

    SymbolReferenceView Document::ReferenceSymbol(const SymbolReference& reference) const {
        if (std::holds_alternative<std::monostate>(reference)) {
            return std::monostate{};
        }

        if (std::holds_alternative<const SymbolInfo*>(reference)) {
            return std::get<const SymbolInfo*>(reference);
        }

        const auto& symbol_list = std::get<SymbolList>(reference);
        return arena->CopySpan<const SymbolInfo*>(symbol_list);
    }

    std::string_view Document::StoreTokenText(std::string_view text) const {
        return arena->CopyString(text);
    }

    void Document::StoreIncludeSource(IncludeSnapshot snapshot) {
        include_snapshots_.push_back(std::move(snapshot));
    }

    void Document::PrepareInjectedMacros(const SourceFile* source_file) {
        SourceLocation location(source_file, 0, 0);

        for (auto& [name, definition] : pending_macros_) {
            if (definition.original_token.location.source_file() == nullptr) {
                definition.original_token.location = location;
            }

            auto it = macro_table.find(name);
            if (it != macro_table.end() && it->second.original_token.location.source_file() == nullptr) {
                it->second.original_token.location = location;
            }
        }
    }

    void Document::InjectMacro(MacroDefinition definition) {
        definition.original_token.text = StoreTokenText(definition.original_token.text);

        for (auto& token : definition.replacement_list) {
            token.text = StoreTokenText(token.text);
        }

        for (auto& token : definition.params) {
            token.text = StoreTokenText(token.text);
        }

        const auto name = definition.original_token.text;
        macro_table.insert_or_assign(name, definition);
        pending_macros_.insert_or_assign(name, std::move(definition));
    }

    void Document::InjectMacro(std::string_view name) {
        InjectMacro(MacroDefinition{
            .is_function = false,
            .original_token = Token{
                .text = name,
                .type = TokenType::kIdentifier
            },
            .replacement_list = { Token{
                .text = "1",
                .type = TokenType::kNumberLiteral
            } }
        });
    }

    void Document::FinalizeInjectedMacros() {
        auto* root = symbols.root_scope();

        for (const auto& [name, definition] : pending_macros_) {
            const auto definition_location = definition.original_token.location;
            auto* node = arena->Construct<PreprocessorNode>(arena.get(), root);

            node->directive = "define";
            node->begin     = definition_location;

            node->end       = SourceLocation(
                definition_location.source_file(),
                definition_location.line(),
                definition_location.column() + static_cast<std::uint32_t>(name.size()));

            node->symbol    = symbols.AddMacroSymbol(node, name, definition_location);

            node->tokens.assign_range(definition.replacement_list);
            ast->pprefs.push_back(node);
        }

        pending_macros_.clear();
    }

    NamespaceInfo* Document::FindNamespace(const Scope* scope) {
        return const_cast<NamespaceInfo*>(std::as_const(*this).FindNamespace(scope));
    }

    const NamespaceInfo* Document::FindNamespace(const Scope* scope) const {
        if (scope == nullptr)
            return nullptr;
        if (scope == symbols.root_scope())
            return &global_namespace;
        if (scope->kind() != ScopeKind::kNamespace)
            return nullptr;

        return FindNamespace(scope->host_symbol());
    }

    NamespaceInfo* Document::FindNamespace(const SymbolInfo* symbol) {
        return const_cast<NamespaceInfo*>(std::as_const(*this).FindNamespace(symbol));
    }

    const NamespaceInfo* Document::FindNamespace(const SymbolInfo* symbol) const {
        if (symbol == nullptr || symbol->kind != SymbolKind::kNamespace) {
            return nullptr;
        }

        const auto it = namespace_symbols.find(symbol);
        return it == namespace_symbols.end() ? nullptr : it->second;
    }

    NamespaceInfo* Document::NamespaceForDeclaration(const Scope* scope) {
        while (scope != nullptr) {
            if (auto* space = FindNamespace(scope)) {
                return space;
            }

            if (scope->kind() != ScopeKind::kGlobalTransparent &&
                scope->kind() != ScopeKind::kBlockTransparent)
            {
                return nullptr;
            }

            scope = scope->parent();
        }

        return nullptr;
    }

    SymbolReference Document::LookupUnqualified(const Scope* scope, std::string_view name) const {
        for (auto* current = scope; current != nullptr; current = current->parent()) {
            if (const auto* space = FindNamespace(current)) {
                for (auto* owner = space; owner != nullptr; owner = owner->parent) {
                    auto found = owner->Lookup(name);
                    if (!std::holds_alternative<std::monostate>(found)) {
                        return found;
                    }
                }

                break;
            }

            if (const auto* symbol = current->FindSymbolInCurrentScope(name)) {
                return symbol;
            }
        }

        SymbolList functions;
        for (const auto& builtin : builtins) {
            auto found = builtin->LookupUnqualified(builtin->symbols.root_scope(), name);
            if (const auto* single = std::get_if<const SymbolInfo*>(&found)) {
                if (*single == nullptr) {
                    continue;
                }

                if ((*single)->kind != SymbolKind::kFunctionDecl &&
                    (*single)->kind != SymbolKind::kFunctionImpl)
                {
                    return *single;
                }

                functions.push_back(*single);
            } else if (const auto* list = std::get_if<SymbolList>(&found)) {
                functions.append_range(*list);
            }
        }

        DeduplicateFunctionOrigins(functions);

        if (functions.size() == 1)
            return functions.front();
        if (!functions.empty())
            return functions;
        return std::monostate{};
    }

    bool Document::RegisterNamespaceMember(const SymbolInfo* symbol, std::string_view name) {
        if (symbol == nullptr) {
            return true;
        }

        auto* space = NamespaceForDeclaration(symbol->located_scope);
        if (space == nullptr) {
            return true;
        }

        auto [it, instered] = space->members.try_emplace(name, symbol);
        if (instered) {
            return true;
        }

        auto IsFunction = [](const SymbolInfo* value) -> bool {
            return value != nullptr &&
                (value->kind == SymbolKind::kFunctionDecl || value->kind == SymbolKind::kFunctionImpl);
        };

        if (auto* single = std::get_if<const SymbolInfo*>(&it->second)) {
            if (*single == symbol)
                return true;
            if (!IsFunction(*single) || !IsFunction(symbol))
                return false;

            it->second = SymbolList{ *single, symbol };
            return true;
        }

        if (auto* list = std::get_if<SymbolList>(&it->second)) {
            if (!IsFunction(symbol))
                return false;
            if (!std::ranges::contains(*list, symbol))
                list->push_back(symbol);
            return true;
        }

        return false;
    }

    QualifiedScope Document::GetQualifiedScope(const SymbolInfo* symbol) const {
        if (symbol == nullptr) {
            return {};
        }

        // namespace or namespace alias
        if (symbol->kind == SymbolKind::kNamespace) {
            const auto* space = FindNamespace(symbol);
            return space != nullptr ? QualifiedScope(space) : QualifiedScope{};
        }

        // enum alias
        ankerl::unordered_dense::set<const SymbolInfo*> visited;

        while (symbol != nullptr && symbol->kind == SymbolKind::kTypeAlias) {
            if (!visited.insert(symbol).second) {
                return {};
            }

            if (symbol->node == nullptr || symbol->node->kind() != AstNodeKind::kTypeAliasDeclaration) {
                return {};
            }

            auto* decl = static_cast<const TypeAliasDeclarationNode*>(symbol->node);
            const auto& type_spec = decl->type_spec;

            if (!type_spec.array_sizes.empty() ||
                !type_spec.template_args.empty() ||
                type_spec.function_type != nullptr ||
                type_spec.spirv_type != nullptr)
            {
                return {};
            }

            symbol = type_spec.named_type_symbol;
        }

        if (symbol == nullptr || symbol->kind != SymbolKind::kEnum || symbol->internal_scope == nullptr) {
            return {};
        }

        return QualifiedScope(symbol->internal_scope);
    }
}
