#include "stdafx.h"
#include "TypeResolver.hpp"

#include <cstddef>
#include <algorithm>
#include <charconv>
#include <format>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <variant>

#include "Analyzer/Passes/ConstantEvaluator.hpp"

namespace glsld {
    namespace {
        std::string GetTypeBitsPrefix(const TypeDescriptor& type_desc) {
            std::string prefix;
            switch (type_desc.family) {
            case BaseFamily::kBool:
                prefix = "b";
                break;
            case BaseFamily::kInt:
                prefix = std::format("i{}", type_desc.bits);
                break;
            case BaseFamily::kUint:
                prefix = std::format("u{}", type_desc.bits);
                break;
            case BaseFamily::kFloat:
                prefix = std::format("f{}", type_desc.bits);
                break;
            default:
                break;
            }

            return prefix;
        }

        std::string GetScalarTypename(const TypeDescriptor& type_desc) {
            std::string name;
            switch (type_desc.family) {
            case BaseFamily::kBool:
                name = "bool";
                break;
            case BaseFamily::kInt:
                name = std::format("int{}_t", type_desc.bits);
                break;
            case BaseFamily::kUint:
                name = std::format("uint{}_t", type_desc.bits);
                break;
            case BaseFamily::kFloat:
                name = std::format("float{}_t", type_desc.bits);
                break;
            default:
                break;
            }

            return name;
        }

        void SeparateType(TypeInfo& type_info, bool keep_vector) {
            if (keep_vector) {
                std::string prefix = GetTypeBitsPrefix(type_info.type_desc);
                type_info.typename_token.text    = std::format("{}vec{}", prefix, type_info.type_desc.vector_length);
                type_info.type_desc.vector_count = 1;
            } else {
                type_info.typename_token.text     = GetScalarTypename(type_info.type_desc);
                type_info.type_desc.vector_count  = 1;
                type_info.type_desc.vector_length = 1;
            }
        }

        TypeInfo GetCanonicalTypeInfo(const TypeDescriptor& type_desc) {
            if (type_desc.family == BaseFamily::kUnknown) {
                return {
                    .typename_token{
                        .text = "unknown",
                        .type = TokenType::kUnknown
                    },
                    .type_desc = type_desc
                };
            }

            auto arithmetic_structure = type_desc.arithmetic_structure();
            using enum TypeDescriptor::ArithmeticStructure;

            TypeInfo type_info;
            type_info.typename_token.type = TokenType::kBuiltInType;

            if (arithmetic_structure == kScalar) {
                type_info.typename_token.text = GetScalarTypename(type_desc);
            } else {
                std::string prefix = GetTypeBitsPrefix(type_desc);
                if (arithmetic_structure == kVector) {
                    type_info.typename_token.text = std::format("{}vec{}", prefix, type_desc.vector_length);
                } else {
                    type_info.typename_token.text = std::format("{}mat{}x{}", prefix, type_desc.vector_count, type_desc.vector_length);
                }
            }

            type_info.type_desc = type_desc;
            return type_info;
        }

