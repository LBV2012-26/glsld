#include "stdafx.h"
#include "Symbol.hpp"

#include <algorithm>
#include <format>
#include <print>
#include <magic_enum/magic_enum_all.hpp>

#include "Utils/Utils.hpp"

namespace glsld {
    int ConvertSymbolKind(SymbolKind kind) {
        switch (kind) {
        case SymbolKind::kAttribute:
            return 7;  // Property
        case SymbolKind::kPreprocessor:
            return 2;  // Module
        case SymbolKind::kInterface:
            return 11; // Interface
        case SymbolKind::kFunctionDecl:
        case SymbolKind::kFunctionImpl:
            return 12; // Function
        case SymbolKind::kVariable:
        case SymbolKind::kParameter:
            return 13; // Variable
        case SymbolKind::kMacro:
            return 14; // Constant
        case SymbolKind::kStruct:
            return 23; // Struct
        default:
            return 13; // Fallback to Variable
        }
    }

    bool TypeInfo::operator==(const TypeInfo& other) const {
        if (typename_token.text != other.typename_token.text ||
            typename_token.type != other.typename_token.type)
        {
            return false;
        }

        if (block_symbol != other.block_symbol) {
            return false;
        }

        if (array_sizes.size() != other.array_sizes.size()) {
            return false;
        }

        for (auto i = 0uz; i != array_sizes.size(); ++i) {
            if (array_sizes[i].text != other.array_sizes[i].text ||
                array_sizes[i].type != other.array_sizes[i].type)
            {
                return false;
            }
        }

        if (qualifiers.size() != other.qualifiers.size()) {
            return false;
        }

        for (auto i = 0uz; i != qualifiers.size(); ++i) {
            if (qualifiers[i].text != other.qualifiers[i].text ||
                qualifiers[i].type != other.qualifiers[i].type)
            {
                return false;
            }
        }

        return true;
    }

    SymbolInfo::operator bool() const {
        return !name.empty();
    }

    Scope::Scope(Scope* parent)
        : parent_{ parent }
        , index_{ parent ? parent->index_ + 1 : 0 }
    {}

    const SymbolInfo* Scope::FindSymbol(std::string_view name) const {
        if (auto symbol = FindSymbolInCurrentScope(name)) {
            return symbol;
        }

        if (parent_ != nullptr) {
            return parent_->FindSymbol(name);
        }

        return nullptr;
    }

    const SymbolInfo* Scope::FindSymbolForHighlighting(std::string_view name) const {
        if (auto symbol = FindSymbolInCurrentScope(name)) {
            return symbol;
        }

        std::string decl_pattern = std::format("__Decl_{}", name);
        std::string impl_pattern = std::format("__Impl_{}", name);
        for (const auto& [mangled_name, info] : symbols_) {
            if (mangled_name.starts_with(decl_pattern) || mangled_name.starts_with(impl_pattern)) {
                return &info;
            }
        }

        if (parent_ != nullptr) {
            return parent_->FindSymbolForHighlighting(name);
        }

        return nullptr;
    }

    const SymbolInfo* Scope::FindSymbolInCurrentScope(std::string_view name) const {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) {
            return &it->second;
        }

        for (const auto& child : children_) {
            if (child->kind_ == ScopeKind::kGlobalTransparent ||
                child->kind_ == ScopeKind::kBlockTransparent)
            {
                if (auto symbol = child->FindSymbolInCurrentScope(name)) {
                    return symbol;
                }
            }
        }

