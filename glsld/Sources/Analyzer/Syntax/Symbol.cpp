#include "stdafx.h"
#include "Symbol.hpp"

#include <algorithm>
#include <format>
#include <print>
#include <magic_enum/magic_enum_all.hpp>

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

    bool TypeInfo::CompareWithoutQualifiers(const TypeInfo& other) const {
        if ((is_function_reference && other.is_function_reference)) {
            return true;
        }

        if (typename_token.text != other.typename_token.text ||
            typename_token.type != other.typename_token.type)
        {
            return false;
        }

        if (spirv_signature.has_value() != other.spirv_signature.has_value()) {
            return false;
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
    {}

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

                const auto* symbol = child->FindSymbolInCurrentScope(name);
                if (symbol != nullptr && (symbol->kind == SymbolKind::kStruct || symbol->kind == SymbolKind::kInterface)) {
                    return symbol;
                }
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

        for (const auto& [key, symbol] : symbols_) {
            if (symbol->kind == SymbolKind::kInterface && key.length() > name.length() + 1) {
                auto space_pos = key.rfind(' ');
                if (space_pos != std::string::npos && key.substr(space_pos + 1) == name) {
                    return symbol.get();
                }
            }
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

        if (unique_symbol->kind == SymbolKind::kInterface && unique_symbol->node != nullptr) {
            std::string_view storage;

            const auto* declare = static_cast<const InterfaceDeclarationNode*>(unique_symbol->node);
            for (const auto& specifier : declare->type_spec.specifiers) {
                if (specifier.text == "in" || specifier.text == "out") {
                    storage = specifier.text;
                    break;
                }
            }

            if (!storage.empty()) {
                key = std::format("{} {}", storage, key);
            }
        }

        auto [it, _] = symbols_.try_emplace(key, std::move(unique_symbol));
        return it->second.get();
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

    SymbolReference DocumentSymbols::FindFunctionsByOriginalName(std::string_view base_name) const {
        auto it = function_name_map_.find(base_name);
        if (it != function_name_map_.end()) {
            return it->second;
        }

        for (const auto* builtin : builtin_symbols_) {
            auto result = builtin->FindFunctionsByOriginalName(base_name);
            if (!std::holds_alternative<std::monostate>(result)) {
                return result;
            }
        }

        return std::monostate{};
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

        utils::PrintIndent(indent_level);
        std::println("-> {} Scope index {}, host symbol: {} @ 0x{:X} (Parent: 0x{:X}) [Lines {}:{}-{}:{}]",
                     magic_enum::enum_name(scope->kind_),
                     scope->index_,
                     scope->host_symbol_ ? scope->host_symbol_->name : "<null>",
                     reinterpret_cast<std::uintptr_t>(scope),
                     reinterpret_cast<std::uintptr_t>(scope->parent_),
                     scope->interval_.first.line(), scope->interval_.first.column(),
                     scope->interval_.second.line(), scope->interval_.second.column());

        if (!scope->symbols_.empty()) {
            utils::PrintIndent(indent_level + 1);
            std::println("Symbols:");
            for (const auto& [name, symbol] : scope->symbols_) {
                utils::PrintIndent(indent_level + 2);
                std::println("- '{}' (Kind: {}, Declared at L{})", symbol->name, magic_enum::enum_name(symbol->kind), symbol->location.line());
            }
        }

        if (!scope->children_.empty()) {
            utils::PrintIndent(indent_level + 1);
            std::println("Children Scopes:");
            for (const auto& child : scope->children_) {
                PrintScopes(child.get(), indent_level + 2);
            }
        }
    }
}
