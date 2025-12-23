#include "stdafx.h"
#include "SymbolTable.hpp"

#include <algorithm>
#include <print>
#include <magic_enum/magic_enum_all.hpp>

namespace glsld {
    namespace {
        void PrintIndent(int level) {
            for (int i = 0; i < level; ++i) {
                std::print("  "); // 2 spaces per indent level
            }
        }
    }

    int ConvertSymbolKind(SymbolKind kind) {
        switch (kind) {
        case SymbolKind::kPreprocessor:
            return 2;  // Module
        case SymbolKind::kInterface:
            return 11; // Interface (用来区分普通 struct)
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

    SymbolInfo::SymbolInfo(std::string_view name, const SourceLocation& location, SymbolKind kind)
        : name{ name }
        , location{ location }
        , kind{ kind }
    {}

    SymbolInfo::operator bool() const {
        return !name.empty();
    }

    Scope::Scope(Scope* parent)
        : parent_{ parent }
        , index_{ parent ? parent->index_ + 1 : 0 }
    {}

    const SymbolInfo* Scope::FindSymbol(std::string_view name) const {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) {
            return &it->second;
        }

        if (parent_ != nullptr) {
            return parent_->FindSymbol(name);
        }

        return nullptr;
    }

    const SymbolInfo* Scope::FindSymbolInCurrentScope(std::string_view name) const {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) {
            return &it->second;
        }

        return nullptr;
    }

    bool Scope::AddSymbol(SymbolInfo symbol) {
        auto result = symbols_.try_emplace(symbol.name, std::move(symbol));
        return result.second;
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

    DocumentSymbols::DocumentSymbols()
        : root_scope_{ std::make_unique<Scope>(nullptr) }
    {}

    const Scope* DocumentSymbols::FindScopeAt(const SourceLocation& location) const {
        return FindScopeRecursive(root_scope_.get(), location);
    }

    const SymbolInfo* DocumentSymbols::FindSymbolAt(std::string_view name, const SourceLocation& location) const {
        const auto* scope = FindScopeAt(location);
        if (scope != nullptr) {
            return scope->FindSymbol(name);
        }

        return nullptr;
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

    const Scope* DocumentSymbols::FindScopeRecursive(const Scope* current, const SourceLocation& location) const {
        auto Comparer = [](const SourceLocation& source_loc, const SourceLocation& scope_loc) -> bool {
            return source_loc.line <  scope_loc.line ||
                  (source_loc.line == scope_loc.line && source_loc.column < scope_loc.column);
        };

        auto Projector = [](const std::unique_ptr<Scope>& scope) -> const SourceLocation& {
            return scope->interval_.first;
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

    bool DocumentSymbols::IsLocationInScope(const Scope* scope, const SourceLocation& location) const {
        if (location.line > scope->interval_.first.line && location.line < scope->interval_.second.line) {
            return true;
        }

        if (location.line == scope->interval_.first.line && location.column >= scope->interval_.first.column) {
            if (location.line < scope->interval_.second.line ||
                (location.line == scope->interval_.second.line && location.column < scope->interval_.second.column))
            {
                return true;
            }
        }

        return false;
    }

    void DocumentSymbols::PrintScopes(const Scope* scope, int indent_level) const {
        if (scope == nullptr) {
            return;
        }

        PrintIndent(indent_level);
        std::println("-> {} Scope index {} @ 0x{:X} (Parent: 0x{:X}) [Lines {}:{}-{}:{}]",
                     magic_enum::enum_name(scope->kind_),
                     scope->index_,
                     reinterpret_cast<std::uintptr_t>(scope),
                     reinterpret_cast<std::uintptr_t>(scope->parent_),
                     scope->interval_.first.line, scope->interval_.first.column,
                     scope->interval_.second.line, scope->interval_.second.column);

        if (!scope->symbols_.empty()) {
            PrintIndent(indent_level + 1);
            std::println("Symbols:");
            for (const auto& [name, symbol] : scope->symbols_) {
                PrintIndent(indent_level + 2);
                std::println("- '{}' (Kind: {}, Declared at L{})", symbol.name,
                             magic_enum::enum_name(symbol.kind), symbol.location.line);
            }
        }

        if (!scope->children_.empty()) {
            PrintIndent(indent_level + 1);
            std::println("Children Scopes:");
            for (const auto& child : scope->children_) {
                PrintScopes(child.get(), indent_level + 2);
            }
        }
    }
}
