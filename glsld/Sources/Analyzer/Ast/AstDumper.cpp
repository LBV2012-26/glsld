#include "stdafx.h"
#include "AstDumper.hpp"

#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <print>

#include <magic_enum/magic_enum_all.hpp>
#include "Utils/Utils.hpp"

namespace glsld {
    void AstDumper::VisitTranslationUnit(TranslationUnitNode* node) {
        PrintIndent();
        std::println("TranslationUnit {}", FormatRange(node));

        ++indent_level_;
        Base::VisitTranslationUnit(node);
        --indent_level_;
    }

    void AstDumper::VisitDeclarationGroup(DeclarationGroupNode* node) {
        PrintIndent();
        std::println("DeclarationGroup {}", FormatRange(node));

        ++indent_level_;
        Base::VisitDeclarationGroup(node);
        --indent_level_;
    }

    void AstDumper::VisitPreprocessor(PreprocessorNode* node) {
        PrintIndent();
        std::println("Preprocessor #{} (Tokens: {}) {}", node->directive, node->tokens.size(), FormatRange(node));
    }

    void AstDumper::VisitAttribute(AttributeNode* node) {
        PrintIndent();
        std::string dump_info;
        if (node->namespace_.type != TokenType::kUnknown) {
            dump_info = std::format("Attribute [[{}::{}]]", node->namespace_.text, node->name.text);
        } else {
            dump_info = std::format("Attribute [[{}]]", node->name.text);
        }

        if (node->argument != nullptr) {
            dump_info += " Argument: ";
            std::print("{}", dump_info);
            TraverseWithoutIndent(node->argument.get());
        } else {
            std::println("{}", dump_info);
        }
    }

    void AstDumper::VisitFunctionDeclaration(FunctionDeclarationNode* node) {
        PrintIndent();
        std::string name = node->declared_symbol ? node->declared_symbol->name : "unnamed";
        std::println("FunctionDeclaration '{}' Type: {} {}", name, TypeToString(node->type_spec), FormatRange(node));

        ++indent_level_;

        PrintIndent();
        std::println("Parameters:");
        ++indent_level_;
        for (const auto& param : node->params) {
            Traverse(param.get());
        }
        --indent_level_;

        if (node->body != nullptr) {
            PrintIndent();
            std::println("Body:");
            ++indent_level_;
            Traverse(node->body.get());
            --indent_level_;
        }

        --indent_level_;
    }

    void AstDumper::VisitVariableDeclaration(VariableDeclarationNode* node) {
        PrintIndent();
        std::string name = node->declared_symbol ? node->declared_symbol->name : "<anon>";
        std::string type = node->declared_symbol ? node->declared_symbol->type_info.typename_token.text : TypeToString(node->type_spec);

        std::println("VariableDeclaration '{}' Type: {} {}", name, type, FormatRange(node));

        ++indent_level_;

        if (!node->type_spec.array_sizes.empty()) {
            PrintIndent();
            std::println("ArraySize:");
            ++indent_level_;
            for (const auto& array_size : node->type_spec.array_sizes) {
                Traverse(array_size.get());
            }
            --indent_level_;
        }

        if (node->init != nullptr) {
            PrintIndent();
            std::print("Init: ");
            TraverseWithoutIndent(node->init.get());
        }

        --indent_level_;
    }

    void AstDumper::VisitInterfaceDeclaration(InterfaceDeclarationNode* node) {
        PrintIndent();
        std::string name = node->declared_symbol ? node->declared_symbol->name : "<anon>";
        std::println("InterfaceDeclaration '{}' {}", name, FormatRange(node));

        ++indent_level_;

        if (node->body != nullptr) {
            PrintIndent();
            std::println("Body:");
            ++indent_level_;
            Traverse(node->body.get());
            --indent_level_;
        }

        if (node->instances != nullptr) {
            PrintIndent();
            std::println("Instances:");
            ++indent_level_;
            Traverse(node->instances.get());
            --indent_level_;
        }

        --indent_level_;
    }

