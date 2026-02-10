#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
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

    enum class BaseFamily {
        kUnknown,
        kVoid,
        kBool,
        kInt,
        kUint,
        kFloat,
        kOpaque
    };

    struct TypeDescriptor {
        BaseFamily family{ BaseFamily::kUnknown };
        int        bits{};
        int        rows{ 1 };
        int        cols{ 1 };
        bool       is_matrix{ false };

        bool operator==(const TypeDescriptor& other) const;
    };

    struct SymbolInfo;
    struct TypeInfo {
        Token              typename_token;
        TypeDescriptor     type_desc;
        const SymbolInfo*  block_symbol{ nullptr };
        std::vector<Token> array_sizes;

        bool is_builtin() const;
        bool is_valid() const;
        bool is_array() const;

        bool operator==(const TypeInfo& other) const;
    };

    class Scope;
    struct SymbolInfo {
        std::string           name;
        SourceLocation        location;
        SymbolKind            kind{};
        TypeInfo              type_info{};
        std::vector<TypeInfo> param_typeinfos;
        Scope*                located_scope{ nullptr };
        Scope*                internal_scope{ nullptr };

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
        ScopeKind   kind() const;

    private:
        friend class DocumentSymbols;
        friend class Parser;

        SymbolInfo* AddSymbol(SymbolInfo symbol);
        SymbolInfo* AddSymbol(std::string_view name, SourceLocation location, SymbolKind kind);
        SymbolInfo  RemoveSymbol(std::string_view name);

        Scope*                                                parent_;
        std::size_t                                           index_;
        std::pair<SourceLocation, SourceLocation>             interval_;
        std::vector<std::unique_ptr<Scope>>                   children_;
        utils::StringHeteroHashTable<std::string, SymbolInfo> symbols_;
        ScopeKind                                             kind_{ ScopeKind::kTransparent };
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

        std::unique_ptr<Scope> root_scope_;
    };
}

#include "Symbol.inl"
