#pragma once

#include <FrontEnd/Syntax/Document.hpp>
#include <FrontEnd/Syntax/Symbol.hpp>

namespace glsld {
    class MacroBinder {
    public:
        MacroBinder(Document& document, int version_replica, VersionPointer version_pointer);

    private:
        void BindMacroInvocations();
        void BindMacroFunctionArguments();
        void BindPreprocessorIdentifiers();

        Document&      document_;
        int            version_replica_{};
        VersionPointer version_pointer_;
    };
}
