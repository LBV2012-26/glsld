#include "stdafx.h"
#include "TypeResolver.hpp"

#include <cstddef>
#include <charconv>
#include <format>
#include <ranges>
#include <stdexcept>
#include <utility>
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

        enum class MatchGrade {
            kFailed      = 0,
            kTypeUpgrade = 1,
            kBitsUpgrade = 2,
            kExactMatch  = 3
        };

        MatchGrade TryImplicityConvert(const TypeInfo& from, const TypeInfo& to) {
            if (from.typename_token.text == "unknown" || to.typename_token.text == "unknown" ||
                from.typename_token.text == ""        || to.typename_token.text == "")
            {
                return MatchGrade::kFailed;
            }

            if (from == to) {
                throw std::logic_error("This path should not be reached since exact matches are handled separately.");
            }

            if (from.is_array() != to.is_array() ||
                from.block_symbol != nullptr || to.block_symbol != nullptr)
            {
                return MatchGrade::kFailed;
            }

            const auto& from_desc = from.type_desc;
            const auto& to_desc   = to.type_desc;

            if (from_desc.family == BaseFamily::kOpaque  || to_desc.family == BaseFamily::kOpaque  ||
                from_desc.family == BaseFamily::kUnknown || to_desc.family == BaseFamily::kUnknown ||
                from_desc.family == BaseFamily::kBool    || to_desc.family == BaseFamily::kBool)
            {
                return MatchGrade::kFailed;
            }

            if (from_desc.vector_count  != to_desc.vector_count  ||
                from_desc.vector_length != to_desc.vector_length ||
                from_desc.is_matrix()   != to_desc.is_matrix())
            {
                return MatchGrade::kFailed;
            }

            if (from_desc == to_desc) {
                return MatchGrade::kExactMatch;
            }

            // 类型提升
            // 相同 Family，允许位宽提升
            if (from_desc.family == to_desc.family) {
                if (from_desc.bits <= to_desc.bits) {
                    return MatchGrade::kBitsUpgrade;
                } else {
                    return MatchGrade::kFailed;
                }
            }

            // int -> uint/float/double
            if (from_desc.family == BaseFamily::kInt) {
                if (to_desc.family == BaseFamily::kUint || to_desc.family == BaseFamily::kFloat) {
                    if (from_desc.bits <= to_desc.bits) {
                        return MatchGrade::kTypeUpgrade;
                    } else {
                        return MatchGrade::kFailed;
                    }
                } else {
                    return MatchGrade::kFailed;
                }
            }

            // uint -> float/double
            if (from_desc.family == BaseFamily::kUint) {
                if (to_desc.family == BaseFamily::kFloat) {
                    if (from_desc.bits <= to_desc.bits) {
                        return MatchGrade::kTypeUpgrade;
                    } else {
                        return MatchGrade::kFailed;
                    }
                } else {
                    return MatchGrade::kFailed;
                }
            }

            if (from_desc.family == BaseFamily::kFloat) {
                return MatchGrade::kFailed; // float -> double 的情况已经在位宽提升中判断
            }

            return MatchGrade::kFailed;
        }

        enum class MatchResult {
            kLhsBetter,
            kRhsBetter,
            kAmbiguous
        };

        struct CandidateScore {
            const SymbolInfo* symbol;
            std::vector<MatchGrade> param_grades;
        };

        MatchResult CompareCandidates(const CandidateScore& lhs, const CandidateScore& rhs) {
            bool lhs_better = false;
            bool rhs_better = false;

            for (auto i = 0uz; i != lhs.param_grades.size(); ++i) {
                if (lhs.param_grades[i] > rhs.param_grades[i])
                    lhs_better = true;
                if (rhs.param_grades[i] > lhs.param_grades[i])
                    rhs_better = true;
            }

            if (lhs_better && !rhs_better)
                return MatchResult::kLhsBetter;
            if (rhs_better && !lhs_better)
                return MatchResult::kRhsBetter;

            if (!lhs_better && !rhs_better) {
                if (lhs.symbol->kind == SymbolKind::kFunctionImpl && rhs.symbol->kind == SymbolKind::kFunctionDecl)
                    return MatchResult::kLhsBetter;
                if (rhs.symbol->kind == SymbolKind::kFunctionImpl && lhs.symbol->kind == SymbolKind::kFunctionDecl)
                    return MatchResult::kRhsBetter;
            }

            return MatchResult::kAmbiguous;
        }
    }

    TypeResolver::TypeResolver(const DocumentSymbols& symbols, BindingMap& bindings, int version_replica,
                               std::shared_ptr<const std::atomic<int>> version_pointer)
        : AstVisitor(version_replica, version_pointer)
        , symbols_{ symbols }
        , bindings_{ bindings }
    {}

    void TypeResolver::VisitFunctionDeclaration(FunctionDeclarationNode* node) {
        if (node->declared_symbol == nullptr) {
            return;
        }

        auto* function_symbol = node->declared_symbol;
        bindings_.try_emplace(function_symbol->location, function_symbol);
        function_symbol->type_info = ExtractTypeInfo(node->type_spec);

        const auto* block_symbol = function_symbol->type_info.block_symbol;
        if (function_symbol->type_info.block_symbol != nullptr) {
            bindings_.try_emplace(function_symbol->type_info.typename_token.location, block_symbol);
        }

        function_symbol->param_typeinfos.clear();
        for (auto& param_node : node->params) {
            VisitVariableDeclaration(param_node.get());

            TypeInfo param_typeinfo;
            if (param_node->declared_symbol != nullptr) { // 是否无参数只有类型
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

    void TypeResolver::VisitCallExpression(CallExpressionNode* node) {
        std::vector<TypeInfo> call_arg_types;
        for (const auto& arg : node->args) {
            Traverse(arg.get());
            call_arg_types.push_back(arg->evaluated_type); // 处理参数类型
        }

        Traverse(node->callee.get()); // 如果是函数列表（未确定重载），不会进行任何操作

        auto* callee_node = static_cast<VariableExpressionNode*>(node->callee.get());
        if (std::holds_alternative<SymbolList>(callee_node->linked_symbols)) {
            const auto& candidates = std::get<SymbolList>(callee_node->linked_symbols);

            auto resolved = ResolveOverload(candidates, call_arg_types);
            if (std::holds_alternative<const SymbolInfo*>(resolved)) {
                const auto* best_match = std::get<const SymbolInfo*>(resolved);
                callee_node->linked_symbols   = best_match;
                callee_node->evaluated_type   = best_match->type_info;
                node->evaluated_type          = best_match->type_info;
                bindings_[callee_node->begin] = best_match;
            } else if (std::holds_alternative<SymbolList>(resolved)) {
                callee_node->linked_symbols = std::get<SymbolList>(resolved);
            } else {
                callee_node->linked_symbols = std::monostate{};
            }
        } else if (std::holds_alternative<const SymbolInfo*>(callee_node->linked_symbols)) {
            if (const auto* symbol = std::get<const SymbolInfo*>(callee_node->linked_symbols)) {
                callee_node->evaluated_type = symbol->type_info;
                node->evaluated_type = symbol->type_info;
            } // 只有一个符号并且已经推导，直接过
        }
    }

    void TypeResolver::VisitIndexExpression(IndexExpressionNode* node) {
        Traverse(node->base.get());

        const auto& base_type = node->base->evaluated_type;
        auto& evaluated_type = node->evaluated_type;

        if (base_type.is_array()) {
            evaluated_type = base_type;
            evaluated_type.array_sizes.erase(evaluated_type.array_sizes.begin());
        } else if (base_type.type_desc.vector_length > 1) {
            evaluated_type = GetCanonicalTypeInfo(base_type); // 从向量或者矩阵中剥离子类型
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

        if (block_symbol != nullptr  && block_symbol->internal_scope != nullptr && node->member != nullptr) {
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
        if (type_spec.typename_token().type == TokenType::kUnknown) {
            return {};
        }

        TypeInfo info;

        const auto& typename_token = type_spec.typename_token();
        info.typename_token = typename_token;

        if (type_spec.specifiers.size() > 0) {
            info.qualifiers.clear();
            info.qualifiers.assign_range(type_spec.specifiers | std::views::take(type_spec.specifiers.size() - 1));
        }

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
            } else if (size->kind() == AstNodeKind::kVariableExpression) {
                const auto* var_expr = static_cast<const VariableExpressionNode*>(size.get());
                info.array_sizes.push_back(Token{
                    .text = var_expr->name,
                    .type = var_expr->token_type
                });

                if (std::holds_alternative<const SymbolInfo*>(var_expr->linked_symbols)) {
                    const auto* size_symbol = std::get<const SymbolInfo*>(var_expr->linked_symbols);
                    bindings_.try_emplace(var_expr->begin, size_symbol);
                }
            }
        }

        if (typename_token.type == TokenType::kIdentifier) {
            auto* type_symbol = current_scope_->FindSymbol(typename_token.text);
            if (type_symbol == nullptr) {
                return {};
            }

            bool is_block = type_symbol->kind == SymbolKind::kInterface
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
            if (text.contains(prefix)) {
                desc.family = BaseFamily::kOpaque;
                return desc;
            }
        }

        if (text == "bool")
            return { BaseFamily::kBool,  32, 1, 1 };
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
                return {
                    .typename_token{
                        .text = "double",
                        .type = TokenType::kPrimitive
                    },
                    .type_desc{
                        .family        = BaseFamily::kFloat,
                        .bits          = 64,
                        .vector_count  = 1,
                        .vector_length = 1
                    }
                };
            }

            if (text.ends_with("hf") || text.ends_with("HF") || text.ends_with("Hf") || text.ends_with("hF")) {
                return {
                    .typename_token{
                        .text = "half",
                        .type = TokenType::kPrimitive
                    },
                    .type_desc{
                        .family        = BaseFamily::kFloat,
                        .bits          = 16,
                        .vector_count  = 1,
                        .vector_length = 1
                    }
                };
            }

            if (text.contains('.') || text.contains('e') || text.contains('E') ||
                text.ends_with('f') || text.ends_with('F'))
            {
                return {
                    .typename_token{
                        .text = "float",
                        .type = TokenType::kPrimitive
                    },
                    .type_desc{
                        .family        = BaseFamily::kFloat,
                        .bits          = 32,
                        .vector_count  = 1,
                        .vector_length = 1
                    }
                };
            }

            if (text.find('.') == std::string_view::npos) {
                if (text.ends_with('u') || text.ends_with('U')) {
                    return {
                        .typename_token{
                            .text = "uint",
                            .type = TokenType::kPrimitive
                        },
                        .type_desc{
                            .family        = BaseFamily::kUint,
                            .bits          = 32,
                            .vector_count  = 1,
                            .vector_length = 1
                        }
                    };
                } else {
                    return {
                        .typename_token{
                            .text = "int",
                            .type = TokenType::kPrimitive
                        },
                        .type_desc{
                            .family        = BaseFamily::kInt,
                            .bits          = 32,
                            .vector_count  = 1,
                            .vector_length = 1
                        }
                    };
                }
            }
        }

        if (token.type == TokenType::kPrimitive) {
            if (token.text == "true" || token.text == "false") {
                return {
                    .typename_token{
                        .text = "bool",
                        .type = TokenType::kPrimitive
                    },
                    .type_desc{
                        .family        = BaseFamily::kBool,
                        .bits          = 32,
                        .vector_count  = 1,
                        .vector_length = 1
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

    SymbolReference TypeResolver::ResolveOverload(const SymbolList& candidates, std::span<const TypeInfo> call_arg_types) {
        std::vector<TypeInfo> normalized_call_args(call_arg_types.begin(), call_arg_types.end());
        if (normalized_call_args.empty()) {
            normalized_call_args.push_back(TypeInfo{
                .typename_token = Token{
                    .text = "void",
                    .type = TokenType::kPrimitive
                }
            });
        }

        std::vector<CandidateScore> possible_matches;
        SymbolList failed_matches;

        auto CompareTypeInfoIgnoreQualifiers = [](const TypeInfo& lhs, const TypeInfo& rhs) -> bool {
            if (lhs.typename_token.text != rhs.typename_token.text ||
                lhs.typename_token.type != rhs.typename_token.type)
            {
                return false;
            }

            if (lhs.block_symbol != rhs.block_symbol) {
                return false;
            }

            if (lhs.array_sizes.size() != rhs.array_sizes.size()) {
                return false;
            }

            for (auto i = 0uz; i != lhs.array_sizes.size(); ++i) {
                if (lhs.array_sizes[i].text != rhs.array_sizes[i].text ||
                    lhs.array_sizes[i].type != rhs.array_sizes[i].type)
                {
                    return false;
                }
            }

            return true;
        };

        for (const auto* symbol : candidates) {
            const auto& param_typeinfos = symbol->param_typeinfos;
            if (param_typeinfos.size() != normalized_call_args.size()) {
                continue;
            }

            std::vector<MatchGrade> current_grades;
            bool match_failed = false;

            for (auto i = 0uz; i != normalized_call_args.size(); ++i) {
                const auto& call_type   = normalized_call_args[i];
                const auto& target_type = param_typeinfos[i];

                if (CompareTypeInfoIgnoreQualifiers(call_type, target_type)) {
                    current_grades.push_back(MatchGrade::kExactMatch);
                } else {
                    auto match_grade = TryImplicityConvert(call_type, target_type);
                    if (match_grade != MatchGrade::kFailed) {
                        current_grades.push_back(match_grade);
                    } else {
                        match_failed = true;
                        break;
                    }
                }
            }

            if (!match_failed) {
                possible_matches.push_back(CandidateScore{
                    .symbol       = symbol,
                    .param_grades = std::move(current_grades)
                });
            } else {
                failed_matches.push_back(symbol);
            }
        }

        if (possible_matches.empty() && failed_matches.empty()) {
            return std::monostate{};
        } else if (possible_matches.empty() && !failed_matches.empty()) {
            return failed_matches; // 全都不对
        } else if (possible_matches.size() == 1) {
            return possible_matches.front().symbol; // 只有一个
        }

        std::vector<CandidateScore> best_matches;
        for (const auto& current : possible_matches) {
            if (best_matches.empty()) {
                best_matches.push_back(current);
                continue;
            }

            auto compare_result = CompareCandidates(current, best_matches.front()); // 严格偏序不存在石头剪刀布循环，直接比较第一个即可
            if (compare_result == MatchResult::kLhsBetter) {
                best_matches.clear();
                best_matches.push_back(current);
            } else if (compare_result == MatchResult::kRhsBetter) {
                // do nothing
            } else { // ambiguous
                best_matches.push_back(current);
            }
        }

        if (best_matches.size() == 1) {
            return best_matches.front().symbol;
        } else if (best_matches.empty()) {
            return std::monostate{};
        } else {
            SymbolList ambiguous_symbols;
            for (const auto& match : best_matches) {
                ambiguous_symbols.push_back(match.symbol);
            }

            return ambiguous_symbols;
        }
    }
}
