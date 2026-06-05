#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "Analyzer/Syntax/Token.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    enum class SymbolKind {
        kAttribute,
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
        enum class ArithmeticStructure {
            kMatrix, kVector, kScalar
        };

        BaseFamily family{ BaseFamily::kUnknown };
        int bits{};
        // mat2x3 -> 2 x vector with size 3
        int vector_count{};
        int vector_length{};

        bool operator==(const TypeDescriptor& other) const;
        ArithmeticStructure arithmetic_structure() const;
    };

    enum class SpirvOperandKind {
        kLiteral,
        kIdReference
    };

    struct SpirvOperandSignature {
        SpirvOperandKind kind{ SpirvOperandKind::kLiteral };
        std::string      value;

        bool operator==(const SpirvOperandSignature& other) const;
    };

    struct SpirvTypeSignature {
        std::vector<std::string>           extensions;
        std::vector<std::int64_t>          capabilities;
        std::string                        set;
        std::int64_t                       id{};
        std::vector<SpirvOperandSignature> operands;
        std::string                        error;

        bool has_id{ false };
        bool valid{ false };

        bool operator==(const SpirvTypeSignature& other) const;
    };

    struct SymbolInfo;
    struct TypeInfo {
        Token                                     typename_token;
        TypeDescriptor                            type_desc;
        const SymbolInfo*                         block_symbol{ nullptr };
        std::vector<std::optional<std::uint64_t>> array_sizes;
        std::vector<Token>                        qualifiers;
        std::string                               spirv_type;
        std::optional<SpirvTypeSignature>         spirv_signature;

        bool operator==(const TypeInfo& other) const;
        bool CompareWithoutQualifiers(const TypeInfo& other) const;
        bool is_builtin() const;
        bool is_valid() const;
        bool is_array() const;
        bool is_const() const;
    };

    class Scope;
    struct AstNode;
    struct SymbolInfo {
        std::string           name;
        SourceLocation        location;
        SymbolKind            kind{};
        TypeInfo              type_info{};
        std::vector<TypeInfo> param_typeinfos;
        Scope*                located_scope{ nullptr };
        Scope*                internal_scope{ nullptr };
        const AstNode*        node{ nullptr };

        operator bool() const;
    };

    enum class ScopeKind {
        kBlock,
        kCommon,
        kMacroTemporary,
        kGlobalTransparent,
        kBlockTransparent
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
        const SymbolInfo* FindTypeSymbol(std::string_view name) const;
        const SymbolInfo* FindSymbolInCurrentScope(std::string_view name) const;
        void GetVisibleSymbols(std::vector<const SymbolInfo*>& symbols) const;

        const auto& host_symbol() const;
        const auto& interval() const;
        const auto& children() const;
        const auto& symbols() const;
        ScopeKind   kind() const;

    private:
        friend class DocumentSymbols;
        friend class Parser;

        SymbolInfo* AddSymbol(SymbolInfo symbol);
        SymbolInfo* AddSymbol(const AstNode* node, std::string_view name, const SourceLocation& location, SymbolKind kind);
        SymbolInfo  RemoveSymbol(std::string_view name);
        void CollectLocalSymbols(std::vector<const SymbolInfo*>& symbols) const;

        const Scope*                                     parent_;
        const SymbolInfo*                                host_symbol_{ nullptr };
        std::size_t                                      index_;
        std::pair<SourceLocation, SourceLocation>        interval_;
        std::vector<std::unique_ptr<Scope>>              children_;
        StringHeteroHashMap<std::unique_ptr<SymbolInfo>> symbols_;
        ScopeKind                                        kind_{ ScopeKind::kGlobalTransparent };
    };

    using SymbolList      = std::vector<const SymbolInfo*>;
    using SymbolReference = std::variant<std::monostate, const SymbolInfo*, SymbolList>;

    class DocumentSymbols {
    public:
        DocumentSymbols();

        const Scope* FindScopeAt(const SourceLocation& location) const;
        const SymbolInfo* FindSymbolAt(std::string_view name, const SourceLocation& location) const;
        SymbolReference FindFunctionsByOriginalName(std::string_view base_name) const;
        void Dump() const;

        void AddFunctionBaseName(const std::string& base_name, const SymbolInfo* symbol);

        Scope* const root_scope() const;

    private:
        const Scope* FindScopeRecursive(const Scope* current, const SourceLocation& location) const;
        void PrintScopes(const Scope* scope, int indent_level) const;

        std::unique_ptr<Scope>               root_scope_;
        StringHeteroHashMap<SymbolReference> function_name_map_;
    };
}

#include "Symbol.inl"
