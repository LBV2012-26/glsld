#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Analyzer/Syntax/Token.hpp"
#include "Base/Hash.hpp"

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

    struct SymbolInfo;
    struct TypeInfo {
        Token typename_token;
        const SymbolInfo* block_symbol{ nullptr };
        bool is_array{ false };

        bool is_builtin() const;
        bool is_valid() const;
    };

    class Scope;
    struct SymbolInfo {
        std::string name;
        SourceLocation location;
        SymbolKind kind{};
        TypeInfo type_info{};
        Scope* parent_scope{ nullptr };
        Scope* body_scope{ nullptr };

        operator bool() const;
    };

    enum class ScopeKind {
        kBlock,
        kCommon,
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

        SymbolInfo* AddSymbol(SymbolInfo symbol);
        SymbolInfo* AddSymbol(std::string_view name, SourceLocation location, SymbolKind kind);
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

        const Scope* FindScopeAt(SourceLocation location) const;
        const SymbolInfo* FindSymbolAt(std::string_view name, SourceLocation location) const;
        std::vector<const SymbolInfo*> FindFunctionsByOriginalName(std::string_view base_name) const;
        void Dump() const;

        Scope* const root_scope() const;

    private:
        const Scope* FindScopeRecursive(const Scope* current, SourceLocation location) const;
        bool IsLocationInScope(const Scope* scope, SourceLocation location) const;
        void PrintScopes(const Scope* scope, int indent_level) const;

        std::unordered_set<std::int64_t> visited_symbol_ids_;
        std::unique_ptr<Scope> root_scope_;
    };
}

#include "SymbolTable.inl"
