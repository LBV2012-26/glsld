#include "SymbolTable.hpp"

namespace glsld {
    inline const auto& Scope::interval() const {
        return interval_;
    }

    inline const auto& Scope::children() const {
        return children_;
    }

    inline const auto& Scope::symbols() const {
        return symbols_;
    }

    inline ScopeKind Scope::kind() const {
        return kind_;
    }

    inline const SymbolInfo* Scope::AddSymbol(SymbolInfo symbol) {
        auto [it, _] = symbols_.try_emplace(symbol.name, std::move(symbol));
        return &it->second;
    }

    inline const SymbolInfo* Scope::AddSymbol(std::string_view name, SourceLocation location, SymbolKind kind) {
        auto [it, _] = symbols_.try_emplace(std::string(name), std::string(name), location, kind);
        return &it->second;
    }

    inline Scope* const DocumentSymbols::root_scope() {
        return root_scope_.get();
    }
}
