#pragma once

#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"

namespace glsld {
    template <typename NodeType>
    class LocatorHelper : public AstVisitor {
    public:
        LocatorHelper(const SourceLocation& target)
            : AstVisitor(0, nullptr)
            , target_{ target }
        {}

        bool IsPositionInNode(const NodeType* node) const {
            // [begin.line, begin.col] <= target_ <= [end.line, end.col]
            return node->begin <= target_ && target_ <= node->end;
        }

        bool IsPositionDeeper(const NodeType* node) const {
            if (best_match_ == nullptr) {
                return true;
            }

            if (best_match_->begin > node->begin) {
                return false;
            } else if (best_match_->begin < node->begin) {
                return true;
            } else {
                // best_match_->begin == node->begin
                if (best_match_->end < node->end) {
                    return false;
                } else if (best_match_->end > node->end) {
                    return true;
                } else {
                    // best_match_->end == node->end
                    return true;
                }
            }

            return true;
        }

        const NodeType* const result() const {
            return best_match_;
        }

    protected:
        SourceLocation target_;
        NodeType*      best_match_{ nullptr };
    };

    class LeafLocator final : public LocatorHelper<AstNode> {
    public:
        LeafLocator(const Document& document, const SourceLocation& target);

    private:
        void Traverse(AstNode* node) override;
    };

    class ContextLocator final : public LocatorHelper<AstNode> {
    public:
        ContextLocator(const Document& document, const SourceLocation& target);

    private:
        void Traverse(AstNode* node) override;
        void VisitMemberAccessExpression(MemberAccessExpressionNode* node) override;
    };

    class SignatureLocator final : public LocatorHelper<CallExpressionNode> {
    public:
        SignatureLocator(const Document& document, const SourceLocation& target);

    private:
        void VisitCallExpression(CallExpressionNode* node) override;
    };
}