    void AstDumper::VisitStructDeclaration(StructDeclarationNode* node) {
        PrintIndent();
        std::string name = node->declared_symbol ? node->declared_symbol->name : "<anon>";
        std::println("StructDeclaration '{}' {}", name, FormatRange(node));

        ++indent_level_;

        if (node->body != nullptr) {
            PrintIndent();
            std::println("Body:");
            ++indent_level_;
            Traverse(node->body.get());
            --indent_level_;
        }

        if (node->instances != nullptr) {
            PrintIndent();
            std::println("Instances:");
            ++indent_level_;
            Traverse(node->instances.get());
            --indent_level_;
        }

        --indent_level_;
    }

    void AstDumper::VisitCompoundStatement(CompoundStatementNode* node) {
        PrintIndent();
        std::println("CompoundStatement (Scope: 0x{:X}) {}", reinterpret_cast<std::uintptr_t>(node->internal_scope), FormatRange(node));

        ++indent_level_;
        Base::VisitCompoundStatement(node);
        --indent_level_;
    }

    void AstDumper::VisitIfStatement(IfStatementNode* node) {
        PrintIndent();
        std::println("IfStatement {}", FormatRange(node));

        ++indent_level_;

        PrintIndent();
        std::print("Condition: ");
        TraverseWithoutIndent(node->condition.get());

        if (node->then_branch != nullptr) {
            PrintIndent();
            std::println("Then:");
            ++indent_level_;
            Traverse(node->then_branch.get());
            --indent_level_;
        }

        if (node->else_branch != nullptr) {
            PrintIndent();
            std::println("Else:");
            ++indent_level_;
            Traverse(node->else_branch.get());
            --indent_level_;
        }

        --indent_level_;
    }

    void AstDumper::VisitForStatement(ForStatementNode* node) {
        PrintIndent();
        std::println("ForStatement {}", FormatRange(node));

        if (node->init != nullptr) {
            PrintIndent();
            std::print("Init: ");
            TraverseWithoutIndent(node->init.get());
        }

        if (node->condition != nullptr) {
            PrintIndent();
            std::print("Condition: ");
            TraverseWithoutIndent(node->condition.get());
        }

        if (node->iteration != nullptr) {
            PrintIndent();
            std::print("Iteration: ");
            TraverseWithoutIndent(node->iteration.get());
        }

        if (node->body != nullptr) {
            PrintIndent();
            std::println("Body:");
            ++indent_level_;
            Traverse(node->body.get());
            --indent_level_;
        }
    }

    void AstDumper::VisitWhileStatement(WhileStatementNode* node) {
        PrintIndent();
        std::println("WhileStatement {}", FormatRange(node));

        ++indent_level_;

        if (node->condition != nullptr) {
            PrintIndent();
            std::print("Condition: ");
            TraverseWithoutIndent(node->condition.get());
        }

        if (node->body != nullptr) {
            PrintIndent();
            std::println("Body:");
            ++indent_level_;
            Traverse(node->body.get());
            --indent_level_;
        }

        --indent_level_;
    }

    void AstDumper::VisitDoStatement(DoStatementNode* node) {
        PrintIndent();
        std::println("DoStatement {}", FormatRange(node));

        ++indent_level_;

        if (node->body != nullptr) {
            PrintIndent();
            std::println("Body:");
            ++indent_level_;
            Traverse(node->body.get());
            --indent_level_;
        }

        if (node->condition != nullptr) {
            PrintIndent();
            std::print("Condition: ");
            TraverseWithoutIndent(node->condition.get());
        }

        --indent_level_;
    }

    void AstDumper::VisitSwitchStatement(SwitchStatementNode* node) {
        PrintIndent();
        std::println("SwitchStatement {}", FormatRange(node));

        ++indent_level_;

        if (node->condition != nullptr) {
            PrintIndent();
            std::print("Condition: ");
            TraverseWithoutIndent(node->condition.get());
        }

        for (const auto& case_node : node->cases) {
            Traverse(case_node.get());
        }

        --indent_level_;
    }

