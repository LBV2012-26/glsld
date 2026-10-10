#pragma once

#include <string>
#include <vector>

#include <Analyzer/Ast/AstVisitor.hpp>
#include <Analyzer/Syntax/Document.hpp>
#include <Base/FileSystem/Source.hpp>

namespace glsld {
    struct InlayHint {
        const SourceLocation* location{ nullptr };
        std::string           label;
        int                   offset{};
        int                   kind{ 2 };
        bool                  padding_right{ true };
    };

    class InlayHintCollector final : public AstVisitor {
    public:
        InlayHintCollector(const Document& document);

        const std::vector<InlayHint>& hints() const;

    private:
        void VisitNamespaceDeclaration(NamespaceDeclarationNode* node) override;
        void VisitFunctionDeclaration(FunctionDeclarationNode* node) override;
        void VisitVariableDeclaration(VariableDeclarationNode* node) override;
        void VisitInitializerListExpression(InitializerListExpressionNode* node) override;
        void VisitCallExpression(CallExpressionNode* node) override;

        void CollectMacroArgumentHints(const Document& document);

        const Document& document_;
        std::vector<InlayHint> hints_;
    };
}
