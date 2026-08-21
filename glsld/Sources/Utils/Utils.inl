#include "Utils.hpp"

#include <format>

namespace glsld::Utils {
    std::string BuildQualifierParameterList(const IsQualifierArgument auto* node) {
        if (node == nullptr) {
            return {};
        }

        std::string params;
        for (auto i = 0uz; i != node->params.size(); ++i) {
            params += SerializeQualifierArguments(node->params[i]);
            if (i + 1 != node->params.size()) {
                params += ", ";
            }
        }

        return params;
    }

    template <typename Ty>
    std::optional<std::vector<Ty>> CollectArgumentArray(
        const QualifierArgumentNode* rhs,
        QualifierArgumentKind required_kind,
        auto&& pred)
    {
        if (rhs == nullptr || rhs->arg_kind != QualifierArgumentKind::kArray) {
            return std::nullopt;
        }

        std::vector<Ty> result;
        for (const auto& child : rhs->children) {
            if (child == nullptr || child->arg_kind != required_kind) {
                return std::nullopt;
            }

            auto value = pred(child->token.text);
            result.push_back(std::move(value));
        }

        std::ranges::sort(result);
        auto [first, last] = std::ranges::unique(result);
        result.erase(first, last);
        return result;
    }
}