    void AstDumper::VisitCaseStatement(CaseStatementNode* node) {
        PrintIndent();
        std::println("CaseStatement {}", FormatRange(node));

        ++indent_level_;

        if (node->condition != nullptr) {
            PrintIndent();
            std::print("Condition: ");
            TraverseWithoutIndent(node->condition.get());
        } else {
            PrintIndent();
            std::println("Default:");
        }

        for (const auto& statement : node->body) {
            Traverse(statement.get());
        }

        --indent_level_;
    }

    void AstDumper::VisitReturnStatement(ReturnStatementNode* node) {
        PrintIndent();
        std::println("ReturnStatement {}", FormatRange(node));

        ++indent_level_;
        Base::VisitReturnStatement(node);
        --indent_level_;
    }

    void AstDumper::VisitBreakStatement(BreakStatementNode* node) {
        PrintIndent();
        std::println("BreakStatement {}", FormatRange(node));
    }

    void AstDumper::VisitContinueStatement(ContinueStatementNode* node) {
        PrintIndent();
        std::println("ContinueStatement {}", FormatRange(node));
    }

    void AstDumper::VisitDiscardStatement(DiscardStatementNode* node) {
        PrintIndent();
        std::println("DiscardStatement {}", FormatRange(node));
    }

    void AstDumper::VisitExpressionStatement(ExpressionStatementNode* node) {
        PrintIndent();
        std::println("ExpressionStatement {}", FormatRange(node));

        ++indent_level_;
        Base::VisitExpressionStatement(node);
        --indent_level_;
    }

    void AstDumper::VisitNullStatement(NullStatementNode* node) {
        PrintIndent();
        std::println("NullStatement {}", FormatRange(node));
    }

    void AstDumper::VisitInitializerListExpression(InitializerListExpressionNode* node) {
        PrintIndent();
        std::println("InitListExpression (Elements: {}) {}", node->elements.size(), FormatRange(node));

        ++indent_level_;
        Base::VisitInitializerListExpression(node);
        --indent_level_;
    }

    void AstDumper::VisitBinaryExpression(BinaryExpressionNode* node) {
        PrintIndent();
        std::println("BinaryExpression '{}' {}", magic_enum::enum_name(node->op), FormatRange(node));

        ++indent_level_;

        if (node->left != nullptr) {
            PrintIndent();
            std::print("Left: ");
            TraverseWithoutIndent(node->left.get());
        }

        if (node->right != nullptr) {
            PrintIndent();
            std::print("Right: ");
            TraverseWithoutIndent(node->right.get());
        }

        --indent_level_;
    }

    void AstDumper::VisitUnaryExpression(UnaryExpressionNode* node) {
        PrintIndent();

        auto op_name = magic_enum::enum_name(node->op);
        if (node->is_postfix) {
            std::println("UnaryExpression (Postfix) '{}' {}", op_name, FormatRange(node));
        } else {
            std::println("UnaryExpression (Prefix) '{}' {}", op_name, FormatRange(node));
        }

        ++indent_level_;

        if (node->operand != nullptr) {
            PrintIndent();
            std::print("Operand: ");
            TraverseWithoutIndent(node->operand.get());
        }

        --indent_level_;
    }

    void AstDumper::VisitTernaryExpression(TernaryExpressionNode* node) {
        PrintIndent();
        std::println("TernaryExpression {}", FormatRange(node));

        ++indent_level_;

        if (node->condition != nullptr) {
            PrintIndent();
            std::print("Condition: ");
            TraverseWithoutIndent(node->condition.get());
        }

        if (node->true_expr != nullptr) {
            PrintIndent();
            std::println("TrueExpression:");
            ++indent_level_;
            Traverse(node->true_expr.get());
            --indent_level_;
        }

        if (node->false_expr != nullptr) {
            PrintIndent();
            std::println("FalseExpression:");
            ++indent_level_;
            Traverse(node->false_expr.get());
            --indent_level_;
        }

        --indent_level_;
    }

