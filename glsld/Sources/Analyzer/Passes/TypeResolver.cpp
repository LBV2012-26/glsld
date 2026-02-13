#include "stdafx.h"
#include "TypeResolver.hpp"

#include <format>
#include <variant>
#include <vector>

namespace glsld {
    namespace {
        void SeparateType(TypeInfo& type_info, bool keep_vector) {
            if (keep_vector) {
                std::string prefix;
                switch (type_info.type_desc.family) {
                case BaseFamily::kBool:
                    prefix = "b";
                    break;
                case BaseFamily::kInt:
                    prefix = std::format("i{}", type_info.type_desc.bits);
                    break;
                case BaseFamily::kUint:
                    prefix = std::format("u{}", type_info.type_desc.bits);
                    break;
                case BaseFamily::kFloat:
                    prefix = std::format("f{}", type_info.type_desc.bits);
                    break;
                default:
                    break;
                }

                type_info.typename_token.text    = std::format("{}vec{}", prefix, type_info.type_desc.vector_length);
                type_info.type_desc.vector_count = 1;
            } else {
                switch (type_info.type_desc.family) {
                case BaseFamily::kBool:
                    type_info.typename_token.text = "bool";
                    break;
                case BaseFamily::kInt:
                    type_info.typename_token.text = std::format("int{}_t", type_info.type_desc.bits);
                    break;
                case BaseFamily::kUint:
                    type_info.typename_token.text = std::format("uint{}_t", type_info.type_desc.bits);
                    break;
                case BaseFamily::kFloat:
                    type_info.typename_token.text = std::format("float{}_t", type_info.type_desc.bits);
                    break;
                default:
                    break;
                }

                type_info.type_desc.vector_count  = 1;
                type_info.type_desc.vector_length = 1;
            }
        }

        TypeInfo GetCanonicalTypeInfo(const TypeInfo& base_type) {
            if (base_type.type_desc.family == BaseFamily::kUnknown ||
                base_type.type_desc.family == BaseFamily::kVoid ||
                base_type.type_desc.family == BaseFamily::kOpaque)
            {
                return base_type;
            }

            TypeInfo canonical_info = base_type;

            if (base_type.type_desc.is_matrix()) {
                SeparateType(canonical_info, true);
            } else {
                SeparateType(canonical_info, false);
            }

            return canonical_info;
        }
    }

    TypeResolver::TypeResolver(const DocumentSymbols& symbols, BindingMap& bindings)
        : symbols_{ symbols }
        , bindings_{ bindings }
    {}

    void TypeResolver::VisitFunctionDeclaration(FunctionDeclarationNode* node) {
        if (node->declared_symbol == nullptr) {
            return;
        }

        auto* function_symbol = node->declared_symbol;
        bindings_.try_emplace(function_symbol->location, function_symbol);
        function_symbol->type_info = ExtractTypeInfo(node->type_spec);

        function_symbol->param_typeinfos.clear();
        for (auto& param_node : node->params) {
            VisitVariableDeclaration(param_node.get());

            TypeInfo param_typeinfo;
            if (param_node->declared_symbol != nullptr) {
                param_typeinfo = param_node->declared_symbol->type_info;
            } else {
                param_typeinfo = ExtractTypeInfo(param_node->type_spec);
            }

            function_symbol->param_typeinfos.push_back(param_typeinfo);
        }

        if (node->body != nullptr) {
            Traverse(node->body.get());
        }
    }

    void TypeResolver::VisitVariableDeclaration(VariableDeclarationNode* node) {
        if (node->declared_symbol == nullptr) {
            return;
        }

        auto* variable_symbol = node->declared_symbol;
        bindings_.try_emplace(variable_symbol->location, variable_symbol);
        variable_symbol->type_info = ExtractTypeInfo(node->type_spec);

        AstVisitor::VisitVariableDeclaration(node);
    }

    void TypeResolver::VisitInterfaceDeclaration(InterfaceDeclarationNode* node) {
        if (node->declared_symbol != nullptr) {
            bindings_.try_emplace(node->declared_symbol->location, node->declared_symbol);
        }

        AstVisitor::VisitInterfaceDeclaration(node);
    }

    void TypeResolver::VisitStructDeclaration(StructDeclarationNode* node) {
        if (node->declared_symbol != nullptr) {
            bindings_.try_emplace(node->declared_symbol->location, node->declared_symbol);
        }

        AstVisitor::VisitStructDeclaration(node);
    }

    void TypeResolver::VisitIndexExpression(IndexExpressionNode* node) {
        Traverse(node->base.get());

        const auto& base_type = node->base->evaluated_type;
        auto& evaluated_type = node->evaluated_type;

        if (base_type.is_array()) {
            evaluated_type = base_type;
            evaluated_type.array_sizes.erase(evaluated_type.array_sizes.begin());
        } else if (base_type.type_desc.vector_length > 1) {
            evaluated_type = GetCanonicalTypeInfo(base_type);
        } else {
            evaluated_type = base_type;
        }
    }

