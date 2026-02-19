#include "stdafx.h"
#include "OverloadResolver.hpp"

#include <cstddef>
#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace glsld {
    namespace {
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

    OverloadResolver::OverloadResolver(const DocumentSymbols& symbols, BindingMap& bindings, int version_replica,
                                       std::shared_ptr<const std::atomic<int>> version_pointer)
        : AstVisitor(version_replica, version_pointer)
        , symbols_{ symbols }
        , bindings_{ bindings }
    {}

    void OverloadResolver::VisitCallExpression(CallExpressionNode* node) {
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

    SymbolReference OverloadResolver::ResolveOverload(const SymbolList& candidates, std::span<const TypeInfo> call_arg_types) {
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
