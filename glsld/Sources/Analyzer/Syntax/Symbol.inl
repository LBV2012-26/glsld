#include "Symbol.hpp"

namespace glsld {
    inline bool TypeDescriptor::operator==(const TypeDescriptor& other) const {
        return family       == other.family       && bits          == other.bits &&
               vector_count == other.vector_count && vector_length == other.vector_length;
    }

    inline TypeDescriptor::ArithmeticStructure TypeDescriptor::arithmetic_structure() const {
        if (vector_count == 1 && vector_length == 1)
            return ArithmeticStructure::kScalar;
        if (vector_count > 1)
            return ArithmeticStructure::kMatrix;
        return ArithmeticStructure::kVector;
    }

    inline bool SpirvOperandSignature::operator==(const SpirvOperandSignature& other) const {
        return kind == other.kind && value == other.value;
    }

    inline bool SpirvTypeSignature::operator==(const SpirvTypeSignature& other) const {
        return valid        == other.valid &&
               has_id       == other.has_id &&
               id           == other.id &&
               set          == other.set &&
               extensions   == other.extensions &&
               capabilities == other.capabilities &&
               operands     == other.operands;
    }

    inline bool TypeInfo::is_builtin() const {
        return block_symbol == nullptr && typename_token.type == TokenType::kBuiltInType;
    }

    inline bool TypeInfo::is_valid() const {
        return typename_token.type != TokenType::kUnknown;
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

    inline SymbolInfo::operator bool() const {
        return !name.empty();
    }

    inline void Scope::AddBuiltinScope(const Scope* builtin_scope) {
        builtin_parents_.push_back(builtin_scope);
    }

    inline const auto& Scope::host_symbol() const {
        return host_symbol_;
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
        auto unique_symbol = std::make_unique<SymbolInfo>(std::move(symbol));
        auto [it, _] = symbols_.try_emplace(unique_symbol->name, std::move(unique_symbol));
        return it->second.get();
    }

    inline SymbolInfo* Scope::AddSymbol(const AstNode* node, std::string_view name, const SourceLocation& location, SymbolKind kind) {
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

    inline void DocumentSymbols::AttachBuiltinSymbols(const DocumentSymbols* builtin) {
        builtin_symbols_.push_back(builtin);
    }

    inline void DocumentSymbols::AddFunctionBaseName(const std::string& base_name, const SymbolInfo* symbol) {
        auto& symbols = function_name_map_[base_name];

        if (std::holds_alternative<std::monostate>(symbols)) {
            symbols = symbol;
        } else if (std::holds_alternative<SymbolList>(symbols)) {
            std::get<SymbolList>(symbols).push_back(symbol);
        } else {
            const auto* existing_symbol = std::get<const SymbolInfo*>(symbols);
            symbols = SymbolList{ existing_symbol, symbol };
        }
    }
}