    void AstDumper::VisitCallExpression(CallExpressionNode* node) {
        PrintIndent();
        std::println("CallExpression {}", FormatRange(node));

        ++indent_level_;

        if (node->callee != nullptr) {
            PrintIndent();
            std::print("Callee: ");
            TraverseWithoutIndent(node->callee.get());
        }

        if (!node->args.empty()) {
            PrintIndent();
            std::println("Arguments:");
            ++indent_level_;
            for (const auto& arg : node->args) {
                Traverse(arg.get());
            }
            --indent_level_;
        }

        --indent_level_;
    }

    void AstDumper::VisitIndexExpression(IndexExpressionNode* node) {
        PrintIndent();
        std::println("IndexExpression {}", FormatRange(node));

        ++indent_level_;

        PrintIndent();
        std::print("Base: ");
        TraverseWithoutIndent(node->base.get());

        PrintIndent();
        std::print("Index: ");
        if (node->index != nullptr) {
            TraverseWithoutIndent(node->index.get());
        } else {
            std::println("<null>");
        }

        --indent_level_;
    }

    void AstDumper::VisitVariableExpression(VariableExpressionNode* node) {
        PrintIndent();

        const auto& typename_token = node->evaluated_type.typename_token;
        std::string type;
        if (node->evaluated_type.typename_token.type == TokenType::kUnknown) {
            type = magic_enum::enum_name(node->original_token.type);
        } else {
            type = node->evaluated_type.typename_token.text;
            for (auto array_size : node->evaluated_type.array_sizes) {
                std::format_to(std::back_inserter(type), "[{}]", array_size);
            }
        }

        std::println("VariableExpression '{}' Type: {} {}", node->name, type, FormatRange(node));
    }

    void AstDumper::VisitRawExpression(RawExpressionNode* node) {
        PrintIndent();
        std::string snippet;
        for (std::size_t i = 0; i != node->tokens.size(); ++i) {
            snippet += std::string(node->tokens[i].text);
        }

        std::println("RawExpression '{}' {}", snippet, FormatRange(node));
    }

    void AstDumper::VisitMemberAccessExpression(MemberAccessExpressionNode* node) {
        PrintIndent();
        std::println("MemberAccessExpression '[object].[member]' {}", FormatRange(node));

        ++indent_level_;

        if (node->object != nullptr) {
            PrintIndent();
            std::print("Object: ");
            TraverseWithoutIndent(node->object.get());
        }

        if (node->member != nullptr) {
            PrintIndent();
            std::print("Member: ");
            TraverseWithoutIndent(node->member.get());
        }

        --indent_level_;
    }

    std::string AstDumper::FormatRange(AstNode* node) const {
        return std::format("[{}:{}-{}:{}]", node->begin.line, node->begin.column, node->end.line, node->end.column);
    }

    std::string AstDumper::TypeToString(TypeSpecifier& type_spec) const {
        std::string result;
        for (auto i = 0uz; i != type_spec.specifiers.size(); ++i) {
            const auto& specifier = type_spec.specifiers[i];
            if (i + 1 != type_spec.specifiers.size()) {
                result += std::string(specifier.text) + " ";
            } else {
                result += specifier.text;
            }
        }

        // TODO: modify to evaluate
        for (const auto& array_size : type_spec.array_sizes) {
            std::string array_dimension;
            if (array_size->kind() == AstNodeKind::kLiteralExpression) {
                const auto* raw_node = static_cast<const RawExpressionNode*>(array_size.get());
                for (const auto& token : raw_node->tokens) {
                    array_dimension += token.text;
                }
            } else if (array_size->kind() == AstNodeKind::kVariableExpression) {
                const auto* var_expr = static_cast<const VariableExpressionNode*>(array_size.get());
                array_dimension = var_expr->name;
            }

            std::format_to(std::back_inserter(result), "[{}]", array_dimension);
        }

        return result.empty() ? "void" : result;
    }

    void AstDumper::PrintIndent() {
        if (suspend_indent_) {
            suspend_indent_ = false;
            return;
        }

        utils::PrintIndent(indent_level_);
    }

    void AstDumper::TraverseWithoutIndent(auto* node) {
        suspend_indent_ = true;
        Traverse(node);
    }
}