    void TypeResolver::VisitVariableExpression(VariableExpressionNode* node) {
        if (node->name == "true" || node->name == "false") {
            TypeInfo info{
                .typename_token = Token{
                    .text = "bool",
                    .type = TokenType::kPrimitive
                }
            };

            node->evaluated_type = info;
            return;
        }

        if (std::holds_alternative<const SymbolInfo*>(node->linked_symbols)) {
            if (const auto* symbol = std::get<const SymbolInfo*>(node->linked_symbols)) {
                node->evaluated_type = symbol->type_info; // 根据指向的符号类型推导当前符号类型
            }
        }
    }

    void TypeResolver::VisitRawExpression(RawExpressionNode* node) {
        if (!node->tokens.empty()) {
            // 看第一个就够
            node->evaluated_type = SniffLiteralType(node->tokens.front());
        }
    }

    void TypeResolver::VisitMemberAccessExpression(MemberAccessExpressionNode* node) {
        // object.member
        Traverse(node->object.get()); // 递归推导对象类型

        auto object_type = node->object->evaluated_type;
        const auto* block_symbol = object_type.block_symbol;

        if (block_symbol != nullptr  && block_symbol->internal_scope != nullptr) {
            const auto* member_symbol = block_symbol->internal_scope->FindSymbol(node->member->name);

            if (member_symbol != nullptr) {
                node->member->linked_symbols = member_symbol;
                node->evaluated_type = member_symbol->type_info;

                bindings_.try_emplace(node->member->begin, member_symbol);
            }
        } else if (object_type.is_builtin()) {
            node->evaluated_type = ResolveSwizzleType(object_type, node->member->name);
        }
    }

    TypeInfo TypeResolver::ExtractTypeInfo(const TypeSpecifier& type_spec) {
        TypeInfo info;

        const auto& typename_token = type_spec.typename_token();
        info.typename_token = typename_token;
        info.array_sizes.clear();

        for (const auto& size : type_spec.array_sizes) {
            if (size == nullptr) {
                continue;
            }

            if (size->kind() == AstNodeKind::kLiteralExpression) {
                const auto* raw_node = static_cast<const RawExpressionNode*>(size.get());
                for (const auto& token : raw_node->tokens) {
                    info.array_sizes.push_back(token);
                }
            } else {
                const auto* var_expr = static_cast<const VariableExpressionNode*>(size.get());
                info.array_sizes.push_back(var_expr->evaluated_type.typename_token);

                if (std::holds_alternative<const SymbolInfo*>(var_expr->linked_symbols)) {
                    const auto* size_symbol = std::get<const SymbolInfo*>(var_expr->linked_symbols);
                    bindings_.try_emplace(var_expr->begin, size_symbol);
                }
            }
        }

        if (typename_token.type == TokenType::kIdentifier) {
            auto* type_symbol = current_scope_->FindSymbol(typename_token.text);
            bool  is_block    = type_symbol->kind == SymbolKind::kInterface
                             || type_symbol->kind == SymbolKind::kStruct;

            if (type_symbol != nullptr && is_block) {
                info.block_symbol = type_symbol;
                bindings_.try_emplace(typename_token.location, type_symbol);
            }
        }

        info.type_desc = ParseTypeDescriptor(typename_token.text);

        return info;
    }

