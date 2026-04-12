#pragma once

#include <atomic>
#include <memory>
#include "Analyzer/Syntax/Document.hpp"

namespace glsld {
    class MacroBinder {
    public:
        MacroBinder(Document& document, int version_replica, std::shared_ptr<const std::atomic<int>> version_pointer);

    private:
        void BindMacroInvocations();
        void BindMacroBodyIdentifiers();
        void BindMacroFunctionArguments();
        void BindMacroAfterDirective();

        Document&                               document_;
        int                                     version_replica_{};
        std::shared_ptr<const std::atomic<int>> version_pointer_;
    };
}
