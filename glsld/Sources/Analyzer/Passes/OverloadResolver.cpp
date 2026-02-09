#include "stdafx.h"
#include "OverloadResolver.hpp"

#include <cstddef>
#include <charconv>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace glsld {
    namespace {
        enum class BaseFamily {
            kUnknown,
            kVoid,
            kBool,
            kInt,
            kUint,
            kFloat,
            kOpaque
        };

        struct TypeDesc {
            BaseFamily family{ BaseFamily::kUnknown };
            int        bits{};
            int        rows{ 1 };
            int        cols{ 1 };
            bool       is_matrix{ false };

            bool operator==(const TypeDesc& other) const {
                return family    == other.family
                    && bits      == other.bits
                    && rows      == other.rows
                    && cols      == other.cols
                    && is_matrix == other.is_matrix;
            }
        };

        TypeDesc ParseGlslType(std::string_view text) {
            static const std::vector<std::string> kOpaquePrefix{
                "sampler", "isampler", "usampler",
                "image", "iimage", "uimage",
                "texture", "shadow",
                "subpass", "isubpass", "usubpass",
                "accelerationStructure", "rayQuery", "rayPayload",
                "hitAttribute", "callableData", "shaderRecord",
                "atomic"
            };

            TypeDesc desc;

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
            desc.is_matrix = is_matrix;

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
                    desc.cols = suffix[x_pos - 1] - '0';
                    desc.rows = suffix[x_pos + 1] - '0';
                }
            } else {
                int dimension = suffix[0] - '0';
                desc.rows = dimension; // vector lengths or matrix rows
                if (is_matrix) {
                    desc.cols = dimension;
                } else {
                    desc.cols = 1;
                }
            }

            return desc;
        }

        bool CanImplicityConvert(const TypeInfo& from, const TypeInfo& to) {
            if (from == to) {
                return true;
            }

            if (from.is_array() != to.is_array() ||
                from.block_symbol != nullptr || to.block_symbol != nullptr)
            {
                return false;
            }

            auto from_desc = ParseGlslType(from.typename_token.text);
            auto to_desc   = ParseGlslType(to.typename_token.text);

            if (from_desc.family == BaseFamily::kOpaque  || to_desc.family == BaseFamily::kOpaque  ||
                from_desc.family == BaseFamily::kUnknown || to_desc.family == BaseFamily::kUnknown ||
                from_desc.family == BaseFamily::kBool    || to_desc.family == BaseFamily::kBool)
            {
                return false;
            }

            if (from_desc.rows != to_desc.rows || from_desc.cols != to_desc.cols || from_desc.is_matrix != to_desc.is_matrix) {
                return false;
            }

            // 类型提升
            // 相同 Family，允许位宽提升
            if (from_desc.family == to_desc.family) {
                return from_desc.bits <= to_desc.bits;
            }

            // int -> uint/float/double
            if (from_desc.family == BaseFamily::kInt)
            {
                if (to_desc.family == BaseFamily::kFloat || to_desc.family == BaseFamily::kUint) {
                    return from_desc.bits <= to_desc.bits;
                }
            }

            // uint -> float/double
            if (from_desc.family == BaseFamily::kUint) {
                if (to_desc.family == BaseFamily::kFloat) {
                    return from_desc.bits <= to_desc.bits;
                } else if (to_desc.family == BaseFamily::kInt) {
                    return false;
                }
            }

            if (from_desc.family == BaseFamily::kFloat) {
                return false;
            }

            return false;
        }

        enum class MatchGrade {
            kNone,
            kImplicit,
            kExact
        };

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

    OverloadResolver::OverloadResolver(const DocumentSymbols& symbols, BindingMap& bindings)
        : symbols_{ symbols }
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

                if (call_type == target_type) {
                    current_grades.push_back(MatchGrade::kExact);
                } else if (CanImplicityConvert(call_type, target_type)) {
                    current_grades.push_back(MatchGrade::kImplicit);
                } else {
                    match_failed = true;
                    break;
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
            return failed_matches;
        } else if (possible_matches.size() == 1) {
            return possible_matches.front().symbol;
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
