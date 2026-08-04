#pragma once

#include <cstddef>
#include <memory>
#include <shared_mutex>
#include <span>
#include <string_view>
#include <vector>

#include <ankerl/unordered_dense.h>

#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Symbol.hpp"
#include "Analyzer/Syntax/Token.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    struct Contribution {
        SourceLocation definition;
        SourceLocation reference;
    };

    class GlobalIndex {
    public:
        void IndexDocument(std::string_view uri, const Document& document);
        void RestoreDocument(std::string_view uri, std::vector<Contribution> contributions);
        void RemoveDocument(std::string_view uri);

        std::vector<SourceLocation> GetReferences(const SourceLocation& definition) const;
        void DumpStatus() const;
        void Clear();

        static std::vector<Contribution> CollectContributions(const Document& document);

    private:
        void ApplyContributions(std::string_view uri, std::vector<Contribution> contributions);
        void ApplyContributionLocked(const Contribution& contribution);
        void WithdrawContributionsLocked(std::span<const Contribution> contributions);
        void WithdrawContributionLocked(const Contribution& contribution);

        using RefCountMap = ankerl::unordered_dense::map<SourceLocation, std::size_t, LocationHash>;
        ankerl::unordered_dense::map<SourceLocation, RefCountMap, LocationHash> references_; // [Definition, [Reference, RefCount]]

        StringHeteroHashMap<std::vector<Contribution>>                          document_contributions_;
        mutable std::shared_mutex                                               mutex_;
    };

    class TypeMemberIndex {
    public:
        void IndexDocument(std::string_view uri, const DocumentSymbols& symbols);
        void RemoveDocument(std::string_view uri);

        SymbolList GetMembers(std::string_view type_name) const;
        void Clear();

    private:
        StringHeteroHashMap<SymbolList>               type_members_;       // [TypeName, Fields]
        StringHeteroHashMap<std::vector<std::string>> document_typenames_; // [Uri, TypeNames]
        mutable std::shared_mutex                     mutex_;
    };
}
