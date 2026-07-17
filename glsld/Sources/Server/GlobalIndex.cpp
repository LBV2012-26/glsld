#include "stdafx.h"
#include "GlobalIndex.hpp"

#include <algorithm>
#include <compare>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <variant>

#include "Base/Logger.hpp"

namespace glsld {
    void GlobalIndex::IndexDocument(std::string_view uri, const Document& document) {
        std::vector<Contribution> contributions;
        contributions.reserve(document.bindings.size());

        auto AddSymbol = [&](const SourceLocation& ref_location, const SymbolInfo* symbol) -> void {
            if (symbol == nullptr) {
                return;
            }

            if (symbol->location.source_file() == nullptr || ref_location.source_file() == nullptr) {
                return;
            }

            contributions.push_back({
                .def_location = symbol->location,
                .ref_location = ref_location
            });
        };

        for (const auto& [location, symbol] : document.bindings) {
            if (const auto* single = std::get_if<const SymbolInfo*>(&symbol)) {
                AddSymbol(location, *single);
            } else if (const auto* list = std::get_if<SymbolList>(&symbol)) {
                for (const auto* single : *list) {
                    AddSymbol(location, single);
                }
            }
        }

        ApplyContributions(uri, std::move(contributions));
    }

    void GlobalIndex::RemoveDocument(std::string_view uri) {
        std::lock_guard lock(mutex_);

        auto document_it = document_contributions_.find(uri);
        if (document_it == document_contributions_.end()) {
            return;
        }

        WithdrawOldContributionLocked(document_it->second);
        document_contributions_.erase(document_it);
    }

    void GlobalIndex::ApplyContributions(std::string_view uri, std::vector<Contribution> contributions) {
        std::ranges::sort(contributions, [](const auto& lhs, const auto& rhs) -> bool {
            const auto def_compare = lhs.def_location <=> rhs.def_location;

            if (!std::is_eq(def_compare)) {
                return std::is_lt(def_compare);
            }

            return std::is_lt(lhs.ref_location <=> rhs.ref_location);
        });

        auto [first, last] = std::ranges::unique(contributions, [](const auto& lhs, const auto& rhs) -> bool {
            return lhs.def_location == rhs.def_location && lhs.ref_location == rhs.ref_location;
        });

        contributions.erase(first, last);

        std::lock_guard lock(mutex_);

        auto old_document_it = document_contributions_.find(uri);
        if (old_document_it != document_contributions_.end()) {
            WithdrawOldContributionLocked(old_document_it->second);
        }

        for (const auto& [def_location, ref_location] : contributions) {
            ++references_[def_location][ref_location];
        }

        document_contributions_.insert_or_assign(uri, std::move(contributions));
    }

    std::vector<SourceLocation> GlobalIndex::GetReferences(const SourceLocation& def_location) const {
        std::vector<SourceLocation> result;

        {
            std::shared_lock lock(mutex_);

            auto it = references_.find(def_location);
            if (it == references_.end()) {
                return result;
            }

            result.reserve(it->second.size());

            for (const auto& [location, ref_count] : it->second) {
                if (ref_count > 0) {
                    result.push_back(location);
                }
            }
        }

        std::ranges::sort(result, [](const auto& lhs, const auto& rhs) -> bool {
            return (lhs <=> rhs) < 0;
        });

        return result;
    }

    void GlobalIndex::DumpStatus() const {
        std::shared_lock lock(mutex_);

        auto unique_reference_count = 0uz;
        auto contribution_count     = 0uz;

        for (const auto& [_, references] : references_) {
            unique_reference_count += references.size();
        }

        for (const auto& [_, contributions] : document_contributions_) {
            contribution_count += contributions.size();
        }

        GLSLD_LOG_DEBUG(
            GLSLD_LOG_ROOT(),
            "[GlobalRefIndex] {} definitions, {} unique references, "
            "{} document contributions across {} documents",
            references_.size(),
            unique_reference_count,
            contribution_count,
            document_contributions_.size());
    }

    void GlobalIndex::Clear() {
        std::lock_guard lock(mutex_);
        references_.clear();
        document_contributions_.clear();
    }

