// Ast.cpp
#include "pch.hpp"
#include "Ast.hpp"

#include <algorithm>
#include <span>

namespace glsld {
    namespace {
        template <typename Ty>
        std::unique_ptr<Ty> CloneNode(const std::unique_ptr<Ty>& ptr) {
            if (ptr == nullptr) {
                return nullptr;
            }

            return std::unique_ptr<Ty>(static_cast<Ty*>(ptr->Clone().release()));
        }

        template <typename Ty>
        std::vector<std::unique_ptr<Ty>> CloneVector(std::span<const std::unique_ptr<Ty>> data) {
            std::vector<std::unique_ptr<Ty>> cloned;
            cloned.reserve(data.size());

            for (const auto& item : data) {
                cloned.push_back(CloneNode(item));
            }

            return cloned;
        }
    }

    AstNode::AstNode(Scope* scope)
        : located_scope{ scope }
    {}

    AstNode::AstNode(const AstNode& other)
        : begin{ other.begin }
        , end{ other.end }
        , located_scope{ other.located_scope }
        , internal_scope{ other.internal_scope }
    {}

    AstNode& AstNode::operator=(const AstNode& other) {
        if (this != &other) {
            begin          = other.begin;
            end            = other.end;
            located_scope  = other.located_scope;
            internal_scope = other.internal_scope;
        }

        return *this;
    }

    void* AstNode::operator new(std::size_t size) {
        return thread_local_arena->Allocate(size);
    }

    void* AstNode::operator new(std::size_t size, std::align_val_t alignment) {
        return thread_local_arena->Allocate(size, static_cast<std::size_t>(alignment));
    }

    void AstNode::operator delete(void* ptr, std::size_t size) noexcept {
        // Do nothing, memory will be freed when arena is reset or destroyed
    }

    void AstNode::operator delete(void* ptr, std::align_val_t alignment) noexcept {
        // Do nothing, memory will be freed when arena is reset or destroyed
    }

    void AstNode::operator delete(void* ptr, std::size_t size, std::align_val_t alignment) noexcept {
        // Do nothing, memory will be freed when arena is reset or destroyed
    }

    QualifierArgumentNode::QualifierArgumentNode(const QualifierArgumentNode& other)
        : AstNode(other)
        , arg_kind{ other.arg_kind }
        , token{ other.token }
        , rhs_expr{ CloneNode(other.rhs_expr) }
        , children{ CloneVector<QualifierArgumentNode>(other.children) }
    {}

