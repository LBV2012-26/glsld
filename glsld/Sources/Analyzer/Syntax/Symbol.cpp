#include "pch.hpp"
#include "Symbol.hpp"

#include <algorithm>
#include <format>
#include <print>
#include <ranges>

#include <magic_enum/magic_enum_all.hpp>

#include "Base/Hash.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    int ConvertSymbolKind(SymbolKind kind) {
        switch (kind) {
        case SymbolKind::kAttribute:
            return 7;  // Property
        case SymbolKind::kPreprocessor:
            return 2;  // Module
        case SymbolKind::kInterface:
            return 11; // Interface
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

    bool SpirvTypeSignature::operator==(const SpirvTypeSignature& other) const noexcept {
        return std::ranges::equal(extensions, other.extensions)
            && std::ranges::equal(capabilities, other.capabilities)
            && set == other.set
            && id  == other.id
            && std::ranges::equal(operands, other.operands);
    }

    bool TypeInfo::operator==(const TypeInfo& other) const {
        if (!CompareWithoutQualifiers(other)) {
            return false;
        }

        if (qualifiers.size() != other.qualifiers.size()) {
            return false;
        }

        for (auto i = 0uz; i != qualifiers.size(); ++i) {
            if (qualifiers[i].text != other.qualifiers[i].text ||
                qualifiers[i].type != other.qualifiers[i].type)
            {
                return false;
            }
        }

        return true;
    }

    std::size_t TypeDescriptorHash::operator()(const TypeDescriptor& desc) const {
        auto seed = 0uz;
        HashCombine(seed, static_cast<std::size_t>(desc.family));
        HashCombine(seed, static_cast<std::size_t>(desc.bits));
        HashCombine(seed, static_cast<std::size_t>(desc.vector_count));
        HashCombine(seed, static_cast<std::size_t>(desc.vector_length));
        return seed;
    }

    bool TypeInfo::CompareWithoutQualifiers(const TypeInfo& other) const {
        if ((is_func_ref && other.is_func_ref)) {
            return true;
        }

        if (typename_token.type != other.typename_token.type) {
            auto IsCore = [](TokenType type, BaseFamily family) -> bool {
                const bool is_core_type   = (type == TokenType::kPrimitive || type == TokenType::kBuiltInType);
                const bool is_core_family = (family == BaseFamily::kBool
                                          || family == BaseFamily::kInt
                                          || family == BaseFamily::kUint
                                          || family == BaseFamily::kFloat
                                          || family == BaseFamily::kVoid);

                return is_core_type && is_core_family;
            };

            if (!IsCore(typename_token.type, type_desc.family) ||
                !IsCore(other.typename_token.type, other.type_desc.family))
            {
                return false;
            }

            if (type_desc != other.type_desc) {
                return false;
            }
        } else {
            if (typename_token.text != other.typename_token.text) {
                return false;
            }
        }

        if (spirv_signature.has_value() != other.spirv_signature.has_value()) {
            return false;
        }

        if (typename_token.type == TokenType::kPrimitive) {
            if (type_desc.family != other.type_desc.family || type_desc.bits != other.type_desc.bits) {
                return false;
            }
        } else if (typename_token.type == TokenType::kBuiltInType || typename_token.type == TokenType::kIdentifier) {
            if (typename_token.text != other.typename_token.text) {
                return false;
            }
        }

        if (spirv_signature.has_value() && other.spirv_signature.has_value()) {
            if (!(spirv_signature.value() == other.spirv_signature.value())) {
                return false;
            }
        }

        if (block_symbol != other.block_symbol) {
            return false;
        }

        if (array_sizes.size() != other.array_sizes.size()) {
            return false;
        }

        for (auto i = 0uz; i != array_sizes.size(); ++i) {
            if (array_sizes[i] != other.array_sizes[i]) {
                return false;
            }
        }

        if (!template_args.empty() && !other.template_args.empty()) {
            if (template_args.size() != other.template_args.size()) {
                return false;
            }

            for (auto i = 0uz; i != template_args.size(); ++i) {
                if (template_args[i] != other.template_args[i]) {
                    return false;
                }
            }
        }

        return true;
    }

    Scope::Scope(Scope* parent)
        : parent_{ parent }
        , index_{ parent ? parent->index_ + 1 : 0 }
    {
        if (parent_ != nullptr) {
            visible_types_ = parent_->visible_types_;
        }
    }

    const SymbolInfo* Scope::FindSymbol(std::string_view name) const {
        for (const auto* scope = this; scope != nullptr; scope = scope->parent_) {
            if (const auto* symbol = scope->FindSymbolInCurrentScope(name)) {
                return symbol;
            }
        }

        for (const auto* builtin_parent : builtin_parents_) {
            if (builtin_parent != nullptr) {
                if (const auto* symbol = builtin_parent->FindSymbol(name)) {
                    return symbol;
                }
            }
        }

        return nullptr;
    }

    const SymbolInfo* Scope::FindTypeSymbol(std::string_view name) const {
        for (const auto* scope = this; scope != nullptr; scope = scope->parent_) {
            const auto* symbol = scope->FindSymbolInCurrentScope(name);
            if (symbol != nullptr && (symbol->kind == SymbolKind::kStruct || symbol->kind == SymbolKind::kInterface)) {
                return symbol;
            }

            for (const auto& child : scope->children_) {
                if (child->kind_ != ScopeKind::kGlobalTransparent &&
                    child->kind_ != ScopeKind::kBlockTransparent)
                {
                    continue;
                }

#pragma warning(push)
#pragma warning(disable : 4456)
                const auto* symbol = child->FindSymbolInCurrentScope(name);
                if (symbol != nullptr && (symbol->kind == SymbolKind::kStruct || symbol->kind == SymbolKind::kInterface)) {
                    return symbol;
                }
#pragma warning(pop)
            }
        }

        for (const auto* builtin_parent : builtin_parents_) {
            if (builtin_parent != nullptr) {
                if (const auto* symbol = builtin_parent->FindTypeSymbol(name)) {
                    return symbol;
                }
            }
        }

        return nullptr;
    }

    const SymbolInfo* Scope::FindSymbolInCurrentScope(std::string_view name) const {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) {
            return it->second.get();
        }

        for (const auto& child : children_) {
            if (child->kind_ == ScopeKind::kGlobalTransparent ||
                child->kind_ == ScopeKind::kBlockTransparent)
            {
                if (auto symbol = child->FindSymbolInCurrentScope(name)) {
                    return symbol;
                }
            }
        }

        auto interface_it = block_base_names_.find(name);
        if (interface_it != block_base_names_.end()) {
            return interface_it->second;
        }

        // 只有根作用域的 builtin_parents_ 可能不为空
        for (const auto* builtin_parent : builtin_parents_) {
            if (builtin_parent != nullptr) {
                if (const auto* symbol = builtin_parent->FindSymbolInCurrentScope(name)) {
                    return symbol;
                }
            }
        }

        return nullptr;
    }

    void Scope::GetVisibleSymbols(std::vector<const SymbolInfo*>& symbols) const {
        CollectLocalSymbols(symbols);

        if (parent_ != nullptr) {
            parent_->GetVisibleSymbols(symbols);
        }

        for (const auto* builtin_parent : builtin_parents_) {
            if (builtin_parent != nullptr) {
                builtin_parent->GetVisibleSymbols(symbols);
            }
        }
    }

    SymbolInfo* Scope::AddSymbol(SymbolInfo symbol) {
        auto unique_symbol = std::make_unique<SymbolInfo>(std::move(symbol));
        auto key           = unique_symbol->name;

        std::string base_name;

        if (unique_symbol->kind == SymbolKind::kInterface && unique_symbol->node != nullptr) {
            std::string_view storage;

            const auto* declaration = static_cast<const InterfaceDeclarationNode*>(unique_symbol->node);
            for (const auto& specifier : declaration->type_spec.specifiers) {
                if (specifier.text == "in" || specifier.text == "out") {
                    storage = specifier.text;
                    break;
                }
            }

            if (!storage.empty()) {
                base_name = key;
                key = std::format("{} {}", storage, key);
            }
        }

        auto [it, _] = symbols_.try_emplace(std::move(key), std::move(unique_symbol));
        auto* inserted_symbol = it->second.get();

        if (!base_name.empty()) {
            block_base_names_.try_emplace(base_name, inserted_symbol);
        }

        if (inserted_symbol->kind == SymbolKind::kStruct || inserted_symbol->kind == SymbolKind::kInterface) {
            visible_types_.try_emplace(inserted_symbol->name, inserted_symbol);
            if (!base_name.empty()) {
                visible_types_.try_emplace(std::move(base_name), inserted_symbol);
            }
        }

        return inserted_symbol;
    }

    SymbolInfo Scope::RemoveSymbol(std::string_view name) {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) {
            auto&& removed_symbol = std::move(it->second);
            symbols_.erase(it);
            return *removed_symbol;
        }

        return {};
    }

    void Scope::CollectLocalSymbols(std::vector<const SymbolInfo*>& symbols) const {
        for (const auto& [_, symbol] : symbols_) {
            symbols.push_back(symbol.get());
        }

        for (const auto& child : children_) {
            if (child->kind_ == ScopeKind::kGlobalTransparent ||
                child->kind_ == ScopeKind::kBlockTransparent)
            {
                child->CollectLocalSymbols(symbols);
            }
        }
    }

    DocumentSymbols::DocumentSymbols()
        : root_scope_{ std::make_unique<Scope>(nullptr) }
    {}

    SymbolInfo* DocumentSymbols::AddMacroSymbol(const AstNode * node, std::string_view name, const SourceLocation & location) {
        auto symbol = std::make_unique<SymbolInfo>(SymbolInfo{
            .name          = std::string(name),
            .location      = location,
            .kind          = SymbolKind::kMacro,
            .located_scope = root_scope_.get(),
            .node          = node
        });

        auto* result = symbol.get();
        macro_symbols_.push_back(std::move(symbol));
        macro_symbols_by_name_[result->name].push_back(result);
        return result;
    }

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

    const SymbolInfo* DocumentSymbols::FindMacroSymbol(const Token& definition) const {
        auto it = macro_symbols_by_name_.find(definition.text);
        if (it == macro_symbols_by_name_.end()) {
            return nullptr;
        }

        for (const auto* symbol : it->second | std::views::reverse) {
            if (symbol->location == definition.location) {
                return symbol;
            }
        }

        return nullptr;
    }

    SymbolReference DocumentSymbols::FindFunctionsByOriginalName(std::string_view base_name) const {
        SymbolList overloads;

        auto local_it = function_name_map_.find(base_name);
        if (local_it != function_name_map_.end()) {
            const auto& result = local_it->second;
            if (std::holds_alternative<const SymbolInfo*>(result)) {
                overloads.push_back(std::get<const SymbolInfo*>(result));
            } else if (std::holds_alternative<SymbolList>(result)) {
                const auto& list = std::get<SymbolList>(result);
                overloads.append_range(list);
            }
        }

        for (const auto* builtin : builtin_symbols_) {
            const auto result = builtin->FindFunctionsByOriginalName(base_name);

            std::visit(Overloaded{
                [&](const SymbolInfo* symbol) -> void {
                    overloads.push_back(symbol);
                },
                [&](const SymbolList& list) -> void {
                    overloads.append_range(list);
                },
                [](std::monostate) -> void {}
            }, result);
        }

        if (overloads.size() == 1) {
            return overloads.front();
        } else if (overloads.size() > 1) {
            return overloads;
        } else {
            return std::monostate{};
        }
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

    void DocumentSymbols::AddFunctionBaseName(std::string_view base_name, const SymbolInfo* symbol) {
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

    const Scope* DocumentSymbols::FindScopeRecursive(const Scope* current, const SourceLocation& location) const {
        auto Comparer = [](const SourceLocation& source_loc, const SourceLocation& scope_loc) -> bool {
            return *source_loc.source_file() == *scope_loc.source_file() && source_loc < scope_loc;
        };

        auto Projector = [](const std::unique_ptr<Scope>& scope) -> const SourceLocation& {
            return scope->interval_.first;
        };

        auto IsLocationInScope = [](const Scope* scope, const SourceLocation& location) -> bool {
            if (*scope->interval_.first.source_file()  != *location.source_file() &&
                *scope->interval_.second.source_file() != *location.source_file())
            {
                return false;
            }

            return (scope->interval_.first <= location) && (location < scope->interval_.second);
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

    void DocumentSymbols::PrintScopes(const Scope* scope, int indent_level) const {
        if (scope == nullptr) {
            return;
        }

        Utils::PrintIndent(indent_level);
        std::println("-> {} Scope index {}, host symbol: {} @ 0x{:X} (Parent: 0x{:X}) [Lines {}:{}-{}:{}]",
                     magic_enum::enum_name(scope->kind_),
                     scope->index_,
                     scope->host_symbol_ ? scope->host_symbol_->name : "<null>",
                     reinterpret_cast<std::uintptr_t>(scope),
                     reinterpret_cast<std::uintptr_t>(scope->parent_),
                     scope->interval_.first.line(), scope->interval_.first.column(),
                     scope->interval_.second.line(), scope->interval_.second.column());

        if (!scope->symbols_.empty()) {
            Utils::PrintIndent(indent_level + 1);
            std::println("Symbols:");
            for (const auto& [name, symbol] : scope->symbols_) {
                Utils::PrintIndent(indent_level + 2);
                std::println("- '{}' (Kind: {}, Declared at L{})", symbol->name, magic_enum::enum_name(symbol->kind), symbol->location.line());
            }
        }

        if (!scope->children_.empty()) {
            Utils::PrintIndent(indent_level + 1);
            std::println("Children Scopes:");
            for (const auto& child : scope->children_) {
                PrintScopes(child.get(), indent_level + 2);
            }
        }
    }
}