        MatchGrade TryImplicityConvert(const TypeInfo& from, const TypeInfo& to) {
            if (from.typename_token.text == "" || to.typename_token.text == "") {
                return MatchGrade::kFailed;
            }

            if (from.typename_token.type == TokenType::kUnknown || to.typename_token.type == TokenType::kUnknown) {
                return MatchGrade::kWildcard;
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

            if (from_desc.vector_count           != to_desc.vector_count  ||
                from_desc.vector_length          != to_desc.vector_length ||
                from_desc.arithmetic_structure() != to_desc.arithmetic_structure())
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

        MatchResult CompareCandidates(const CandidateScore& lhs, const CandidateScore& rhs) {
            bool lhs_better = false;
            bool rhs_better = false;

            auto min_size = std::min(lhs.param_grades.size(), rhs.param_grades.size());

            for (auto i = 0uz; i != min_size; ++i) {
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
                if (lhs.symbol->param_typeinfos.size() < rhs.symbol->param_typeinfos.size()) {
                    return MatchResult::kLhsBetter;
                } else if (lhs.symbol->param_typeinfos.size() > rhs.symbol->param_typeinfos.size()) {
                    return MatchResult::kRhsBetter;
                }

                if (lhs.symbol->kind == SymbolKind::kFunctionImpl && rhs.symbol->kind == SymbolKind::kFunctionDecl)
                    return MatchResult::kLhsBetter;
                if (rhs.symbol->kind == SymbolKind::kFunctionImpl && lhs.symbol->kind == SymbolKind::kFunctionDecl)
                    return MatchResult::kRhsBetter;
            }

            return MatchResult::kAmbiguous;
        }
    }

    TypeResolver::TypeResolver(Document& document, int version_replica, std::shared_ptr<const std::atomic<int>> version_pointer)
        : AstVisitor(version_replica, version_pointer)
        , document_{ document }
    {}

    int TypeResolver::RankSignatureCandidates(const SymbolList& candidates, std::span<const TypeInfo> call_arg_types) {
        std::vector<TypeInfo> normalized_call_args(call_arg_types.begin(), call_arg_types.end());
        if (normalized_call_args.empty()) {
            normalized_call_args.push_back({
                .typename_token = Token{
                    .text = "void",
                    .type = TokenType::kPrimitive
                }
            });
        }

        struct ScoreResult {
            CandidateScore score;
            bool match_successful{ false };
        };

        std::vector<ScoreResult> score_results;

        for (const auto* symbol : candidates) {
            const auto& param_typeinfos = symbol->param_typeinfos;
            if (normalized_call_args.size() > param_typeinfos.size()) {
                score_results.push_back({
                    .score = {
                        .symbol = symbol,
                        .param_grades = {}
                    },
                    .match_successful = false
                });
                continue;
            }

            std::vector<MatchGrade> current_grades;

            for (auto i = 0uz; i != normalized_call_args.size(); ++i) {
                const auto& call_type   = normalized_call_args[i];
                const auto& target_type = param_typeinfos[i];

                if (call_type.CompareWithoutQualifiers(target_type)) {
                    current_grades.push_back(MatchGrade::kExactMatch);
                } else {
                    auto match_grade = TryImplicityConvert(call_type, target_type);
                    current_grades.push_back(match_grade);
                }
            }

            CandidateScore score{
                .symbol       = symbol,
                .param_grades = std::move(current_grades)
            };

            ScoreResult result{
                .score            = std::move(score),
                .match_successful = true
            };

            score_results.push_back(std::move(result));
        }

        int index = -1;
        for (auto i = 0uz; i != score_results.size(); ++i) {
            if (!score_results[i].match_successful) {
                continue;
            }

            if (index == -1) {
                index = static_cast<int>(i);
                continue;
            }

            auto compare_result = CompareCandidates(score_results[index].score, score_results[i].score);
            if (compare_result == MatchResult::kRhsBetter) {
                index = static_cast<int>(i);
            }
        }

        return index == -1 ? 0 : index;
    }

    void TypeResolver::VisitTranslationUnit(TranslationUnitNode* node) {
        is_signature_pass_ = true;
        AstVisitor::VisitTranslationUnit(node);

        is_signature_pass_ = false;
        AstVisitor::VisitTranslationUnit(node);
    }

    void TypeResolver::VisitFunctionDeclaration(FunctionDeclarationNode* node) {
        if (node->declared_symbol == nullptr) {
            return;
        }

        auto* function_symbol = node->declared_symbol;
        document_.bindings.try_emplace(function_symbol->location, function_symbol);
        function_symbol->type_info = ExtractTypeInfo(node->type_spec, node->located_scope);

        const auto* block_symbol = function_symbol->type_info.block_symbol;
        if (function_symbol->type_info.block_symbol != nullptr) {
            document_.bindings.try_emplace(function_symbol->type_info.typename_token.location, block_symbol);
        }

        function_symbol->param_typeinfos.clear();
        for (auto& param_node : node->params) {
            VisitVariableDeclaration(param_node.get());

            TypeInfo param_typeinfo;
            if (param_node->declared_symbol != nullptr) { // 是否无参数只有类型
                param_typeinfo = param_node->declared_symbol->type_info;
            } else {
                param_typeinfo = ExtractTypeInfo(param_node->type_spec, param_node->located_scope);
            }

            function_symbol->param_typeinfos.push_back(param_typeinfo);
        }

        if (is_signature_pass_) {
            return;
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
        document_.bindings.try_emplace(variable_symbol->location, variable_symbol);
        variable_symbol->type_info = ExtractTypeInfo(node->type_spec, node->located_scope);

        if (node->init == nullptr) {
            return;
        }

        Traverse(node->init.get());

        variable_symbol->type_info.array_sizes.resize(node->type_spec.array_sizes.size());
        node->init->evaluated_type.array_sizes.resize(node->type_spec.array_sizes.size());

        for (auto i = 0uz; i != node->type_spec.array_sizes.size(); ++i) {
            if (node->type_spec.array_sizes[i] == nullptr) {
                variable_symbol->type_info.array_sizes[i] = node->init->evaluated_type.array_sizes[i];
            }
        }
    }

    void TypeResolver::VisitInterfaceDeclaration(InterfaceDeclarationNode* node) {
        if (node->declared_symbol != nullptr) {
            document_.bindings.try_emplace(node->declared_symbol->location, node->declared_symbol);
        }

        AstVisitor::VisitInterfaceDeclaration(node);
    }

    void TypeResolver::VisitStructDeclaration(StructDeclarationNode* node) {
        if (node->declared_symbol != nullptr) {
            document_.bindings.try_emplace(node->declared_symbol->location, node->declared_symbol);
        }

        AstVisitor::VisitStructDeclaration(node);
    }

    void TypeResolver::VisitBinaryExpression(BinaryExpressionNode* node) {
        Traverse(node->left.get());
        Traverse(node->right.get());

        const auto& left_type  = node->left->evaluated_type;
        const auto& right_type = node->right->evaluated_type;

        if (node->left == nullptr || node->right == nullptr ||
            !left_type.is_valid() || !right_type.is_valid())
        {
            node->evaluated_type = {
                .typename_token{
                    .text = "unknown",
                    .type = TokenType::kUnknown
                }
            };

            return;
        }

        node->evaluated_type = ResolveBinaryOperationType(left_type, right_type, node->op);
    }

    void TypeResolver::VisitUnaryExpression(UnaryExpressionNode* node) {
        if (node->operand == nullptr) {
            return;
        }

        Traverse(node->operand.get());

        TypeInfo operand_type = node->operand->evaluated_type;
        if (operand_type.type_desc.family == BaseFamily::kUnknown) {
            node->evaluated_type = operand_type;
            return;
        }

        TypeDescriptor result_desc = operand_type.type_desc;

        switch (node->op) {
        case TokenType::kPlus:
        case TokenType::kMinus:
        case TokenType::kPlusPlus:
        case TokenType::kMinusMinus:
            if (operand_type.type_desc.family == BaseFamily::kBool || operand_type.type_desc.family == BaseFamily::kOpaque)
                result_desc.family = BaseFamily::kUnknown;
            break;
        case TokenType::kTilde:
            if (operand_type.type_desc.family != BaseFamily::kInt && operand_type.type_desc.family != BaseFamily::kUint)
                result_desc.family = BaseFamily::kUnknown;
            break;
        case TokenType::kExclamation:
            if (operand_type.type_desc.family != BaseFamily::kBool)
                result_desc.family = BaseFamily::kUnknown;
            break;
        default:
            result_desc.family = BaseFamily::kUnknown;
            break;
        }

        if (result_desc.family != BaseFamily::kUnknown) {
            node->evaluated_type = GetCanonicalTypeInfo(result_desc);
        } else {
            node->evaluated_type = {
                .typename_token{
                    .text = "unknown",
                    .type = TokenType::kUnknown
                }
            };
        }
    }

    void TypeResolver::VisitTernaryExpression(TernaryExpressionNode* node) {
        if (node->condition != nullptr)
            Traverse(node->condition.get());
        if (node->true_expr != nullptr)
            Traverse(node->true_expr.get());
        if (node->false_expr != nullptr)
            Traverse(node->false_expr.get());

        if (node->true_expr != nullptr && node->false_expr != nullptr) {
            const auto& true_type  = node->true_expr->evaluated_type;
            const auto& false_type = node->false_expr->evaluated_type;

            if (true_type.CompareWithoutQualifiers(false_type)) {
                node->evaluated_type = true_type;
            } else if (TryImplicityConvert(true_type, false_type) != MatchGrade::kFailed) {
                node->evaluated_type = false_type;
            } else if (TryImplicityConvert(false_type, true_type) != MatchGrade::kFailed) {
                node->evaluated_type = true_type;
            } else {
                node->evaluated_type = {
                    .typename_token{
                        .text = "unknown",
                        .type = TokenType::kUnknown
                    }
                };
            }
        }
    }

    void TypeResolver::VisitCallExpression(CallExpressionNode* node) {
        std::vector<TypeInfo> call_arg_types;
        for (const auto& arg : node->args) {
            if (arg != nullptr) {
                Traverse(arg.get());
                call_arg_types.push_back(arg->evaluated_type); // 处理参数类型
            } else {
                call_arg_types.push_back({
                    .typename_token{
                        .text = "unknown",
                        .type = TokenType::kUnknown
                    },
                });
            }
        }

        Traverse(node->callee.get());

        ExpressionNode* current_base = node->callee.get();
        std::vector<std::int64_t> dimensions;
        bool is_array_constructor = false;

        while (auto* index_node = dynamic_cast<IndexExpressionNode*>(current_base)) {
            is_array_constructor = true;

            auto size = 0z;
            if (index_node->index != nullptr) {
                ConstantEvaluator evaluator;
                auto result = evaluator.Evaluate(index_node->index.get());
                size = result.value_or(std::numeric_limits<std::int64_t>::min());
            } else {
                size = std::numeric_limits<std::int64_t>::min();
            }

            dimensions.push_back(size);
            current_base = index_node->base.get();
        }

        // int array[] = int[](...)
        if (is_array_constructor) {
            if (const auto* base_varexpr = dynamic_cast<const VariableExpressionNode*>(current_base)) {
                bool is_constructor = false;
                TypeDescriptor base_desc;

                if (base_varexpr->original_token.type == TokenType::kPrimitive ||
                    base_varexpr->original_token.type == TokenType::kBuiltInType)
                {
                    is_constructor = true;
                    base_desc = ParseTypeDescriptor(base_varexpr->name);
                } else if (std::holds_alternative<const SymbolInfo*>(base_varexpr->linked_symbols)) {
                    const auto* symbol = std::get<const SymbolInfo*>(base_varexpr->linked_symbols);
                    if (symbol != nullptr) {
                        if (symbol->kind == SymbolKind::kInterface || symbol->kind == SymbolKind::kStruct) {
                            is_constructor = true;
                            base_desc = symbol->type_info.type_desc;
                        }
                    }
                }

                if (is_constructor) {
                    TypeInfo array_type{
                        .typename_token{
                            .text = base_varexpr->name,
                            .type = base_varexpr->original_token.type
                        },
                        .type_desc = base_desc
                    };

                    std::ranges::reverse(dimensions);
                    if (std::ranges::find(dimensions, std::numeric_limits<std::int64_t>::min()) != dimensions.end()) {
                        auto dimensions_from_args = DeduceArraySizesFromArgs(node);
                        for (auto&& [target, source] : std::views::zip(dimensions, dimensions_from_args)) {
                            if (target == std::numeric_limits<std::int64_t>::min()) {
                                target = source;
                            }
                        }
                    }

                    array_type.array_sizes = std::move(dimensions);

                    node->evaluated_type         = array_type;
                    node->callee->evaluated_type = array_type;
                    return;
                }
            }
        }

        auto* callee_node = static_cast<VariableExpressionNode*>(node->callee.get());

        if (callee_node->original_token.type == TokenType::kPrimitive ||
            callee_node->original_token.type == TokenType::kBuiltInType) {
            TypeInfo constructor_type{
                .typename_token{
                    .text = callee_node->name,
                    .type = callee_node->original_token.type
                },
                .type_desc = ParseTypeDescriptor(callee_node->name)
            };

            callee_node->evaluated_type = constructor_type;
            node->evaluated_type        = constructor_type;
            return;
        }

        if (std::holds_alternative<SymbolList>(callee_node->linked_symbols)) {
            const auto& candidates = std::get<SymbolList>(callee_node->linked_symbols);

            auto resolved = ResolveOverload(candidates, call_arg_types);
            if (std::holds_alternative<const SymbolInfo*>(resolved)) {
                const auto* best_match = std::get<const SymbolInfo*>(resolved);
                callee_node->linked_symbols            = best_match;
                callee_node->evaluated_type            = best_match->type_info;
                node->evaluated_type                   = best_match->type_info;
                document_.bindings[callee_node->begin] = best_match;
            } else if (std::holds_alternative<SymbolList>(resolved)) {
                callee_node->linked_symbols = std::get<SymbolList>(resolved);
            } else {
                callee_node->linked_symbols = std::monostate{};
            }
        } else if (std::holds_alternative<const SymbolInfo*>(callee_node->linked_symbols)) {
            const auto* symbol = std::get<const SymbolInfo*>(callee_node->linked_symbols);
            if (symbol != nullptr) {
                if (symbol->kind == SymbolKind::kInterface || symbol->kind == SymbolKind::kStruct) {
                    TypeInfo constructor_type{
                        .typename_token = symbol->type_info.typename_token,
                        .type_desc      = symbol->type_info.type_desc,
                        .block_symbol   = symbol
                    };
                }

                callee_node->evaluated_type = symbol->type_info;
                node->evaluated_type = symbol->type_info;
                document_.bindings[callee_node->begin] = symbol;
            }
        }
    }

    namespace {
        TypeInfo SplitCanonicalTypeInfo(const TypeInfo& base_type) {
            if (base_type.type_desc.family == BaseFamily::kUnknown ||
                base_type.type_desc.family == BaseFamily::kVoid    ||
                base_type.type_desc.family == BaseFamily::kOpaque)
            {
                return base_type;
            }

            TypeInfo canonical_info = base_type;

            using enum TypeDescriptor::ArithmeticStructure;
            if (base_type.type_desc.arithmetic_structure() == kMatrix) {
                SeparateType(canonical_info, true);
            } else if (base_type.type_desc.arithmetic_structure() == kVector) {
                SeparateType(canonical_info, false);
            }

            return canonical_info;
        };
    }

    void TypeResolver::VisitIndexExpression(IndexExpressionNode* node) {
        Traverse(node->base.get());
        Traverse(node->index.get());

        const auto& base_type = node->base->evaluated_type;
        auto& evaluated_type  = node->evaluated_type;

        if (base_type.is_array()) {
            evaluated_type = base_type;
            evaluated_type.array_sizes.erase(evaluated_type.array_sizes.begin());
        } else if (base_type.type_desc.vector_length > 1) {
            evaluated_type = SplitCanonicalTypeInfo(base_type); // 从向量或者矩阵中剥离子类型
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
        Traverse(node->object.get());
        // Traverse(node->member.get()); SymbolLinker 不知道结构体内部作用域，遍历了也鸡毛用没有

        auto object_type = node->object->evaluated_type;
        const auto* block_symbol = object_type.block_symbol;

        if (block_symbol != nullptr  && block_symbol->internal_scope != nullptr && node->member != nullptr) {
            const auto* member_symbol = block_symbol->internal_scope->FindSymbol(node->member->name);

            if (member_symbol != nullptr) {
                node->member->linked_symbols = member_symbol;
                node->evaluated_type = member_symbol->type_info;

                document_.bindings.try_emplace(node->member->begin, member_symbol);
            }
        } else if (object_type.is_builtin()) {
            node->evaluated_type = ResolveSwizzleType(object_type, node->member->name);
        }
    }

    std::vector<std::int64_t> TypeResolver::DeduceArraySizesFromArgs(const CallExpressionNode* call_node) {
        std::vector<std::int64_t> dimensions;
        if (call_node->args.empty()) {
            return dimensions;
        }

        dimensions.push_back(static_cast<std::int64_t>(call_node->args.size()));
        const auto* first_arg = call_node->args.front().get();

        if (const auto* next_call = dynamic_cast<const CallExpressionNode*>(first_arg)) {
            if (dynamic_cast<IndexExpressionNode*>(next_call->callee.get()) != nullptr) {
                auto next_dimensions = DeduceArraySizesFromArgs(next_call);
                dimensions.append_range(next_dimensions | std::views::as_rvalue);
            }
        }

        return dimensions;
    }

    TypeInfo TypeResolver::ExtractTypeInfo(const TypeSpecifier& type_spec, const Scope* located_scope) {
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

            ConstantEvaluator evaluator;
            auto result = evaluator.Evaluate(size.get());
            info.array_sizes.push_back(result.value_or(std::numeric_limits<std::int64_t>::min()));
        }

        if (typename_token.type == TokenType::kIdentifier) {
            auto* type_symbol = located_scope->FindTypeSymbol(typename_token.text);
            if (type_symbol == nullptr) {
                return {};
            }

            info.block_symbol = type_symbol;
            document_.bindings.try_emplace(typename_token.location, type_symbol);
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
        if (base_type.type_desc.arithmetic_structure() != TypeDescriptor::ArithmeticStructure::kVector ||
            base_type.type_desc.family == BaseFamily::kUnknown)
        {
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

                if (call_type.CompareWithoutQualifiers(target_type)) {
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
                possible_matches.push_back({
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

    TypeInfo TypeResolver::ResolveBinaryOperationType(const TypeInfo& left_type, const TypeInfo& right_type, TokenType op) {
        auto IsLogicalOperator = [](TokenType op) -> bool {
            return op == TokenType::kAmpersandAmpersand
                || op == TokenType::kVerticalBarVerticalBar
                || op == TokenType::kCaretCaret;
        };

        auto IsRelationalOperator = [](TokenType op) -> bool {
            return op == TokenType::kLessThan  || op == TokenType::kGreaterThan
                || op == TokenType::kLessEqual || op == TokenType::kGreaterEqual;
        };

        auto IsEqualityOperator = [](TokenType op) -> bool {
            return op == TokenType::kEqualEqual || op == TokenType::kNotEqual;
        };

        if (IsLogicalOperator(op) || IsRelationalOperator(op) || IsEqualityOperator(op)) {
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

        auto IsAssignmentOperator = [](TokenType op) -> bool {
            return op == TokenType::kEqual          || op == TokenType::kPlusEqual       || op == TokenType::kMinusEqual ||
                   op == TokenType::kStarEqual      || op == TokenType::kSlashEqual      || op == TokenType::kPercentEqual ||
                   op == TokenType::kLeftShiftEqual || op == TokenType::kRightShiftEqual ||
                   op == TokenType::kAmpersandEqual || op == TokenType::kCaretEqual      || op == TokenType::kVerticalBarEqual;
        };

        if (IsAssignmentOperator(op)) {
            return left_type;
        }

        return ResolveArithmeticPromotion(left_type, right_type, op);
    }

    TypeInfo TypeResolver::ResolveArithmeticPromotion(const TypeInfo& left_type, const TypeInfo& right_type, TokenType op) {
        if (left_type.CompareWithoutQualifiers(right_type)) {
            return left_type;
        }

        auto left_desc  = left_type.type_desc;
        auto right_desc = right_type.type_desc;

        TypeDescriptor result_desc;
        result_desc.family = std::max(left_desc.family, right_desc.family);
        result_desc.bits   = std::max(left_desc.bits,   right_desc.bits);

        auto left_structure  = left_desc.arithmetic_structure();
        auto right_structure = right_desc.arithmetic_structure();

        using enum TypeDescriptor::ArithmeticStructure;

        if (left_structure == kMatrix || right_structure == kMatrix) {
            if (left_structure == kMatrix && right_structure == kMatrix) {
                if (op == TokenType::kStar) {
                    if (left_desc.vector_count == right_desc.vector_length) {
                        // mat2x3 * mat4x2 -> mat4x3
                        result_desc.vector_count  = right_desc.vector_count;
                        result_desc.vector_length = left_desc.vector_length;
                    }
                } else { // +, -, /
                    if (left_desc.vector_count == right_desc.vector_count &&
                        left_desc.vector_length == right_desc.vector_length)
                    {
                        result_desc.vector_count  = left_desc.vector_count;
                        result_desc.vector_length = left_desc.vector_length;
                    }
                }
            } else if (left_structure == kMatrix && right_structure == kVector) {
                if (op == TokenType::kStar && left_desc.vector_count == right_desc.vector_length) {
                    result_desc.vector_count  = 1;
                    result_desc.vector_length = left_desc.vector_length;
                }
            } else if (left_structure == kVector && right_structure == kMatrix) {
                if (op == TokenType::kStar && left_desc.vector_length == right_desc.vector_length) {
                    result_desc.vector_count  = 1;
                    result_desc.vector_length = right_desc.vector_count;
                }
            } else { // 矩阵和标量
                if (left_structure == kMatrix) {
                    result_desc.vector_count  = left_desc.vector_count;
                    result_desc.vector_length = left_desc.vector_length;
                } else { // right is matrix
                    result_desc.vector_count  = right_desc.vector_count;
                    result_desc.vector_length = right_desc.vector_length;
                }
            }
        } else {
            if (left_structure == kScalar && right_structure == kVector) {
                result_desc.vector_count  = right_desc.vector_count;
                result_desc.vector_length = right_desc.vector_length;
            } else if (left_structure == kVector && right_structure == kScalar) {
                result_desc.vector_count  = left_desc.vector_count;
                result_desc.vector_length = left_desc.vector_length;
            } else if (left_structure == kScalar && right_structure == kScalar) {
                result_desc.vector_count  = left_desc.vector_count;
                result_desc.vector_length = left_desc.vector_length;
            } else if (left_structure == kVector && right_structure == kVector) {
                if (left_desc.vector_length == right_desc.vector_length) {
                    result_desc.vector_count  = 1;
                    result_desc.vector_length = left_desc.vector_length;
                }
            }
        }

        return GetCanonicalTypeInfo(result_desc);
    }
}
