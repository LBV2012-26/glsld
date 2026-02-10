#include "Symbol.hpp"

namespace glsld {
    inline bool TypeDescriptor::operator==(const TypeDescriptor& other) const {
        return family    == other.family
            && bits      == other.bits
            && rows      == other.rows
            && cols      == other.cols
            && is_matrix == other.is_matrix;
    }

    inline bool TypeInfo::is_builtin() const {
        return block_symbol == nullptr;
    }

    inline bool TypeInfo::is_valid() const {
        return !typename_token.text.empty();
    }

    inline bool TypeInfo::is_array() const {
        return !array_sizes.empty();
    }

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

    inline SymbolInfo* Scope::AddSymbol(SymbolInfo symbol) {
        auto [it, _] = symbols_.try_emplace(symbol.name, std::move(symbol));
        return &it->second;
    }

    inline SymbolInfo* Scope::AddSymbol(std::string_view name, SourceLocation location, SymbolKind kind) {
        SymbolInfo symbol{
            .name          = std::string(name),
            .location      = location,
            .kind          = kind,
            .located_scope = this
        };

        return AddSymbol(std::move(symbol));
    }

    inline Scope* const DocumentSymbols::root_scope() const {
        return root_scope_.get();
    }
}