        return nullptr;
    }

    void Scope::GetVisibleSymbols(std::vector<const SymbolInfo*>& symbols) const {
        CollectLocalSymbols(symbols);

        if (parent_ != nullptr) {
            parent_->GetVisibleSymbols(symbols);
        }
    }

    SymbolInfo Scope::RemoveSymbol(std::string_view name) {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) {
            auto&& removed_symbol = std::move(it->second);
            symbols_.erase(it);
            return removed_symbol;
        }

        return {};
    }

    void Scope::CollectLocalSymbols(std::vector<const SymbolInfo*>& symbols) const {
        for (const auto& [_, symbol] : symbols_) {
            symbols.push_back(&symbol);
        }

        for (const auto& child : children_) {
            if (child->kind_ == ScopeKind::kGlobalTransparent ||
                child->kind_ == ScopeKind::kBlockTransparent)
            {
                child->CollectLocalSymbols(symbols);
            }
        }
    }

    DocumentSymbols::DocumentSymbols()
        : root_scope_{ std::make_unique<Scope>(nullptr) }
    {}

    const Scope* DocumentSymbols::FindScopeAt(SourceLocation location) const {
        return FindScopeRecursive(root_scope_.get(), location);
    }

    const SymbolInfo* DocumentSymbols::FindSymbolAt(std::string_view name, SourceLocation location) const {
        const auto* scope = FindScopeAt(location);
        if (scope != nullptr) {
            return scope->FindSymbol(name);
        }

        return nullptr;
    }

    std::vector<const SymbolInfo*> DocumentSymbols::FindFunctionsByOriginalName(std::string_view base_name) const {
        std::vector<const SymbolInfo*> results;

        for (const auto& [mangled_name, symbol] : root_scope_->symbols_) {
            if (mangled_name.starts_with("__Decl_") || mangled_name.starts_with("__Impl_")) {
                if (mangled_name.contains(base_name)) {
                    results.push_back(&symbol);
                }
            }
        }

        return results;
    }

    void DocumentSymbols::Dump() const {
        std::println("=============== Symbol Tree Dump ===============");

        if (root_scope_) {
            PrintScopes(root_scope_.get(), 0);
        } else {
            std::println("Root scope is null.");
        }

        std::println("==============================================");
    }

    const Scope* DocumentSymbols::FindScopeRecursive(const Scope* current, SourceLocation location) const {
        auto Comparer = [](const SourceLocation& source_loc, const SourceLocation& scope_loc) -> bool {
            return source_loc < scope_loc;
        };

        auto Projector = [](const std::unique_ptr<Scope>& scope) -> const SourceLocation& {
            return scope->interval_.first;
        };

        auto IsLocationInScope = [](const Scope* scope, SourceLocation location) -> bool {
            return (scope->interval_.first <= location) && (location < scope->interval_.second);
        };

        auto it = std::ranges::upper_bound(current->children_, location, Comparer, Projector);
        if (it != current->children_.begin()) {
            const auto& candidate = *(--it);

            if (IsLocationInScope(candidate.get(), location)) {
                return FindScopeRecursive(candidate.get(), location);
            }
        }

        return current;
    }

    void DocumentSymbols::PrintScopes(const Scope* scope, int indent_level) const {
        if (scope == nullptr) {
            return;
        }

        utils::PrintIndent(indent_level);
        std::println("-> {} Scope index {}, host symbol: {} @ 0x{:X} (Parent: 0x{:X}) [Lines {}:{}-{}:{}]",
                     magic_enum::enum_name(scope->kind_),
                     scope->index_,
                     scope->host_symbol_ ? scope->host_symbol_->name : "<null>",
                     reinterpret_cast<std::uintptr_t>(scope),
                     reinterpret_cast<std::uintptr_t>(scope->parent_),
                     scope->interval_.first.line, scope->interval_.first.column,
                     scope->interval_.second.line, scope->interval_.second.column);

        if (!scope->symbols_.empty()) {
            utils::PrintIndent(indent_level + 1);
            std::println("Symbols:");
            for (const auto& [name, symbol] : scope->symbols_) {
                utils::PrintIndent(indent_level + 2);
                std::println("- '{}' (Kind: {}, Declared at L{})", symbol.name,
                             magic_enum::enum_name(symbol.kind), symbol.location.line);
            }
        }

        if (!scope->children_.empty()) {
            utils::PrintIndent(indent_level + 1);
            std::println("Children Scopes:");
            for (const auto& child : scope->children_) {
                PrintScopes(child.get(), indent_level + 2);
            }
        }
    }
}
