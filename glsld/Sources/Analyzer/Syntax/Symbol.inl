#include "Symbol.hpp"

namespace glsld {
    inline bool TypeDescriptor::operator==(const TypeDescriptor& other) const {
        return family       == other.family       && bits          == other.bits &&
               vector_count == other.vector_count && vector_length == other.vector_length;
    }

    inline bool TypeDescriptor::is_matrix() const {
        return vector_count > 1;
    }

    inline bool TypeInfo::is_builtin() const {
        return block_symbol == nullptr && typename_token.type == TokenType::kBuiltInType;
    }

    inline bool TypeInfo::is_valid() const {
        return !typename_token.text.empty();
    }

    inline bool TypeInfo::is_array() const {
        return !array_sizes.empty();
    }

    inline bool TypeInfo::is_const() const {
        for (const auto& qualifier : qualifiers) {
            if (qualifier.text == "const") {
                return true;
            }
        }

        return false;
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

    inline SymbolInfo* Scope::AddSymbol(const AstNode* node, std::string_view name, SourceLocation location, SymbolKind kind) {
        SymbolInfo symbol{
            .name          = std::string(name),
            .location      = location,
            .kind          = kind,
            .located_scope = this,
            .node          = node
        };

        return AddSymbol(std::move(symbol));
    }

    inline Scope* const DocumentSymbols::root_scope() const {
        return root_scope_.get();
    }
}