    TypeDescriptor TypeResolver::ParseTypeDescriptor(std::string_view text) {
        static const std::vector<std::string> kOpaquePrefix{
            "sampler", "isampler", "usampler",
            "image", "iimage", "uimage",
            "texture", "shadow",
            "subpass", "isubpass", "usubpass",
            "accelerationStructure", "rayQuery", "rayPayload",
            "hitAttribute", "callableData", "shaderRecord",
            "atomic"
        };

        TypeDescriptor desc;

        for (const auto& prefix : kOpaquePrefix) {
            if (text.find(prefix) != std::string_view::npos) {
                desc.family = BaseFamily::kOpaque;
                return desc;
            }
        }

        if (text == "bool")
            return { BaseFamily::kBool,  0,  1, 1 };
        if (text == "int")
            return { BaseFamily::kInt,   32, 1, 1 };
        if (text == "uint")
            return { BaseFamily::kUint,  32, 1, 1 };
        if (text == "float")
            return { BaseFamily::kFloat, 32, 1, 1 };
        if (text == "double")
            return { BaseFamily::kFloat, 64, 1, 1 };
        if (text == "int8_t")
            return { BaseFamily::kInt,   8,  1, 1 };
        if (text == "int16_t")
            return { BaseFamily::kInt,   16, 1, 1 };
        if (text == "int32_t")
            return { BaseFamily::kInt,   32, 1, 1 };
        if (text == "int64_t")
            return { BaseFamily::kInt,   64, 1, 1 };
        if (text == "uint8_t")
            return { BaseFamily::kUint,  8,  1, 1 };
        if (text == "uint16_t")
            return { BaseFamily::kUint,  16, 1, 1 };
        if (text == "uint32_t")
            return { BaseFamily::kUint,  32, 1, 1 };
        if (text == "uint64_t")
            return { BaseFamily::kUint,  64, 1, 1 };
        if (text == "float16_t")
            return { BaseFamily::kFloat, 16, 1, 1 };
        if (text == "float32_t")
            return { BaseFamily::kFloat, 32, 1, 1 };
        if (text == "float64_t")
            return { BaseFamily::kFloat, 64, 1, 1 };

        std::size_t vec_pos = text.find("vec");
        std::size_t mat_pos = text.find("mat");
        bool is_vector = (vec_pos != std::string_view::npos);
        bool is_matrix = (mat_pos != std::string_view::npos);

        std::string_view prefix;
        if (is_matrix) {
            prefix = text.substr(0, mat_pos);
        } else {
            prefix = text.substr(0, vec_pos);
        }

        desc.family = BaseFamily::kFloat;
        desc.bits   = 32;

        if (prefix.empty()) {
            // vec2, mat4 -> float32
        } else if (prefix == "b") {
            desc.family = BaseFamily::kBool;
        } else if (prefix == "i") {
            desc.family = BaseFamily::kInt;
        } else if (prefix == "u") {
            desc.family = BaseFamily::kUint;
        } else if (prefix == "d") {
            desc.family = BaseFamily::kFloat;
            desc.bits   = 64;
        } else if (prefix == "h") {
            desc.family = BaseFamily::kFloat;
            desc.bits   = 16;
        } else if (prefix == "f") {
            // such as default;
        } else { // 带数字的
            if (prefix.rfind("f", 0) == 0) {
                desc.family = BaseFamily::kFloat;
            } else if (prefix.rfind("i", 0) == 0) {
                desc.family = BaseFamily::kInt;
            } else if (prefix.rfind("u", 0) == 0) {
                desc.family = BaseFamily::kUint;
            } else {
                return { BaseFamily::kUnknown };
            }

            std::size_t num_start = (prefix[0] == 'f' || prefix[0] == 'i' || prefix[0] == 'u') ? 1 : 0;
            int bits = 0;
            std::from_chars(prefix.data() + num_start, prefix.data() + prefix.size(), bits);

            if (bits > 0) {
                desc.bits = bits;
            }
        }

        std::string_view suffix;
        if (is_matrix) {
            suffix = text.substr(mat_pos + 3);
        } else {
            suffix = text.substr(vec_pos + 3);
        }

        if (suffix.empty()) {
            return { BaseFamily::kUnknown };
        }

        if (auto x_pos = suffix.find('x'); x_pos != std::string_view::npos) {
            if (x_pos > 0 && x_pos + 1 < suffix.size()) {
                desc.vector_count  = suffix[x_pos - 1] - '0';
                desc.vector_length = suffix[x_pos + 1] - '0';
            }
        } else {
            int dimension = suffix[0] - '0';
            desc.vector_length = dimension; // vector lengths or matrix rows
            if (is_matrix) {
                desc.vector_count = dimension;
            } else {
                desc.vector_count = 1;
            }
        }

        return desc;
    }

    TypeInfo TypeResolver::SniffLiteralType(const Token& token) {
        if (token.type == TokenType::kNumberLiteral) {
            std::string_view text = token.text;

            if (text.ends_with("lf") || text.ends_with("LF") || text.ends_with("Lf") || text.ends_with("lF")) {
                return TypeInfo{
                    .typename_token = Token{
                        .text = "double",
                        .type = TokenType::kPrimitive
                    }
                };
            }

            if (text.find('.') != std::string_view::npos ||
                text.find('e') != std::string_view::npos ||
                text.find('E') != std::string_view::npos ||
                text.ends_with('f') || text.ends_with('F'))
            {
                return TypeInfo{
                    .typename_token = Token{
                        .text = "float",
                        .type = TokenType::kPrimitive
                    }
                };
            }

            if (text.find('.') == std::string_view::npos) {
                if (text.ends_with('u') || text.ends_with('U')) {
                    return TypeInfo{
                        .typename_token = Token{
                            .text = "uint",
                            .type = TokenType::kPrimitive
                        }
                    };
                } else {
                    return TypeInfo{
                        .typename_token = Token{
                            .text = "int",
                            .type = TokenType::kPrimitive
                        }
                    };
                }
            }
        }

        if (token.type == TokenType::kPrimitive) {
            if (token.text == "true" || token.text == "false") {
                return TypeInfo{
                    .typename_token = Token{
                        .text = "bool",
                        .type = TokenType::kPrimitive
                    }
                };
            }
        }

        return TypeInfo{
            .typename_token = Token{
                .text = "unknown",
                .type = TokenType::kUnknown
            }
        };
    }

    TypeInfo TypeResolver::ResolveSwizzleType(const TypeInfo& base_type, std::string_view swizzle) {
        if (base_type.type_desc.is_matrix() || base_type.type_desc.family == BaseFamily::kUnknown) {
            return base_type;
        }

        TypeInfo result = base_type;
        result.type_desc.vector_count  = 1;
        result.type_desc.vector_length = static_cast<int>(swizzle.size());

        if (result.type_desc.vector_length > 1) {
            SeparateType(result, true);
        } else {
            SeparateType(result, false);
        }
        
        return result;
    }
}
