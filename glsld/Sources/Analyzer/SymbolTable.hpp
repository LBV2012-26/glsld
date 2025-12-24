#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Analyzer/Token.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    enum class SymbolKind {
        kFunctionDecl,
        kFunctionImpl,
        kInterface,
        kMacro,
        kParameter,
        kPreprocessor,
        kStruct,
        kVariable
    };

    int ConvertSymbolKind(SymbolKind kind);

    struct SymbolInfo {
        std::string    name;
        SourceLocation location;
        SymbolKind     kind{};

        SymbolInfo() = default;
        SymbolInfo(std::string_view name, const SourceLocation& location, SymbolKind kind);

        operator bool() const;
    };

    enum class ScopeKind {
        kCommon,
        kStruct,
        kTransparent
    };

    class Scope {
    public:
        Scope(Scope* parent);
        Scope(const Scope&)     = delete;
        Scope(Scope&&) noexcept = default;
        ~Scope()                = default;

        Scope& operator=(const Scope&)     = delete;
        Scope& operator=(Scope&&) noexcept = default;

        const SymbolInfo* FindSymbol(std::string_view name) const;
        const SymbolInfo* FindSymbolForHighlighting(std::string_view name) const;
        const SymbolInfo* FindSymbolInCurrentScope(std::string_view name) const;

        const auto& interval() const;
        const auto& children() const;
        const auto& symbols() const;
        ScopeKind kind() const;

    private:
        friend class DocumentSymbols;
        friend class Parser;

        bool AddSymbol(SymbolInfo symbol);
        SymbolInfo RemoveSymbol(std::string_view name);

        Scope* parent_;
        std::size_t index_;
        std::pair<SourceLocation, SourceLocation> interval_;
        std::vector<std::unique_ptr<Scope>> children_;
        utils::StringHeteroHashTable<std::string, SymbolInfo> symbols_;
        ScopeKind kind_{ ScopeKind::kTransparent };
    };

    class DocumentSymbols {
    public:
        DocumentSymbols();

        const Scope* FindScopeAt(const SourceLocation& location) const;
        const SymbolInfo* FindSymbolAt(std::string_view name, const SourceLocation& location) const;
        void Dump() const;

        Scope* const root_scope();

    private:
        const Scope* FindScopeRecursive(const Scope* current, const SourceLocation& location) const;
        bool IsLocationInScope(const Scope* scope, const SourceLocation& location) const;
        void PrintScopes(const Scope* scope, int indent_level) const;

        std::unordered_set<std::int64_t> visited_symbol_ids_;
        std::unique_ptr<Scope> root_scope_;
    };
}

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

    inline Scope* const DocumentSymbols::root_scope() {
        return root_scope_.get();
    }
}