    void GlobalIndex::WithdrawOldContributionLocked(std::span<const Contribution> contributions) {
        for (const auto& [def_location, ref_location] : contributions) {
            auto definition_it = references_.find(def_location);
            if (definition_it == references_.end()) {
                continue;
            }

            auto& ref_counts = definition_it->second;
            auto reference_it = ref_counts.find(ref_location);
            if (reference_it == ref_counts.end()) {
                continue;
            }

            if (reference_it->second <= 1) {
                ref_counts.erase(reference_it);
            } else {
                --reference_it->second;
            }

            if (ref_counts.empty()) {
                references_.erase(definition_it);
            }
        }
    }

    void TypeMemberIndex::IndexDocument(std::string_view uri, const DocumentSymbols& symbols) {
        std::vector<std::string> typenames;

        auto CollectStructMembers = [](const Scope* scope, SymbolList& members) -> void {
            for (const auto& [_, symbol] : scope->symbols()) {
                members.push_back(symbol.get());
            }
        };

        auto CollectFromScope = [&](this auto&& self, const Scope* scope) -> void {
            for (const auto& [_, symbol] : scope->symbols()) {
                if ((symbol->kind == SymbolKind::kStruct ||
                     symbol->kind == SymbolKind::kInterface) &&
                    symbol->internal_scope != nullptr)
                {
                    auto& members = type_members_[symbol->name];
                    // 类型可能因为包含文件会出现在多个文档中，只收集第一个
                    if (members.empty()) {
                        typenames.push_back(symbol->name);
                    }

                    CollectStructMembers(symbol->internal_scope, members);
                }

                for (const auto& child : scope->children()) {
                    self(child.get());
                }
            }
        };

        std::lock_guard lock(mutex_);
        CollectFromScope(symbols.root_scope());
        document_typenames_.insert_or_assign(uri, std::move(typenames));
    }

    void TypeMemberIndex::RemoveDocument(std::string_view uri) {
        std::lock_guard lock(mutex_);

        auto it = document_typenames_.find(uri);
        if (it == document_typenames_.end()) {
            return;
        }

        for (const auto& type_name : it->second) {
            type_members_.erase(type_name);
        }

        document_typenames_.erase(it);
    }

    SymbolList TypeMemberIndex::GetMembers(std::string_view type_name) const {
        std::shared_lock lock(mutex_);

        auto it = type_members_.find(type_name);
        if (it == type_members_.end()) {
            return {};
        }

        return it->second;
    }

    void TypeMemberIndex::Clear() {
        std::lock_guard lock(mutex_);
        type_members_.clear();
        document_typenames_.clear();
    }

    CompletionTrie::CompletionTrie()
        : root_{ std::make_unique<TrieNode>() }
    {}

    void CompletionTrie::Build(const DocumentSymbols& symbols) {
        Clear();
        CollectScope(symbols.root_scope());
    }

    void CompletionTrie::Search(std::string_view prefix, SymbolList& result) const {
        const auto* node = root_.get();
        for (char ch : prefix) {
            auto it = node->children.find(ch);
            if (it == node->children.end()) {
                return;
            }

            node = it->second.get();
        }

        auto Collect = [&result](this auto&& self, const TrieNode* node) -> void {
            result.append_range(node->symbols);
            for (const auto& [_, child] : node->children) {
                self(child.get());
            }
        };

        Collect(node);
    }

    void CompletionTrie::Clear() {
        root_ = std::make_unique<TrieNode>();
    }

    void CompletionTrie::Insert(std::string_view name, const SymbolInfo* symbol) {
        auto* node = root_.get();
        for (char ch : name) {
            auto& child = node->children[ch];
            if (child == nullptr) {
                child = std::make_unique<TrieNode>();
            }

            node = child.get();
        }

        node->symbols.push_back(symbol);
    }

    void CompletionTrie::CollectScope(const Scope& scope) {
        for (const auto& [_, symbol] : scope.symbols()) {
            Insert(symbol->name, symbol.get());
        }

        for (const auto& child : scope.children()) {
            CollectScope(*child);
        }
    }
}
