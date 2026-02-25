#pragma once


#include <cstdint>
#include <atomic>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Symbol.hpp"

namespace glsld {
    class TypeResolver : public AstVisitor {
    public:
        TypeResolver(Document& document, int version_replica,
                     std::shared_ptr<const std::atomic<int>> version_pointer);

    private:
        void VisitFunctionDeclaration(FunctionDeclarationNode* node) override;
        void VisitVariableDeclaration(VariableDeclarationNode* node) override;
        void VisitInterfaceDeclaration(InterfaceDeclarationNode* node) override;
        void VisitStructDeclaration(StructDeclarationNode* node) override;
        void VisitBinaryExpression(BinaryExpressionNode* node) override;
        void VisitUnaryExpression(UnaryExpressionNode* node) override;
        void VisitTernaryExpression(TernaryExpressionNode* node) override;
        void VisitCallExpression(CallExpressionNode* node) override;
        void VisitIndexExpression(IndexExpressionNode* node) override;
        void VisitVariableExpression(VariableExpressionNode* node) override;
        void VisitRawExpression(RawExpressionNode* node) override;
        void VisitMemberAccessExpression(MemberAccessExpressionNode* node) override;

        std::vector<std::int64_t> DeduceArraySizesFromArgs(const CallExpressionNode* call_node);
        TypeInfo ExtractTypeInfo(const TypeSpecifier& type_spec, const Scope* located_scope);
        TypeDescriptor ParseTypeDescriptor(std::string_view text);
        TypeInfo SniffLiteralType(const Token& token);
        TypeInfo ResolveSwizzleType(const TypeInfo& base_type, std::string_view swizzle);
        SymbolReference ResolveOverload(const SymbolList& candidates, std::span<const TypeInfo> call_arg_types);
        TypeInfo ResolveBinaryOperationType(const TypeInfo& left_type, const TypeInfo& right_type, TokenType op);
        TypeInfo ResolveArithmeticPromotion(const TypeInfo& left_type, const TypeInfo& right_type, TokenType op);

        Document& document_;
    };
}
