#pragma once

#include <atomic>
#include <memory>
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Symbol.hpp"

namespace glsld {
    class MacroBinder {
    public:
        MacroBinder(Document& document, int version_replica, VersionPointer version_pointer);

    private:
        void BindMacroInvocations();
        void BindMacroFunctionArguments();
        void BindPreprocessorIdentifiers();

        SymbolReferenceView ReferenceSymbol(const SymbolReference& reference);

        Document&      document_;
        int            version_replica_{};
        VersionPointer version_pointer_;
    };
}
