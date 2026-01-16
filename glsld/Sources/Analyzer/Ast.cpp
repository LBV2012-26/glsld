#include "stdafx.h"
#include "Ast.hpp"

#include <format>
#include <magic_enum/magic_enum_all.hpp>
#include "Utils/Utils.hpp"

namespace glsld {
    namespace {
        std::string FormatRange(const AstNode* node) {
            return std::format("[{}:{}-{}:{}]", node->begin.line, node->begin.column, node->end.line, node->end.column);
        }

        std::string TypeToString(const TypeSpecifier& spec) {
            std::string result;
            for (const auto& qual : spec.qualifiers) {
                result += std::string(qual.text) + " ";
            }

            return result.empty() ? "void" : result;
        }
    }

    void DumpAst(const AstNode* node, int indent) {
        if (node == nullptr) {
            utils::PrintIndent(indent);
            std::println("<null>");
            return;
        }

        utils::PrintIndent(indent);

        switch (node->kind()) {
        case AstNodeKind::kTranslationUnit: {
            const auto* this_node = static_cast<const TranslationUnitNode*>(node);
            std::println("TranslationUnit {}", FormatRange(this_node));

            for (const auto& stmt : this_node->stmts) {
                DumpAst(stmt.get(), indent + 1);
            }

            break;
        }

        case AstNodeKind::kDeclarationGroup: {
            const auto* this_node = static_cast<const DeclarationGroupNode*>(node);
            std::println("DeclarationGroup {}", FormatRange(this_node));

            for (const auto& decl : this_node->decls) {
                DumpAst(decl.get(), indent + 1);
            }

            break;
        }

        case AstNodeKind::kPreprocessor: {
            const auto* this_node = static_cast<const PreprocessorNode*>(node);
            std::println("Preprocessor #{} (Tokens: {}) {}", this_node->directive, this_node->tokens.size(), FormatRange(this_node));
            break;
        }

        case AstNodeKind::kFunctionDecl: {
            const auto* this_node = static_cast<const FunctionDeclarationNode*>(node);
            std::string name = this_node->declared_symbol ? this_node->declared_symbol->name : "unnamed";
            std::println("FunctionDeclaration '{}' {}", name, FormatRange(this_node));

            utils::PrintIndent(indent + 1);
            std::println("Parameters:");
            for (const auto& param : this_node->params) {
                DumpAst(param.get(), indent + 2);
            }

            if (this_node->body) {
                utils::PrintIndent(indent + 1);
                std::println("Body:");
                DumpAst(this_node->body.get(), indent + 2);
            }

            break;
        }

        case AstNodeKind::kVariableDecl: {
            const auto* this_node = static_cast<const VariableDeclarationNode*>(node);
            std::string name = this_node->declared_symbol ? this_node->declared_symbol->name : "<anon>";
            std::println("Variable '{}' Type: {} {}", name, TypeToString(this_node->type_spec), FormatRange(this_node));

            if (this_node->array_size) {
                utils::PrintIndent(indent + 1);
                std::print("ArraySize: ");
                DumpAst(this_node->array_size.get(), 0);
            }

            if (this_node->init) {
                utils::PrintIndent(indent + 1);
                std::print("Initializer: ");
                DumpAst(this_node->init.get(), 0);
            }

            break;
        }

        case AstNodeKind::kInterfaceDecl: {
            const auto* this_node = static_cast<const InterfaceDeclarationNode*>(node);
            std::string name = this_node->declared_symbol ? this_node->declared_symbol->name : "<anon>";
            std::println("InterfaceDeclaration '{}' {}", name, FormatRange(this_node));

            if (this_node->body) {
                utils::PrintIndent(indent + 1);
                std::println("Body:");
                DumpAst(this_node->body.get(), indent + 2);
            }

            if (this_node->instances) {
                utils::PrintIndent(indent + 1);
                std::println("Instances:");
                DumpAst(this_node->instances.get(), indent + 2);
            }

            break;
        }

        case AstNodeKind::kStructDecl: {
            const auto* this_node = static_cast<const StructDeclarationNode*>(node);
            std::string name = this_node->declared_symbol ? this_node->declared_symbol->name : "<anon>";
            std::println("StructDeclaration '{}' {}", name, FormatRange(this_node));

            if (this_node->body) {
                utils::PrintIndent(indent + 1);
                std::println("Body:");
                DumpAst(this_node->body.get(), indent + 2);
            }

            if (this_node->instances) {
                utils::PrintIndent(indent + 1);
                std::println("Instances:");
                DumpAst(this_node->instances.get(), indent + 2);
            }

            break;
        }

        case AstNodeKind::kCompoundStmt: {
            const auto* this_node = static_cast<const CompoundStatementNode*>(node);
            std::println("CompoundStatement (Scope: 0x{:X}) {}",
                         reinterpret_cast<uintptr_t>(this_node->scope), FormatRange(this_node));

            for (const auto& child : this_node->children) {
                DumpAst(child.get(), indent + 1);
            }

            break;
        }

        case AstNodeKind::kIfStmt: {
            const auto* this_node = static_cast<const IfStatementNode*>(node);
            std::println("IfStatement {}", FormatRange(this_node));

            utils::PrintIndent(indent + 1);
            std::print("Condition: ");
            DumpAst(this_node->condition.get(), 0);

            utils::PrintIndent(indent + 1);
            std::println("Then:");
            DumpAst(this_node->then_branch.get(), indent + 2);

            if (this_node->else_branch) {
                utils::PrintIndent(indent + 1);
                std::println("Else:");
                DumpAst(this_node->else_branch.get(), indent + 2);
            }

            break;
        }

        case AstNodeKind::kForStmt: {
            const auto* this_node = static_cast<const ForStatementNode*>(node);
            std::println("ForStatement {}", FormatRange(this_node));

            if (this_node->init) {
                utils::PrintIndent(indent + 1);
                std::print("Init: ");
                DumpAst(this_node->init.get(), 0);
            }

            if (this_node->condition) {
                utils::PrintIndent(indent + 1);
                std::print("Condition: ");
                DumpAst(this_node->condition.get(), 0);
            }

            if (this_node->iteration) {
                utils::PrintIndent(indent + 1);
                std::print("Iteration: ");
                DumpAst(this_node->iteration.get(), 0);
            }

            if (this_node->body) {
                utils::PrintIndent(indent + 1);
                std::println("Body:");
                DumpAst(this_node->body.get(), indent + 2);
            }

            break;
        }

        case AstNodeKind::kWhileStmt: {
            const auto* this_node = static_cast<const WhileStatementNode*>(node);
            std::println("WhileStatement {}", FormatRange(this_node));

            if (this_node->condition) {
                utils::PrintIndent(indent + 1);
                std::print("Condition: ");
                DumpAst(this_node->condition.get(), 0);
            }

            if (this_node->body) {
                utils::PrintIndent(indent + 1);
                std::println("Body:");
                DumpAst(this_node->body.get(), indent + 2);
            }

            break;
        }

        case AstNodeKind::kDoStmt: {
            const auto* this_node = static_cast<const DoStatementNode*>(node);
            std::println("DoStatement {}", FormatRange(this_node));

            if (this_node->body) {
                utils::PrintIndent(indent + 1);
                std::println("Body:");
                DumpAst(this_node->body.get(), indent + 2);
            }

            if (this_node->condition) {
                utils::PrintIndent(indent + 1);
                std::print("Condition: ");
                DumpAst(this_node->condition.get(), 0);
            }

            break;
        }

        case AstNodeKind::kSwitchStmt: {
            const auto* this_node = static_cast<const SwitchStatementNode*>(node);
            std::println("SwitchStatement {}", FormatRange(this_node));

            if (this_node->condition) {
                utils::PrintIndent(indent + 1);
                std::print("Condition: ");
                DumpAst(this_node->condition.get(), 0);
            }

            for (const auto& case_node : this_node->cases) {
                DumpAst(case_node.get(), indent + 1);
            }

            break;
        }

        case AstNodeKind::kCaseStmt: {
            const auto* this_node = static_cast<const CaseStatementNode*>(node);
            std::println("CaseStatement {}", FormatRange(this_node));

            if (this_node->condition) {
                utils::PrintIndent(indent + 1);
                std::print("Condition: ");
                DumpAst(this_node->condition.get(), 0);
            } else {
                utils::PrintIndent(indent + 1);
                std::println("Default:");
            }

            for (const auto& stmt : this_node->body) {
                DumpAst(stmt.get(), indent + 1);
            }

            break;
        }

        case AstNodeKind::kReturnStmt: {
            const auto* this_node = static_cast<const ReturnStatementNode*>(node);
            std::println("ReturnStatement {}", FormatRange(this_node));
            if (this_node->return_value) {
                DumpAst(this_node->return_value.get(), indent + 1);
            }

            break;
        }

        case AstNodeKind::kBreakStmt: {
            std::println("BreakStatement {}", FormatRange(node));
            break;
        }

        case AstNodeKind::kContinueStmt: {
            std::println("ContinueStatement {}", FormatRange(node));
            break;
        }

        case AstNodeKind::kDiscardStmt: {
            std::println("DiscardStatement {}", FormatRange(node));
            break;
        }

        case AstNodeKind::kExprStmt: {
            const auto* this_node = static_cast<const ExpressionStatementNode*>(node);
            std::println("ExpressionStatement {}", FormatRange(this_node));
            if (this_node->expression) {
                DumpAst(this_node->expression.get(), indent + 1);
            }

            break;
        }

        case AstNodeKind::kNullStmt: {
            std::println("NullStatement {}", FormatRange(node));
            break;
        }

        case AstNodeKind::kBinaryExpr: {
            const auto* this_node = static_cast<const BinaryExpressionNode*>(node);
            std::println("BinaryExpression '{}' {}", magic_enum::enum_name(this_node->op), FormatRange(this_node));

            if (this_node->left) {
                utils::PrintIndent(indent + 1);
                std::print("Left: ");
                DumpAst(this_node->left.get(), 0);
            }

            if (this_node->right) {
                utils::PrintIndent(indent + 1);
                std::print("Right: ");
                DumpAst(this_node->right.get(), 0);
            }

            break;
        }

        case AstNodeKind::kUnaryExpr: {
            const auto* this_node = static_cast<const UnaryExpressionNode*>(node);
            std::println("UnaryExpression '{}' {}", magic_enum::enum_name(this_node->op), FormatRange(this_node));

            if (this_node->operand) {
                utils::PrintIndent(indent + 1);
                std::print("Operand: ");
                DumpAst(this_node->operand.get(), 0);
            }

            break;
        }

        case AstNodeKind::kCallExpr: {
            const auto* this_node = static_cast<const CallExpressionNode*>(node);
            std::println("CallExpression {}", FormatRange(this_node));

            if (this_node->callee) {
                utils::PrintIndent(indent + 1);
                std::print("Callee: ");
                DumpAst(this_node->callee.get(), 0);
            }

            if (!this_node->args.empty()) {
                utils::PrintIndent(indent + 1);
                std::println("Arguments:");
                for (const auto& arg : this_node->args) {
                    DumpAst(arg.get(), indent + 2);
                }
            }

            break;
        }

        case AstNodeKind::kVariableExpr: {
            const auto* this_node = static_cast<const VariableExpressionNode*>(node);
            std::println("VariableExpression '{}' {}", this_node->name, FormatRange(this_node));
            break;
        }

        case AstNodeKind::kLiteralExpr: { // 处理你的 RawExpressionNode
            const auto* this_node = static_cast<const RawExpressionNode*>(node);
            std::string snippet;
            for (int i = 0; i < static_cast<int>(this_node->tokens.size()); ++i) {
                snippet += std::string(this_node->tokens[i].text);
            }

            std::println("RawExpression '{}' {}", snippet, FormatRange(this_node));
            break;
        }

        case AstNodeKind::kMemberAccessExpr: {
            const auto* this_node = static_cast<const MemberAccessExpressionNode*>(node);
            std::println("MemberAccessExpression '{}.{}' {}", "[object]", this_node->member_name, FormatRange(this_node));

            if (this_node->object) {
                utils::PrintIndent(indent + 1);
                std::print("Object: ");
                DumpAst(this_node->object.get(), 0);
            }

            break;
        }

        default:
            std::println("UnknownNode(KindID: {}) {}", static_cast<int>(node->kind()), FormatRange(node));
            break;
        }
    }
}
