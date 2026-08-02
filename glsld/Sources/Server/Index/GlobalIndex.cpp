#include "pch.hpp"
#include "GlobalIndex.hpp"

#include <algorithm>
#include <compare>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <variant>

#include "Base/Logger.hpp"

namespace glsld {
    namespace {
        bool IsIndexableLocation(const SourceLocation& location) {
            const auto* source = location.source_file();
            return source != nullptr && source->indexable();
        }

        bool IsIndexableContribution(const Contribution& contribution) {
            return IsIndexableLocation(contribution.definition)
                && IsIndexableLocation(contribution.reference);
        }
    }

    void GlobalIndex::IndexDocument(std::string_view uri, const Document& document) {
        auto contributions = CollectContributions(document);
        ApplyContributions(uri, std::move(contributions));
    }

    void GlobalIndex::RestoreDocument(std::string_view uri, std::vector<Contribution> contributions) {
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

    std::vector<SourceLocation> GlobalIndex::GetReferences(const SourceLocation& definition) const {
        std::vector<SourceLocation> result;

        {
            std::shared_lock lock(mutex_);

            auto it = references_.find(definition);
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

        GLSLD_LOG(
            debug,
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

    std::vector<Contribution> GlobalIndex::CollectContributions(const Document& document) {
        std::vector<Contribution> contributions;
        contributions.reserve(document.bindings.size());

        auto AddSymbol = [&](const SourceLocation& reference, const SymbolInfo* symbol) -> void {
            if (symbol == nullptr ||
                symbol->location.source_file() == nullptr ||
                reference.source_file() == nullptr)
            {
                return;
            }

            Contribution contribution{
                .definition = symbol->location,
                .reference  = reference
            };

            if (!IsIndexableContribution(contribution)) {
                return;
            }

            contributions.push_back(std::move(contribution));
        };

        for (const auto& [location, symbols] : document.bindings) {
            if (const auto* single = std::get_if<const SymbolInfo*>(&symbols)) {
                AddSymbol(location, *single);
            } else if (const auto* list = std::get_if<SymbolList>(&symbols)) {
                for (const auto* symbol : *list) {
                    AddSymbol(location, symbol);
                }
            }
        }

        return contributions;
    }

    void GlobalIndex::ApplyContributions(std::string_view uri, std::vector<Contribution> contributions) {
        std::erase_if(contributions, [](const auto& contribution) -> bool {
            return !IsIndexableContribution(contribution);
        });

        std::ranges::sort(contributions, [](const auto& lhs, const auto& rhs) -> bool {
            auto def_compare = lhs.definition <=> rhs.definition;

            if (!std::is_eq(def_compare)) {
                return std::is_lt(def_compare);
            }

            return std::is_lt(lhs.reference <=> rhs.reference);
        });

        auto [first, last] = std::ranges::unique(contributions, [](const auto& lhs, const auto& rhs) -> bool {
            return lhs.definition == rhs.definition && lhs.reference == rhs.reference;
        });

        contributions.erase(first, last);

        std::lock_guard lock(mutex_);

        auto old_document_it = document_contributions_.find(uri);
        if (old_document_it != document_contributions_.end()) {
            WithdrawOldContributionLocked(old_document_it->second);
        }

        for (const auto& [definition, reference] : contributions) {
            ++references_[definition][reference];
        }

        document_contributions_.insert_or_assign(uri, std::move(contributions));
    }

    void GlobalIndex::WithdrawOldContributionLocked(std::span<const Contribution> contributions) {
        for (const auto& [definition, reference] : contributions) {
            auto definition_it = references_.find(definition);
            if (definition_it == references_.end()) {
                continue;
            }

            auto& ref_counts = definition_it->second;
            auto reference_it = ref_counts.find(reference);
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
}
