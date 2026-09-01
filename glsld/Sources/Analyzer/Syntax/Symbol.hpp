#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <optional>
#include <span>
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
        int        bits{};

        // mat2x3 -> 2 x vector with size 3
        int vector_count{};
        int vector_length{};

        bool operator==(const TypeDescriptor& other) const;
        ArithmeticStructure arithmetic_structure() const;
    };

    struct TypeDescriptorHash {
        std::size_t operator()(const TypeDescriptor& desc) const;
    };

    enum class SpirvOperandKind {
        kLiteral,
        kIdReference
    };

    struct SpirvOperandSignature {
        SpirvOperandKind kind{ SpirvOperandKind::kLiteral };
        std::string_view value;

        bool operator==(const SpirvOperandSignature&) const noexcept = default;
    };

    struct SpirvTypeSignature {
        std::span<const std::string_view>      extensions;
        std::span<const std::int64_t>          capabilities;
        std::optional<std::string_view>        set;
        std::optional<std::int64_t>            id;
        std::span<const SpirvOperandSignature> operands;

        bool operator==(const SpirvTypeSignature& other) const noexcept;
    };

    struct SymbolInfo;
    struct TypeInfo {
        Token                                         typename_token;
        TypeDescriptor                                type_desc;
        const SymbolInfo*                             block_symbol{ nullptr };
        std::span<const std::optional<std::uint64_t>> array_sizes;
        std::span<const Token>                        qualifiers;
        std::span<const std::string_view>             template_args;
        std::string_view                              spirv_type;
        std::optional<SpirvTypeSignature>             spirv_signature;
        std::span<const std::string_view>             function_signatures;
        bool                                          is_func_ref{ false };

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
        kMacroBody,
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
        const SymbolInfo* FindVisibleType(std::string_view name) const;
        void GetVisibleSymbols(std::vector<const SymbolInfo*>& symbols) const;

        const SymbolInfo* closest_host() const;
        const SymbolInfo* host_symbol() const;
        const auto& interval() const;
        const auto& children() const;
        const auto& symbols() const;
        ScopeKind   kind() const;

    private:
        friend struct Document;
        friend class DocumentSymbols;
        friend class MetadataManager;
        friend class Parser;

        SymbolInfo* AddSymbol(SymbolInfo symbol);
        SymbolInfo* AddSymbol(const AstNode* node, std::string_view name, const SourceLocation& location, SymbolKind kind);
        SymbolInfo  RemoveSymbol(std::string_view name);
        void AddBuiltinScope(const Scope* builtin_scope);
        void CollectLocalSymbols(std::vector<const SymbolInfo*>& symbols) const;

        std::vector<const Scope*>                        builtin_parents_;
        const Scope*                                     parent_;
        const SymbolInfo*                                host_symbol_{ nullptr };
        std::size_t                                      index_;
        std::pair<SourceLocation, SourceLocation>        interval_;
        std::vector<std::unique_ptr<Scope>>              children_;
        StringHeteroHashMap<std::unique_ptr<SymbolInfo>> symbols_;
        StringHeteroHashMap<SymbolInfo*>                 block_base_names_;
        StringHeteroHashMap<SymbolInfo*>                 visible_types_;
        ScopeKind                                        kind_{ ScopeKind::kGlobalTransparent };
    };

    using SymbolList          = std::vector<const SymbolInfo*>;
    using SymbolReference     = std::variant<std::monostate, const SymbolInfo*, SymbolList>;
    using SymbolListView      = std::span<const SymbolInfo* const>;
    using SymbolReferenceView = std::variant<std::monostate, const SymbolInfo*, SymbolListView>;

    template <typename... Ts>
    struct Overloaded : Ts... {
        using Ts::operator()...;
    };

    template <typename... Ts>
    Overloaded(Ts...) -> Overloaded<Ts...>;

    class DocumentSymbols {
    public:
        DocumentSymbols();

        SymbolInfo* AddMacroSymbol(const AstNode* node, std::string_view name, const SourceLocation& location);

        const Scope* FindScopeAt(const SourceLocation& location) const;
        const SymbolInfo* FindSymbolAt(std::string_view name, const SourceLocation& location) const;
        const SymbolInfo* FindMacroSymbol(const Token& definition) const;
        SymbolReference FindFunctionsByOriginalName(std::string_view base_name) const;
        void Dump() const;

        void AttachBuiltinSymbols(const DocumentSymbols* builtin);
        void AddFunctionBaseName(std::string_view base_name, const SymbolInfo* symbol);

        Scope* root_scope() const;
        const auto& macro_symbols() const;

    private:
        const Scope* FindScopeRecursive(const Scope* current, const SourceLocation& location) const;
        void PrintScopes(const Scope* scope, int indent_level) const;

        std::unique_ptr<Scope>                              root_scope_;
        std::vector<const DocumentSymbols*>                 builtin_symbols_;
        StringHeteroHashMap<SymbolReference>                function_name_map_;
        std::vector<std::unique_ptr<SymbolInfo>>            macro_symbols_;
        StringHeteroHashMap<std::vector<const SymbolInfo*>> macro_symbols_by_name_;
    };
}

#include "Symbol.inl"
