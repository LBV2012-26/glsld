#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <string_view>
#include <vector>

#include <ankerl/unordered_dense.h>

#include "Analyzer/Ast/AstVisitor.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Symbol.hpp"

namespace glsld {
    enum class MatchGrade {
        kFailed     = 0,
        kWildcard   = 1,
        kImplicitly = 2,
        kExactMatch = 4
    };

    struct CandidateScore {
        const SymbolInfo*       symbol;
        std::vector<MatchGrade> param_grades;
    };

    struct RankedSignatureCandidates {
        SymbolList candidates;
        int active_index{ -1 };
    };

    class TypeResolver final : public AstVisitor {
    public:
        TypeResolver(Document& document, int version_replica, VersionPointer version_pointer);

        static RankedSignatureCandidates RankSignatureCandidates(const SymbolList& candidates, std::span<const TypeInfo> call_arg_types);

    private:
        void VisitTranslationUnit(TranslationUnitNode* node) override;
        void VisitFunctionDeclaration(FunctionDeclarationNode* node) override;
        void VisitVariableDeclaration(VariableDeclarationNode* node) override;
        void VisitInterfaceDeclaration(InterfaceDeclarationNode* node) override;
        void VisitStructDeclaration(StructDeclarationNode* node) override;
        void VisitInitializerListExpression(InitializerListExpressionNode* node) override;
        void VisitCastExpression(CastExpressionNode* node) override;
        void VisitBinaryExpression(BinaryExpressionNode* node) override;
        void VisitUnaryExpression(UnaryExpressionNode* node) override;
        void VisitTernaryExpression(TernaryExpressionNode* node) override;
        void VisitCallExpression(CallExpressionNode* node) override;
        void VisitIndexExpression(IndexExpressionNode* node) override;
        void VisitVariableExpression(VariableExpressionNode* node) override;
        void VisitRawExpression(RawExpressionNode* node) override;
        void VisitMemberAccessExpression(MemberAccessExpressionNode* node) override;

        void SeparateType(TypeInfo& type_info, bool keep_vector);
        TypeInfo GetCanonicalTypeInfo(const TypeDescriptor& type_desc);
        TypeInfo SplitCanonicalTypeInfo(const TypeInfo& base_type);
        std::optional<TemplateArgumentInfo> ExtractTemplateArgument(ExpressionNode* node, const Scope* located_scope);
        std::vector<std::int64_t> DeduceArraySizesFromArgs(const CallExpressionNode* call_node);
        const FunctionTypeInfo* BuildFunctionType(const SymbolInfo* symbol);
        std::span<const FunctionTypeInfo* const> BuildFunctionTypes(SymbolListView symbols);
        const FunctionTypeInfo* ExtractFunctionTypeInfo(const FunctionTypeSpec* function_type, const Scope* located_scope);
        std::expected<SpirvTypeSignature, std::string> BuildSpirvTypeSignature(const SpirvIntrinsicNode* node);
        TypeInfo ExtractTypeInfo(const TypeSpec& type_spec, const Scope* located_scope);
        TypeDescriptor ParseTypeDescriptor(std::string_view text);
        TypeInfo SniffLiteralType(const Token& token);
        TypeInfo ResolveSwizzleType(const TypeInfo& base_type, std::string_view swizzle);
        SymbolReference ResolveOverload(SymbolListView candidates, std::span<const TypeInfo> call_arg_types);
        TypeInfo ResolveBinaryOperationType(const TypeInfo& left_type, const TypeInfo& right_type, TokenType op);
        TypeInfo ResolveArithmeticPromotion(const TypeInfo& left_type, const TypeInfo& right_type, TokenType op);

        Document& document_;

        using TypeDescriptorCache = ankerl::unordered_dense::map<TypeDescriptor, TypeInfo, TypeDescriptorHash>;
        TypeDescriptorCache type_desc_cache_;

        bool is_signature_pass_{ true };
    };
}
