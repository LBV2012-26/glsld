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

        Document& document_;
    };
}