    QualifierArgumentNode& QualifierArgumentNode::operator=(const QualifierArgumentNode& other) {
        if (this != &other) {
            QualifierArgumentNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    LayoutQualifierNode::LayoutQualifierNode(const LayoutQualifierNode& other)
        : AstNode(other)
        , raw_tokens{ other.raw_tokens }
        , params{ CloneVector<QualifierArgumentNode>(other.params) }
    {}

    LayoutQualifierNode& LayoutQualifierNode::operator=(const LayoutQualifierNode& other) {
        if (this != &other) {
            LayoutQualifierNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    SpirvIntrinsicNode::SpirvIntrinsicNode(const SpirvIntrinsicNode& other)
        : LayoutQualifierNode(other)
        , intrinsic_kind{ other.intrinsic_kind }
        , keyword{ other.keyword }
    {}

    SpirvIntrinsicNode& SpirvIntrinsicNode::operator=(const SpirvIntrinsicNode& other) {
        if (this != &other) {
            SpirvIntrinsicNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    ExpressionNode::ExpressionNode(const ExpressionNode& other)
        : AstNode(other)
        , evaluated_type{ other.evaluated_type }
    {}

    ExpressionNode& ExpressionNode::operator=(const ExpressionNode& other) {
        if (this != &other) {
            AstNode::operator=(other);
            evaluated_type = other.evaluated_type;
        }

        return *this;
    }

    AttributeNode::AttributeNode(const AttributeNode& other)
        : AstNode(other)
        , namespace_{ other.namespace_ }
        , name{ other.name }
        , argument{ CloneNode(other.argument) }
    {}

    AttributeNode& AttributeNode::operator=(const AttributeNode& other) {
        if (this != &other) {
            AttributeNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    StatementNode::StatementNode(const StatementNode& other)
        : AstNode(other)
        , attributes{ CloneVector<AttributeNode>(other.attributes) }
    {}

    StatementNode& StatementNode::operator=(const StatementNode& other) {
        if (this != &other) {
            AstNode::operator=(other);
            attributes = CloneVector<AttributeNode>(other.attributes);
        }

        return *this;
    }

    PreprocessorNode::PreprocessorNode(const PreprocessorNode& other)
        : StatementNode(other)
        , directive{ other.directive }
        , tokens{ other.tokens }
        , params{ other.params }
        , body{ CloneVector<StatementNode>(other.body) }
        , symbol{ other.symbol }
    {}

    PreprocessorNode& PreprocessorNode::operator=(const PreprocessorNode& other) {
        if (this != &other) {
            PreprocessorNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    DeclarationNode::DeclarationNode(const DeclarationNode& other)
        : StatementNode(other)
        , declared_symbol{ other.declared_symbol }
    {}

    DeclarationNode& DeclarationNode::operator=(const DeclarationNode& other) {
        if (this != &other) {
            AstNode::operator=(other);
            declared_symbol = other.declared_symbol;
        }

        return *this;
    }

    CompoundStatementNode::CompoundStatementNode(const CompoundStatementNode& other)
        : StatementNode(other)
        , children{ CloneVector<StatementNode>(other.children) }
    {}

    CompoundStatementNode& CompoundStatementNode::operator=(const CompoundStatementNode& other) {
        if (this != &other) {
            CompoundStatementNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    IfStatementNode::IfStatementNode(const IfStatementNode& other)
        : StatementNode(other)
        , condition{ CloneNode(other.condition) }
        , then_branch{ CloneNode(other.then_branch) }
        , else_branch{ CloneNode(other.else_branch) }
    {}

    IfStatementNode& IfStatementNode::operator=(const IfStatementNode& other) {
        if (this != &other) {
            IfStatementNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    ForStatementNode::ForStatementNode(const ForStatementNode& other)
        : StatementNode(other)
        , init{ CloneNode(other.init) }
        , condition{ CloneNode(other.condition) }
        , iteration{ CloneNode(other.iteration) }
        , body{ CloneNode(other.body) }
    {}

    ForStatementNode& ForStatementNode::operator=(const ForStatementNode& other) {
        if (this != &other) {
            ForStatementNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    WhileStatementNode::WhileStatementNode(const WhileStatementNode& other)
        : StatementNode(other)
        , condition{ CloneNode(other.condition) }
        , body{ CloneNode(other.body) }
    {}

    WhileStatementNode& WhileStatementNode::operator=(const WhileStatementNode& other) {
        if (this != &other) {
            WhileStatementNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    DoStatementNode::DoStatementNode(const DoStatementNode& other)
        : StatementNode(other)
        , body{ CloneNode(other.body) }
        , condition{ CloneNode(other.condition) }
    {}

    DoStatementNode& DoStatementNode::operator=(const DoStatementNode& other) {
        if (this != &other) {
            DoStatementNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    SwitchStatementNode::SwitchStatementNode(const SwitchStatementNode& other)
        : StatementNode(other)
        , condition{ CloneNode(other.condition) }
        , cases{ CloneVector<StatementNode>(other.cases) }
    {}

    SwitchStatementNode& SwitchStatementNode::operator=(const SwitchStatementNode& other) {
        if (this != &other) {
            SwitchStatementNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    CaseStatementNode::CaseStatementNode(const CaseStatementNode& other)
        : StatementNode(other)
        , condition{ CloneNode(other.condition) }
        , body{ CloneVector<StatementNode>(other.body) }
    {}

    CaseStatementNode& CaseStatementNode::operator=(const CaseStatementNode& other) {
        if (this != &other) {
            CaseStatementNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    ReturnStatementNode::ReturnStatementNode(const ReturnStatementNode& other)
        : StatementNode(other)
        , return_value{ CloneNode(other.return_value) }
    {}

    ReturnStatementNode& ReturnStatementNode::operator=(const ReturnStatementNode& other) {
        if (this != &other) {
            ReturnStatementNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    BreakStatementNode::BreakStatementNode(const BreakStatementNode& other)
        : StatementNode(other) {}

    BreakStatementNode& BreakStatementNode::operator=(const BreakStatementNode& other) {
        if (this != &other) {
            BreakStatementNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    ContinueStatementNode::ContinueStatementNode(const ContinueStatementNode& other)
        : StatementNode(other) {}

    ContinueStatementNode& ContinueStatementNode::operator=(const ContinueStatementNode& other) {
        if (this != &other) {
            ContinueStatementNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    DiscardStatementNode::DiscardStatementNode(const DiscardStatementNode& other)
        : StatementNode(other) {}

    DiscardStatementNode& DiscardStatementNode::operator=(const DiscardStatementNode& other) {
        if (this != &other) {
            DiscardStatementNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    ExpressionStatementNode::ExpressionStatementNode(const ExpressionStatementNode& other)
        : StatementNode(other)
        , expr{ CloneNode(other.expr) }
    {}

    ExpressionStatementNode& ExpressionStatementNode::operator=(const ExpressionStatementNode& other) {
        if (this != &other) {
            ExpressionStatementNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    NullStatementNode::NullStatementNode(const NullStatementNode& other)
        : StatementNode(other) {}

    NullStatementNode& NullStatementNode::operator=(const NullStatementNode& other) {
        if (this != &other) {
            NullStatementNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    InitializerListExpressionNode::InitializerListExpressionNode(const InitializerListExpressionNode& other)
        : ExpressionNode(other)
        , elements{ CloneVector<ExpressionNode>(other.elements) }
    {}

    InitializerListExpressionNode& InitializerListExpressionNode::operator=(const InitializerListExpressionNode& other) {
        if (this != &other) {
            InitializerListExpressionNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    BinaryExpressionNode::BinaryExpressionNode(const BinaryExpressionNode& other)
        : ExpressionNode(other)
        , op{ other.op }
        , left{ CloneNode(other.left) }
        , right{ CloneNode(other.right) }
    {}

    BinaryExpressionNode& BinaryExpressionNode::operator=(const BinaryExpressionNode& other) {
        if (this != &other) {
            BinaryExpressionNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    UnaryExpressionNode::UnaryExpressionNode(const UnaryExpressionNode& other)
        : ExpressionNode(other)
        , op{ other.op }
        , is_postfix{ other.is_postfix }
        , operand{ CloneNode(other.operand) }
    {}

    UnaryExpressionNode& UnaryExpressionNode::operator=(const UnaryExpressionNode& other) {
        if (this != &other) {
            UnaryExpressionNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    TernaryExpressionNode::TernaryExpressionNode(const TernaryExpressionNode& other)
        : ExpressionNode(other)
        , condition{ CloneNode(other.condition) }
        , true_expr{ CloneNode(other.true_expr) }
        , false_expr{ CloneNode(other.false_expr) }
    {}

    TernaryExpressionNode& TernaryExpressionNode::operator=(const TernaryExpressionNode& other) {
        if (this != &other) {
            TernaryExpressionNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    CallExpressionNode::CallExpressionNode(const CallExpressionNode& other)
        : ExpressionNode(other)
        , callee{ CloneNode(other.callee) }
        , args{ CloneVector<ExpressionNode>(other.args) }
    {}

    CallExpressionNode& CallExpressionNode::operator=(const CallExpressionNode& other) {
        if (this != &other) {
            CallExpressionNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    IndexExpressionNode::IndexExpressionNode(const IndexExpressionNode& other)
        : ExpressionNode(other)
        , base{ CloneNode(other.base) }
        , index{ CloneNode(other.index) }
    {}

    IndexExpressionNode& IndexExpressionNode::operator=(const IndexExpressionNode& other) {
        if (this != &other) {
            IndexExpressionNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    VariableExpressionNode::VariableExpressionNode(const VariableExpressionNode& other)
        : ExpressionNode(other)
        , original_token{ other.original_token }
        , node_type{ other.node_type }
        , name{ other.name }
        , linked_symbols{ other.linked_symbols }
    {}

    VariableExpressionNode& VariableExpressionNode::operator=(const VariableExpressionNode& other) {
        if (this != &other) {
            VariableExpressionNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    RawExpressionNode::RawExpressionNode(const RawExpressionNode& other)
        : ExpressionNode(other)
        , tokens{ other.tokens }
    {}

    RawExpressionNode& RawExpressionNode::operator=(const RawExpressionNode& other) {
        if (this != &other) {
            RawExpressionNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    MemberAccessExpressionNode::MemberAccessExpressionNode(const MemberAccessExpressionNode& other)
        : ExpressionNode(other)
        , object{ CloneNode(other.object) }
        , member{ CloneNode(other.member) }
    {}

    MemberAccessExpressionNode& MemberAccessExpressionNode::operator=(const MemberAccessExpressionNode& other) {
        if (this != &other) {
            MemberAccessExpressionNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    TypeSpecifier::TypeSpecifier(const TypeSpecifier& other)
        : specifiers{ other.specifiers }
        , template_args{ CloneVector<ExpressionNode>(other.template_args) }
        , array_sizes{ CloneVector<ExpressionNode>(other.array_sizes) }
        , layouts{ CloneVector<LayoutQualifierNode>(other.layouts) }
        , spirv_intrinsics{ CloneVector<SpirvIntrinsicNode>(other.spirv_intrinsics) }
    {
        if (other.spirv_type != nullptr && !spirv_intrinsics.empty()) {
            spirv_type = spirv_intrinsics.back().get();
        }
    }

    TypeSpecifier& TypeSpecifier::operator=(const TypeSpecifier& other) {
        if (this != &other) {
            specifiers       = other.specifiers;
            template_args    = CloneVector<ExpressionNode>(other.template_args);
            array_sizes      = CloneVector<ExpressionNode>(other.array_sizes);
            layouts          = CloneVector<LayoutQualifierNode>(other.layouts);
            spirv_intrinsics = CloneVector<SpirvIntrinsicNode>(other.spirv_intrinsics);

            if (other.spirv_type != nullptr && !spirv_intrinsics.empty()) {
                spirv_type = spirv_intrinsics.back().get();
            } else {
                spirv_type = nullptr;
            }
        }

        return *this;
    }

    bool TypeSpecifier::has_keyword(std::string_view name) const {
        return std::ranges::any_of(specifiers, [name](auto& token) -> bool {
            return token.text == name;
        });
    }

    VariableDeclarationNode::VariableDeclarationNode(const VariableDeclarationNode& other)
        : DeclarationNode(other)
        , init{ CloneNode(other.init) }
        , type_spec{ other.type_spec }
    {}

    VariableDeclarationNode& VariableDeclarationNode::operator=(const VariableDeclarationNode& other) {
        if (this != &other) {
            VariableDeclarationNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    DeclarationGroupNode::DeclarationGroupNode(const DeclarationGroupNode& other)
        : StatementNode(other)
        , declarations{ CloneVector<VariableDeclarationNode>(other.declarations) }
    {}

    DeclarationGroupNode& DeclarationGroupNode::operator=(const DeclarationGroupNode& other) {
        if (this != &other) {
            DeclarationGroupNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    FunctionDeclarationNode::FunctionDeclarationNode(const FunctionDeclarationNode& other)
        : DeclarationNode(other)
        , params{ CloneVector<VariableDeclarationNode>(other.params) }
        , body{ CloneNode(other.body) }
        , type_spec{ other.type_spec }
    {}

    FunctionDeclarationNode& FunctionDeclarationNode::operator=(const FunctionDeclarationNode& other) {
        if (this != &other) {
            FunctionDeclarationNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    InterfaceDeclarationNode::InterfaceDeclarationNode(const InterfaceDeclarationNode& other)
        : DeclarationNode(other)
        , body{ CloneNode(other.body) }
        , instances{ CloneNode(other.instances) }
        , type_spec{ other.type_spec }
    {}

    InterfaceDeclarationNode& InterfaceDeclarationNode::operator=(const InterfaceDeclarationNode& other) {
        if (this != &other) {
            InterfaceDeclarationNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    StructDeclarationNode::StructDeclarationNode(const StructDeclarationNode& other)
        : DeclarationNode(other)
        , body{ CloneNode(other.body) }
        , instances{ CloneNode(other.instances) }
    {}

    StructDeclarationNode& StructDeclarationNode::operator=(const StructDeclarationNode& other) {
        if (this != &other) {
            StructDeclarationNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }

    TranslationUnitNode::TranslationUnitNode(const TranslationUnitNode& other)
        : AstNode(other)
        , statements{ CloneVector<StatementNode>(other.statements) }
        , preprocessor_references{ other.preprocessor_references }
    {}

    TranslationUnitNode& TranslationUnitNode::operator=(const TranslationUnitNode& other) {
        if (this != &other) {
            TranslationUnitNode temp(other);
            std::swap(*this, temp);
        }

        return *this;
    }
}
