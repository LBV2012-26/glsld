#pragma once

#include "Analyzer/Syntax/Document.hpp"

namespace glsld {
    class MacroBinder {
    public:
        MacroBinder(Document& document);

        void BindMacro();

    private:
        void BindMacroInvocations();
        void BindMacroBodyIdentifiers();
        void BindMacroFunctionArguments();
        void BindMacroAfterDirective();

        Document& document_;
    };
}
