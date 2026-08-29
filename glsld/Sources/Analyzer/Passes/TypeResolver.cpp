#include "pch.hpp"
#include "TypeResolver.hpp"

#include <algorithm>
#include <charconv>
#include <format>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

#include "Analyzer/Ast/Ast.hpp"
#include "Analyzer/Passes/ConstantEvaluator.hpp"
#include "Base/Hash.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    namespace {
        std::string GetTypeBitsPrefix(const TypeDescriptor& type_desc) {
            static thread_local ankerl::unordered_dense::map<TypeDescriptor, std::string, TypeDescriptorHash> cache;
            auto it = cache.find(type_desc);
            if (it != cache.end()) {
                return it->second;
            }

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

            auto [inserted_it, _] = cache.try_emplace(type_desc, std::move(prefix));
            return inserted_it->second;
        }

        std::string GetScalarTypename(const TypeDescriptor& type_desc) {
            static thread_local ankerl::unordered_dense::map<TypeDescriptor, std::string, TypeDescriptorHash> cache;
            auto it = cache.find(type_desc);
            if (it != cache.end()) {
                return it->second;
            }

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

            auto [inserted_it, _] = cache.try_emplace(type_desc, std::move(name));
            return inserted_it->second;
        }

        MatchGrade TryImplicityConvert(const TypeInfo& from, const TypeInfo& to) {
            if (from.typename_token.text == "" || to.typename_token.text == "")
                return MatchGrade::kFailed;
            if (from.typename_token.type == TokenType::kUnknown || to.typename_token.type == TokenType::kUnknown)
                return MatchGrade::kWildcard;
            if (from == to)
                throw std::logic_error("This path should not be reached since exact matches are handled separately.");
            if (from.is_array() || to.is_array())
                return MatchGrade::kFailed;

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
                    return MatchGrade::kImplicitly;
                } else {
                    return MatchGrade::kFailed;
                }
            }

            // int -> uint/float/double
            if (from_desc.family == BaseFamily::kInt) {
                if (to_desc.family == BaseFamily::kUint || to_desc.family == BaseFamily::kFloat) {
                    if (from_desc.bits <= to_desc.bits) {
                        return MatchGrade::kImplicitly;
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
                        return MatchGrade::kImplicitly;
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
            int lhs_better = 0;
            int rhs_better = 0;

            const auto min_size = std::min(lhs.param_grades.size(), rhs.param_grades.size());

            for (auto i = 0uz; i != min_size; ++i) {
                if (lhs.param_grades[i] > rhs.param_grades[i])
                    ++lhs_better;
                if (rhs.param_grades[i] > lhs.param_grades[i])
                    ++rhs_better;
            }

            if (lhs_better > 0 && rhs_better == 0)
                return MatchResult::kLhsBetter;
            if (rhs_better > 0 && lhs_better == 0)
                return MatchResult::kRhsBetter;

            if (lhs_better > 0 && rhs_better > 0) {
                if (lhs.symbol->param_typeinfos.size() < rhs.symbol->param_typeinfos.size()) {
                    return MatchResult::kLhsBetter;
                } else if (lhs.symbol->param_typeinfos.size() > rhs.symbol->param_typeinfos.size()) {
                    return MatchResult::kRhsBetter;
                }

                if (lhs_better > rhs_better) {
                    return MatchResult::kLhsBetter;
                } else if (rhs_better > lhs_better) {
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

    TypeResolver::TypeResolver(Document& document, int version_replica, VersionPointer version_pointer)
        : AstVisitor(version_replica, version_pointer)
        , document_{ document }
    {
        Traverse(document_.ast);
    }

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

            const auto compare_result = CompareCandidates(score_results[index].score, score_results[i].score);
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

        for (auto& template_arg : node->type_spec.template_args) {
            Traverse(template_arg);
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
            VisitVariableDeclaration(param_node);

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
            Traverse(node->body);
        }
    }

    void TypeResolver::VisitVariableDeclaration(VariableDeclarationNode* node) {
        if (node->declared_symbol == nullptr) {
            return;
        }

        for (auto* array_size : node->type_spec.array_sizes) {
            Traverse(array_size);
        }

        for (auto* template_arg : node->type_spec.template_args) {
            Traverse(template_arg);
        }

        auto* variable_symbol = node->declared_symbol;
        document_.bindings.try_emplace(variable_symbol->location, variable_symbol);
        variable_symbol->type_info = ExtractTypeInfo(node->type_spec, node->located_scope);

        if (node->init == nullptr) {
            return;
        }

        Traverse(node->init);

        const auto& init_type  = node->init->evaluated_type;
        const auto& init_sizes = init_type.array_sizes;
        const auto& decl_sizes = variable_symbol->type_info.array_sizes;

        if (init_sizes.empty()) {
            return;
        }

        std::vector<std::optional<std::uint64_t>> resolved_sizes(decl_sizes.begin(), decl_sizes.end());

        const auto min_size = std::min(resolved_sizes.size(), init_sizes.size());
        bool changed = false;

        for (auto i = 0uz; i != min_size; ++i) {
            if (!resolved_sizes[i].has_value()) {
                resolved_sizes[i] = init_sizes[i];
                changed = true;
            }
        }

        if (changed) {
            variable_symbol->type_info.array_sizes =
                document_.arena->CopySpan<std::optional<std::uint64_t>>(resolved_sizes);
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

    void TypeResolver::VisitInitializerListExpression(InitializerListExpressionNode* node) {
        if (node->elements.empty()) {
            node->evaluated_type = {
                .typename_token{
                    .text = "unknown",
                    .type = TokenType::kUnknown
                }
            };

            return;
        }

        TypeInfo common_type;
        bool is_first   = true;
        bool type_error = false;

        for (const auto& element : node->elements) {
            if (element == nullptr) {
                continue;
            }

            Traverse(element);
            const auto& element_type = element->evaluated_type;

            if (is_first) {
                common_type = element_type;
                is_first = false;
            } else {
                if (common_type.CompareWithoutQualifiers(element_type)) {
                    continue; // 类型相同，继续检查下一个元素
                } else if (TryImplicityConvert(element_type, common_type) != MatchGrade::kFailed) {
                    continue; // 元素类型可以隐式转换为当前公共类型，继续检查下一个元素
                } else if (TryImplicityConvert(common_type, element_type) != MatchGrade::kFailed) {
                    common_type = element_type; // 有一个高级类型，必须提升 common_type
                } else {
                    type_error = true; // 类型不兼容，标记错误
                    break;
                }
            }
        }

        if (type_error || common_type.typename_token.type == TokenType::kUnknown) {
            node->evaluated_type = {
                .typename_token{
                    .text = "unknown",
                    .type = TokenType::kUnknown
                }
            };
        } else {
            node->evaluated_type = common_type;

            auto old_sizes = node->evaluated_type.array_sizes;
            std::vector<std::optional<std::uint64_t>> new_sizes;
            new_sizes.reserve(old_sizes.size() + 1);
            new_sizes.push_back(node->elements.size());
            new_sizes.append_range(old_sizes);

            node->evaluated_type.array_sizes = document_.arena->CopySpan<std::optional<std::uint64_t>>(new_sizes);
        }
    }

    namespace {
        bool IsArithmeticType(const TypeInfo& type) {
            if (type.is_array() || type.block_symbol != nullptr) {
                return false;
            }

            const auto& desc = type.type_desc;

            if (desc.vector_count < 1 || desc.vector_length < 1) {
                return false;
            }

            switch (desc.family) {
            case BaseFamily::kBool:
            case BaseFamily::kInt:
            case BaseFamily::kUint:
            case BaseFamily::kFloat:
                return true;
            default:
                return false;
            }
        }

        bool IsUInt64Type(const TypeInfo& type) {
            return !type.is_array()
                 && type.block_symbol            == nullptr
                 && type.type_desc.family        == BaseFamily::kUint
                 && type.type_desc.bits          == 64
                 && type.type_desc.vector_count  == 1
                 && type.type_desc.vector_length == 1;
        }

        bool IsUVec2Type(const TypeInfo& type) {
            return !type.is_array()
                && type.block_symbol            == nullptr
                && type.type_desc.family        == BaseFamily::kUint
                && type.type_desc.bits          == 32
                && type.type_desc.vector_count  == 1
                && type.type_desc.vector_length == 2;
        }

        bool IsBufferReferenceType(const TypeInfo& type) {
            const auto* symbol = type.block_symbol;
            if (symbol == nullptr || symbol->kind != SymbolKind::kInterface) {
                return false;
            }

            if (symbol->node->kind() != AstNodeKind::kInterfaceDeclaration) {
                return false;
            }

            auto declaration = static_cast<const InterfaceDeclarationNode*>(symbol->node);

            return std::ranges::any_of(declaration->type_spec.layouts, [](const auto* layout) -> bool {
                if (layout == nullptr) {
                    return false;
                }

                return std::ranges::any_of(layout->raw_tokens, [](const auto& token) -> bool {
                    return token.text == "buffer_reference";
                });
            });
        }

        bool CanExplicitlyCast(const TypeInfo& source, const TypeInfo& target) {
            if (source.is_array() || target.is_array()) {
                return false;
            }

            const bool src_is_buffer_reference = IsBufferReferenceType(source);
            const bool dst_is_buffer_reference = IsBufferReferenceType(target);

            // GL_EXT_buffer_reference:
            //     buffer reference <-> uint64_t
            //
            // GL_EXT_buffer_reference_uvec2:
            //     buffer reference <-> uvec2
            //
            if (src_is_buffer_reference || dst_is_buffer_reference) {
                if (src_is_buffer_reference && dst_is_buffer_reference) {
                    return true;
                }

                const auto& arithmetic_type = src_is_buffer_reference ? target : source;
                return IsUInt64Type(arithmetic_type) || IsUVec2Type(arithmetic_type);
            }

            // GL_NV_explicit_typecast 不支持除 buffer_reference 以外的定义类型强转
            if (source.block_symbol != nullptr || target.block_symbol != nullptr)
                return false;
            if (!IsArithmeticType(source) || !IsArithmeticType(target))
                return false;

            const auto src_structure = source.type_desc.arithmetic_structure();
            const auto dst_structure = target.type_desc.arithmetic_structure();

            switch (src_structure) {
                using enum TypeDescriptor::ArithmeticStructure;
            case kScalar:
                // scalar -> scalar/vector/matrix
                return true;
            case kVector:
                // vector -> vector with same component
                if (dst_structure == kVector) {
                    return target.type_desc.vector_length <= source.type_desc.vector_length;
                }

                // vec4 -> mat2x2
                return source.type_desc.vector_length == 4
                    && dst_structure == kMatrix
                    && target.type_desc.vector_count  == 2
                    && target.type_desc.vector_length == 2;
            case kMatrix:
                // matrix -> matrix
                return dst_structure == kMatrix;
            }

            return false;
        }
    }

    void TypeResolver::VisitCastExpression(CastExpressionNode* node) {
        if (node->operand == nullptr) {
            return;
        }

        Traverse(node->operand);

        const auto source_type = node->operand->evaluated_type;
        const auto target_type = ExtractTypeInfo(node->target_type, node->located_scope);
        if (!target_type.is_valid()) {
            node->evaluated_type = {
                .typename_token{
                    .text = "unknown",
                    .type = TokenType::kUnknown
                }
            };

            return;
        }

        if (!source_type.is_valid()) {
            node->evaluated_type = target_type;
            return;
        }

        if (!CanExplicitlyCast(source_type, target_type)) {
            node->evaluated_type = {
                .typename_token{
                    .text     = "unknown",
                    .location = node->target_type.typename_token().location,
                    .type     = TokenType::kUnknown
                }
            };

            return;
        }

        node->evaluated_type = target_type;
    }

    void TypeResolver::VisitBinaryExpression(BinaryExpressionNode* node) {
        Traverse(node->left);
        Traverse(node->right);

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

        Traverse(node->operand);

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
            Traverse(node->condition);
        if (node->true_expr != nullptr)
            Traverse(node->true_expr);
        if (node->false_expr != nullptr)
            Traverse(node->false_expr);

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
                Traverse(arg);
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

        Traverse(node->callee);

        ExpressionNode* current_base = node->callee;
        std::vector<std::optional<std::uint64_t>> dimensions;
        bool is_array_constructor = false;

        while (current_base->kind() == AstNodeKind::kIndexExpression) {
            auto* index_node = static_cast<IndexExpressionNode*>(current_base);
            is_array_constructor = true;

            std::optional<std::uint64_t> size;
            if (index_node->index != nullptr) {
                ConstantEvaluator evaluator;
                size = evaluator.EvaluateAs<std::uint64_t>(index_node->index);
            } else {
                size = std::nullopt;
            }

            dimensions.push_back(std::move(size));
            current_base = index_node->base;
        }

        // int array[] = int[](...)
        if (is_array_constructor) {
            if (current_base->kind() == AstNodeKind::kVariableExpression) {
                bool is_constructor = false;
                TypeDescriptor base_desc;
                const SymbolInfo* block_symbol = nullptr;

                const auto* base_varexpr = static_cast<VariableExpressionNode*>(current_base);
                if (base_varexpr->original_token.type == TokenType::kPrimitive ||
                    base_varexpr->original_token.type == TokenType::kBuiltInType)
                {
                    is_constructor = true;
                    base_desc = ParseTypeDescriptor(base_varexpr->name);
                } else if (auto* symbol = std::get_if<const SymbolInfo*>(&base_varexpr->linked_symbols)) {
                    if (*symbol != nullptr) {
                        if ((*symbol)->kind == SymbolKind::kInterface || (*symbol)->kind == SymbolKind::kStruct) {
                            is_constructor = true;
                            base_desc      = (*symbol)->type_info.type_desc;
                            block_symbol   = *symbol;
                        }
                    }
                }

                if (is_constructor) {
                    TypeInfo array_type{
                        .typename_token{
                            .text = base_varexpr->name,
                            .type = base_varexpr->original_token.type
                        },
                        .type_desc    = base_desc,
                        .block_symbol = block_symbol
                    };

                    std::ranges::reverse(dimensions);
#ifdef _MSVC_LANG
                    if (std::ranges::contains(dimensions, std::nullopt)) {
#else
                    if (std::ranges::any_of(dimensions, [](const auto& d) -> bool { return !d.has_value(); })) {
#endif
                        const auto dimensions_from_args = DeduceArraySizesFromArgs(node);
                        for (auto&& [target, source] : std::views::zip(dimensions, dimensions_from_args)) {
                            if (!target.has_value()) {
                                target = source;
                            }
                        }
                    }

                    array_type.array_sizes = document_.arena->CopySpan<std::optional<std::uint64_t>>(dimensions);

                    node->evaluated_type         = array_type;
                    node->callee->evaluated_type = array_type;
                    return;
                }
            }
        }

        if (node->callee->kind() != AstNodeKind::kVariableExpression) {
            return;
        }

        auto* callee_node = static_cast<VariableExpressionNode*>(node->callee);

        if (callee_node->original_token.type == TokenType::kPrimitive ||
            callee_node->original_token.type == TokenType::kBuiltInType)
        {
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

        callee_node->linked_symbols = std::visit(Overloaded{
            [&](SymbolListView candidates) -> SymbolReferenceView {
                const auto resolved = ResolveOverload(candidates, call_arg_types);

                return std::visit(Overloaded{
                    [&](const SymbolInfo* best_match) -> SymbolReferenceView {
                        callee_node->evaluated_type            = best_match->type_info;
                        node->evaluated_type                   = best_match->type_info;
                        document_.bindings[callee_node->begin] = best_match;
                        return best_match;
                    },
                    [&](const SymbolList&) -> SymbolReferenceView {
                        return std::get<SymbolListView>(document_.ReferenceSymbol(resolved));
                    },
                    [](std::monostate) -> SymbolReferenceView {
                        return std::monostate{};
                    }
                }, resolved);
            },
            [&](const SymbolInfo* symbol) -> SymbolReferenceView {
                if (symbol == nullptr) {
                    return nullptr;
                }

                TypeInfo result_type = symbol->type_info;
                if (symbol->kind == SymbolKind::kInterface || symbol->kind == SymbolKind::kStruct) {
                    result_type.block_symbol   = symbol;
                    result_type.typename_token = Token{
                        .text     = symbol->name,
                        .location = symbol->location,
                        .type     = TokenType::kIdentifier
                    };
                }

                callee_node->evaluated_type            = result_type;
                node->evaluated_type                   = result_type;
                document_.bindings[callee_node->begin] = symbol;

                return symbol;
            },
            [](std::monostate) -> SymbolReferenceView {
                return std::monostate{};
            }
        }, callee_node->linked_symbols);
    }

    void TypeResolver::VisitIndexExpression(IndexExpressionNode* node) {
        Traverse(node->base);
        Traverse(node->index);

        const auto& base_type = node->base->evaluated_type;
        auto& evaluated_type  = node->evaluated_type;

        if (base_type.is_array()) {
            evaluated_type             = base_type;
            evaluated_type.array_sizes = evaluated_type.array_sizes.subspan(1); // 剥离最外层数组维度
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
                },
                .type_desc = TypeDescriptor{
                    .family        = BaseFamily::kBool,
                    .bits          = 32,
                    .vector_count  = 1,
                    .vector_length = 1
                }
            };

            node->evaluated_type = info;
            return;
        }

        std::visit(Overloaded{
            [&](const SymbolInfo* symbol) -> void {
                if (symbol != nullptr)
                    node->evaluated_type = symbol->type_info; // 根据指向的符号类型推导当前符号类型
                if (symbol && (symbol->kind == SymbolKind::kFunctionDecl || symbol->kind == SymbolKind::kFunctionImpl))
                    node->evaluated_type.is_func_ref = true;
            },
            [&](SymbolListView list) -> void {
                if (!list.empty() && (list.front()->kind == SymbolKind::kFunctionDecl ||
                                      list.front()->kind == SymbolKind::kFunctionImpl))
                {
                    node->evaluated_type.is_func_ref = true;
                }
            },
            [](std::monostate) -> void {}
        }, node->linked_symbols);
    }

    void TypeResolver::VisitRawExpression(RawExpressionNode* node) {
        if (!node->tokens.empty()) {
            // 看第一个就够
            node->evaluated_type = SniffLiteralType(node->tokens.front());
        }
    }

    namespace {
        bool SupportsLengthMethod(const TypeInfo& type) {
            if (type.is_array()) {
                return true;
            }

            if (!type.is_valid() || type.block_symbol != nullptr ||
                type.type_desc.family == BaseFamily::kUnknown ||
                type.type_desc.family == BaseFamily::kVoid ||
                type.type_desc.family == BaseFamily::kOpaque)
            {
                return false;
            }

            using enum TypeDescriptor::ArithmeticStructure;

            const auto structure = type.type_desc.arithmetic_structure();
            return structure == kVector || structure == kMatrix;
        }

        TypeInfo BuildLengthResultType(const SourceLocation& location) {
            return TypeInfo{
                .typename_token = Token{
                    .text     = "int",
                    .location = location,
                    .type     = TokenType::kPrimitive
                },
                .type_desc = TypeDescriptor{
                    .family        = BaseFamily::kInt,
                    .bits          = 32,
                    .vector_count  = 1,
                    .vector_length = 1
                }
            };
        }

        TypeInfo BuildUnknownType(const SourceLocation& location) {
            return TypeInfo{
                .typename_token = Token{
                    .text     = "unknown",
                    .location = location,
                    .type     = TokenType::kUnknown
                },
                .type_desc = TypeDescriptor{
                    .family = BaseFamily::kUnknown
                }
            };
        }
    }

    void TypeResolver::VisitMemberAccessExpression(MemberAccessExpressionNode* node) {
        if (node->object == nullptr) {
            node->evaluated_type = BuildUnknownType(node->begin);
            return;
        }

        Traverse(node->object);
        // Traverse(node->member); SymbolLinker 不知道结构体内部作用域，遍历了也鸡毛用没有

        if (node->member != nullptr && node->member->kind() == AstNodeKind::kCallExpression) {
            auto* length_call = FindLengthCall(node);
            if (length_call == nullptr || !SupportsLengthMethod(node->object->evaluated_type)) {
                auto unknown_type = BuildUnknownType(node->begin);
                node->evaluated_type = unknown_type;

                auto* member_call = static_cast<CallExpressionNode*>(node->member);
                member_call->evaluated_type = unknown_type;

                if (member_call->callee != nullptr) {
                    member_call->callee->evaluated_type = std::move(unknown_type);
                }

                return;
            }

            auto* callee = static_cast<VariableExpressionNode*>(length_call->callee);
            const auto result_type = BuildLengthResultType(callee->original_token.location);

            callee->evaluated_type      = result_type;
            length_call->evaluated_type = result_type;
            node->evaluated_type        = result_type;
            return;
        }

        const auto& object_type  = node->object->evaluated_type;
        const auto* block_symbol = object_type.block_symbol;

        if (block_symbol != nullptr && block_symbol->internal_scope != nullptr && node->member != nullptr) {
            if (node->member->kind() == AstNodeKind::kVariableExpression) {
                auto* member_node = static_cast<VariableExpressionNode*>(node->member);
                const auto* member_symbol = block_symbol->internal_scope->FindSymbol(member_node->name);

                if (member_symbol != nullptr) {
                    member_node->linked_symbols = member_symbol;
                    member_node->evaluated_type = member_symbol->type_info;
                    node->evaluated_type = member_node->evaluated_type;

                    document_.bindings.try_emplace(member_node->begin, member_symbol);
                }
            } else { // node->member->kind() == AstNodeKind::kCallExpression
                // do nothing, GLSL not support member function
            }
        } else if (object_type.is_builtin()) {
            if (node->member == nullptr || node->member->kind() != AstNodeKind::kVariableExpression) {
                return;
            }

            const auto* member_node = static_cast<const VariableExpressionNode*>(node->member);
            node->evaluated_type = ResolveSwizzleType(object_type, member_node->name);
        } else if (node->member == nullptr) {
            node->evaluated_type = object_type;
        }
    }

    void TypeResolver::SeparateType(TypeInfo& type_info, bool keep_vector) {
        if (keep_vector) {
            std::string prefix = GetTypeBitsPrefix(type_info.type_desc);
            type_info.typename_token.text =
                document_.StoreTokenText(std::format("{}vec{}", prefix, type_info.type_desc.vector_length));
            type_info.type_desc.vector_count = 1;
        } else {
            type_info.typename_token.text =
                document_.StoreTokenText(GetScalarTypename(type_info.type_desc));
            type_info.type_desc.vector_count  = 1;
            type_info.type_desc.vector_length = 1;
        }
    }

    TypeInfo TypeResolver::GetCanonicalTypeInfo(const TypeDescriptor& type_desc) {
        auto it = type_desc_cache_.find(type_desc);
        if (it != type_desc_cache_.end()) {
            return it->second;
        }

        if (type_desc.family == BaseFamily::kUnknown) {
            return {
                .typename_token{
                    .text = "unknown",
                    .type = TokenType::kUnknown
                },
                .type_desc = type_desc
            };
        }

        const auto arithmetic_structure = type_desc.arithmetic_structure();
        using enum TypeDescriptor::ArithmeticStructure;

        TypeInfo type_info;
        type_info.typename_token.type = TokenType::kBuiltInType;

        if (arithmetic_structure == kScalar) {
            type_info.typename_token.text = document_.StoreTokenText(GetScalarTypename(type_desc));
        } else {
            std::string prefix = GetTypeBitsPrefix(type_desc);
            if (arithmetic_structure == kVector) {
                type_info.typename_token.text =
                    document_.StoreTokenText(std::format("{}vec{}", prefix, type_desc.vector_length));
            } else {
                type_info.typename_token.text =
                    document_.StoreTokenText(std::format("{}mat{}x{}", prefix, type_desc.vector_count, type_desc.vector_length));
            }
        }

        type_info.type_desc = type_desc;

        auto [inserted_it, _] = type_desc_cache_.try_emplace(type_desc, std::move(type_info));
        return inserted_it->second;
    }

    TypeInfo TypeResolver::SplitCanonicalTypeInfo(const TypeInfo& base_type) {
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

    std::vector<std::int64_t> TypeResolver::DeduceArraySizesFromArgs(const CallExpressionNode* call_node) {
        std::vector<std::int64_t> dimensions;
        if (call_node->args.empty()) {
            return dimensions;
        }

        dimensions.push_back(static_cast<std::int64_t>(call_node->args.size()));
        const auto* first_arg = call_node->args.front();

        if (first_arg->kind() == AstNodeKind::kCallExpression) {
            auto* next_call = static_cast<const CallExpressionNode*>(first_arg);
            if (next_call->callee->kind() == AstNodeKind::kIndexExpression) {
                auto next_dimensions = DeduceArraySizesFromArgs(next_call);
                dimensions.append_range(next_dimensions | std::views::as_rvalue);
            }
        }

        return dimensions;
    }

    namespace {
        std::pair<const QualifierArgumentNode*, std::string_view> ExtractAssignment(const QualifierArgumentNode* node) {
            if (node == nullptr || node->arg_kind != QualifierArgumentKind::kAssignment || node->children.size() != 2) {
                return { nullptr, "" };
            }

            const auto& lhs = node->children.front();
            if (lhs == nullptr || lhs->arg_kind != QualifierArgumentKind::kIdentifier) {
                return { nullptr, "" };
            }

            return { node->children[1], lhs->token.text };
        }

        std::optional<std::vector<std::string>> CollectStringArray(const QualifierArgumentNode* rhs) {
            return Utils::CollectArgumentArray<std::string>(rhs, QualifierArgumentKind::kStringLiteral, Utils::UnquoteStringLiteral);
        }

        std::optional<std::vector<std::int64_t>> CollectIntegerArray(const QualifierArgumentNode* rhs) {
            return Utils::CollectArgumentArray<std::int64_t>(rhs, QualifierArgumentKind::kNumberLiteral, Utils::ParseNumberLiteralToInteger);
        }
    }

    std::expected<SpirvTypeSignature, std::string> TypeResolver::BuildSpirvTypeSignature(const SpirvIntrinsicNode* node) {
        SpirvTypeSignature signature;

        if (node == nullptr) {
            return std::unexpected("spirv_type node is null");
        }

        std::vector<std::string>           signature_extensions;
        std::vector<std::int64_t>          signature_capabilities;
        std::vector<SpirvOperandSignature> signature_operands;

        for (const auto& param : node->params) {
            if (param == nullptr) {
                continue;
            }

            auto [rhs, key] = ExtractAssignment(param);
            if (!key.empty()) {
                if (key == "extensions") {
                    if (!signature.extensions.empty()) {
                        return std::unexpected("duplicate extensions");
                    }

                    auto extensions = CollectStringArray(rhs);
                    if (!extensions.has_value()) {
                        return std::unexpected("invalid extensions format");
                    }

                    signature_extensions = std::move(*extensions);
                    continue;
                }

                if (key == "capabilities") {
                    if (!signature.capabilities.empty()) {
                        return std::unexpected("duplicate capabilities");
                    }

                    auto capabilities = CollectIntegerArray(rhs);
                    if (!capabilities.has_value()) {
                        return std::unexpected("invalid capabilities format");
                    }

                    signature_capabilities = std::move(*capabilities);
                    continue;
                }

                if (key == "id") {
                    if (signature.id.has_value()) {
                        return std::unexpected("duplicate id");
                    }

                    if (rhs == nullptr || rhs->arg_kind != QualifierArgumentKind::kNumberLiteral) {
                        return std::unexpected("invalid id format");
                    }

                    signature.id = Utils::ParseNumberLiteralToInteger(rhs->token.text);
                    continue;
                }

                if (key == "set") {
                    if (signature.set.has_value()) {
                        return std::unexpected("duplicate set");
                    }

                    if (rhs == nullptr || rhs->arg_kind != QualifierArgumentKind::kStringLiteral) {
                        return std::unexpected("invalid set format");
                    }

                    signature.set = document_.StoreTokenText(Utils::UnquoteStringLiteral(rhs->token.text));
                    continue;
                }

                return std::unexpected(std::format("unknown parameter '{}' in spirv_type", key));
            }

            if (!signature.id.has_value()) {
                return std::unexpected("missing id parameter in spirv_type");
            }

            // spirv_id <expr>
            if (param->arg_kind == QualifierArgumentKind::kSequence &&
                !param->children.empty() &&
                param->children.front() != nullptr &&
                param->children.front()->arg_kind == QualifierArgumentKind::kIdentifier &&
                param->children.front()->token.text == "spirv_id")
            {
                if (param->children.size() < 2) {
                    return std::unexpected("missing operand after spirv_id");
                }

                SpirvOperandSignature operand{
                    .kind  = SpirvOperandKind::kIdReference,
                    .value = document_.StoreTokenText(Utils::SerializeQualifierArguments(param->children[1]))
                };

                signature_operands.push_back(std::move(operand));
                continue;
            }

            SpirvOperandSignature operand{
                .kind  = SpirvOperandKind::kLiteral,
                .value = document_.StoreTokenText(Utils::SerializeQualifierArguments(param))
            };

            signature_operands.push_back(std::move(operand));
        }

        if (!signature.id.has_value()) {
            return std::unexpected("missing id parameter in spirv_type");
        }

        std::ranges::sort(signature_extensions);
        auto [extension_first, extension_last] = std::ranges::unique(signature_extensions);
        signature_extensions.erase(extension_first, extension_last);

        std::ranges::sort(signature_capabilities);
        auto [capability_first, capability_last] = std::ranges::unique(signature_capabilities);
        signature_capabilities.erase(capability_first, capability_last);

        std::vector<std::string_view> extension_views;
        for (const auto& extension : signature_extensions) {
            extension_views.push_back(document_.StoreTokenText(extension));
        }

        signature.extensions   = document_.arena->CopySpan<std::string_view>(extension_views);
        signature.capabilities = document_.arena->CopySpan<std::int64_t>(signature_capabilities);
        signature.operands     = document_.arena->CopySpan<SpirvOperandSignature>(signature_operands);

        return signature;
    }

    TypeInfo TypeResolver::ExtractTypeInfo(const TypeSpec& type_spec, const Scope* located_scope) {
        if (type_spec.typename_token().type == TokenType::kUnknown) {
            return {};
        }

        TypeInfo info;

        if (type_spec.typename_token().text == "__Function") {
            info.is_func_ref = true;
        }

        const auto& typename_token = type_spec.typename_token();
        info.typename_token = typename_token;

        std::vector<Token> qualifiers;
        if (type_spec.specifiers.size() > 0) {
            // 去掉最后一个，因为最后一个是类型名
            qualifiers.assign_range(type_spec.specifiers | std::views::take(type_spec.specifiers.size() - 1));
        }

        info.qualifiers = document_.arena->CopySpan<Token>(qualifiers);

        std::vector<std::optional<std::size_t>> array_sizes;
        for (const auto& size : type_spec.array_sizes) {
            if (size == nullptr) {
                array_sizes.push_back(std::nullopt);
                continue;
            }

            ConstantEvaluator evaluator;
            array_sizes.push_back(evaluator.EvaluateAs<std::uint64_t>(size));
        }

        info.array_sizes = document_.arena->CopySpan<std::optional<std::size_t>>(array_sizes);

        // spirv_type
        if (!type_spec.spirv_intrinsics.empty() && type_spec.spirv_type != nullptr &&
            type_spec.spirv_type->intrinsic_kind == SpirvIntrinsicKind::kTypeOverride)
        {
            auto spirv_signature = BuildSpirvTypeSignature(type_spec.spirv_type);
            if (!spirv_signature.has_value()) {
                info.typename_token = {
                    .text     = document_.StoreTokenText(std::format("<error_type>:{}", spirv_signature.error())),
                    .location = type_spec.spirv_type->keyword.location,
                    .type     = TokenType::kUnknown
                };

                info.type_desc = {
                    .family = BaseFamily::kUnknown
                };

                return info;
            }

            info.spirv_signature = std::move(*spirv_signature);
            info.typename_token  = type_spec.spirv_type->keyword;

            const auto spirv_type_params = Utils::BuildQualifierParameterList(type_spec.spirv_type);
            info.spirv_type = document_.StoreTokenText(std::format("spirv_type({})", spirv_type_params));

            info.type_desc = {
                .family = BaseFamily::kOpaque
            };

            return info;
        }

        if (typename_token.type == TokenType::kIdentifier) {
            auto* type_symbol = located_scope->FindTypeSymbol(typename_token.text);
            if (type_symbol == nullptr) {
                return {};
            }

            info.block_symbol = type_symbol;
            document_.bindings.try_emplace(typename_token.location, type_symbol);
        }

        std::vector<std::string> template_args;
        for (const auto& template_arg : type_spec.template_args) {
            std::string arg_text;
            if (template_arg->kind() == AstNodeKind::kVariableExpression) {
                auto* var_expr = static_cast<const VariableExpressionNode*>(template_arg);
                arg_text = var_expr->name;
            } else if (template_arg->kind() == AstNodeKind::kRawExpression) {
                auto* raw_expr = static_cast<const RawExpressionNode*>(template_arg);
                if (!raw_expr->tokens.empty()) {
                    arg_text = raw_expr->tokens.front().text;
                }
            }

            if (!arg_text.empty()) {
                template_args.push_back(std::move(arg_text));
            }
        }

        std::vector<std::string_view> template_arg_views;
        for (const auto& arg : template_args) {
            template_arg_views.push_back(document_.StoreTokenText(arg));
        }

        info.template_args = document_.arena->CopySpan<std::string_view>(template_arg_views);
        info.type_desc     = ParseTypeDescriptor(typename_token.text);

        return info;
    }

    TypeDescriptor TypeResolver::ParseTypeDescriptor(std::string_view text) {
        static thread_local StringHeteroHashMap<TypeDescriptor> cache;
        auto it = cache.find(text);
        if (it != cache.end()) {
            return it->second;
        };

        static const std::vector<std::string> kOpaquePrefix{
            "sampler", "isampler", "usampler",
            "image", "iimage", "uimage",
            "texture", "shadow",
            "subpass", "isubpass", "usubpass",
            "accelerationStructure", "ray", "hit"
            "hitAttribute", "callableData", "shaderRecord",
            "atomic", "NV", "EXT", "KHR"
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

        const auto vec_pos   = text.find("vec");
        const auto mat_pos   = text.find("mat");
        const bool is_matrix = (mat_pos != std::string_view::npos);

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

            const auto num_start = (prefix[0] == 'f' || prefix[0] == 'i' || prefix[0] == 'u') ? 1 : 0;
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
            const int dimension = suffix[0] - '0';
            desc.vector_length = dimension; // vector lengths or matrix rows
            if (is_matrix) {
                desc.vector_count = dimension;
            } else {
                desc.vector_count = 1;
            }
        }

        cache.emplace(text, desc);
        return desc;
    }

    TypeInfo TypeResolver::SniffLiteralType(const Token& token) {
        auto BuildType = [](std::string_view name, BaseFamily family, int bits) -> TypeInfo {
            return TypeInfo{
                .typename_token{
                    .text = name,
                    .type = name.contains("_t") ? TokenType::kBuiltInType : TokenType::kPrimitive
                },
                .type_desc{
                    .family        = family,
                    .bits          = bits,
                    .vector_count  = 1,
                    .vector_length = 1
                }
            };
        };

        if (token.type == TokenType::kNumberLiteral) {
            const auto literal = Utils::AnalyzeNumberLiteral(token.text);

            switch (literal.kind) {
            case Utils::NumberLiteralKind::kSignedInteger:
                if (literal.bits == 64)
                    return BuildType("int64_t", BaseFamily::kInt, 64);
                if (literal.bits == 16)
                    return BuildType("int16_t", BaseFamily::kInt, 16);
                return BuildType("int", BaseFamily::kInt, 32);

            case Utils::NumberLiteralKind::kUnsignedInteger:
                if (literal.bits == 64)
                    return BuildType("uint64_t", BaseFamily::kUint, 64);
                if (literal.bits == 16)
                    return BuildType("uint16_t", BaseFamily::kUint, 16);
                return BuildType("uint", BaseFamily::kUint, 32);

            case Utils::NumberLiteralKind::kFloatingPoint:
                if (literal.bits == 64)
                    return BuildType("double", BaseFamily::kFloat, 64);
                if (literal.bits == 16)
                    return BuildType("float16_t", BaseFamily::kFloat, 16);
                return BuildType("float", BaseFamily::kFloat, 32);

            default:
                return BuildType("unknown", BaseFamily::kUnknown, 0);
            }
        }

        if (token.type == TokenType::kPrimitive) {
            if (token.text == "true" || token.text == "false") {
                return BuildType("bool", BaseFamily::kBool, 32);
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
            return GetCanonicalTypeInfo(TypeDescriptor{
                .family = BaseFamily::kUnknown
            });
        }

        const auto parsed = Utils::ParseVectorSwizzle(
            swizzle, static_cast<std::size_t>(base_type.type_desc.vector_length));

        if (!parsed.has_value()) {
            return GetCanonicalTypeInfo(TypeDescriptor{
                .family = BaseFamily::kUnknown
            });
        }

        TypeInfo result = base_type;
        result.type_desc.vector_count  = 1;
        result.type_desc.vector_length = static_cast<int>(parsed->count);

        SeparateType(result, parsed->count > 1);
        return result;
    }

    SymbolReference TypeResolver::ResolveOverload(SymbolListView candidates, std::span<const TypeInfo> call_arg_types) {
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

            const auto compare_result = CompareCandidates(current, best_matches.front()); // 严格偏序不存在石头剪刀布循环，直接比较第一个即可
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
            return op == TokenType::kEqual
                || op == TokenType::kPlusEqual
                || op == TokenType::kMinusEqual
                || op == TokenType::kStarEqual
                || op == TokenType::kSlashEqual
                || op == TokenType::kPercentEqual
                || op == TokenType::kLeftShiftEqual
                || op == TokenType::kRightShiftEqual
                || op == TokenType::kAmpersandEqual
                || op == TokenType::kCaretEqual
                || op == TokenType::kVerticalBarEqual;
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

        const auto left_desc  = left_type.type_desc;
        const auto right_desc = right_type.type_desc;

        TypeDescriptor result_desc;
        result_desc.family = std::max(left_desc.family, right_desc.family);
        result_desc.bits   = std::max(left_desc.bits,   right_desc.bits);

        const auto left_structure  = left_desc.arithmetic_structure();
        const auto right_structure = right_desc.arithmetic_structure();

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
