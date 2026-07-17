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
    class GlobalIndex {
    public:
        struct Contribution {
            SourceLocation def_location;
            SourceLocation ref_location;
        };

        void IndexDocument(std::string_view uri, const Document& document);
        void RemoveDocument(std::string_view uri);
        void ApplyContributions(std::string_view uri, std::vector<Contribution> contributions);

        std::vector<SourceLocation> GetReferences(const SourceLocation& def_location) const;
        void DumpStatus() const;
        void Clear();

    private:
        void WithdrawOldContributionLocked(std::span<const Contribution> contributions);

        using RefCountMap = ankerl::unordered_dense::map<SourceLocation, std::size_t, LocationHash>;
        ankerl::unordered_dense::map<SourceLocation, RefCountMap, LocationHash> references_; // [def_location, [ref_location, count]]

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
        StringHeteroHashMap<SymbolList>               type_members_;       // [type_name, members]
        StringHeteroHashMap<std::vector<std::string>> document_typenames_; // [uri, typenames]
        mutable std::shared_mutex                     mutex_;
    };

    class CompletionTrie {
    public:
        struct TrieNode {
            SymbolList                                                    symbols;
            ankerl::unordered_dense::map<char, std::unique_ptr<TrieNode>> children;
        };

        CompletionTrie();

        void Build(const DocumentSymbols& symbols);
        void Search(std::string_view prefix, SymbolList& result) const;
        void Clear();

    private:
        void Insert(std::string_view name, const SymbolInfo* symbol);
        void CollectScope(const Scope& scope);

        std::unique_ptr<TrieNode> root_;
    };
}
