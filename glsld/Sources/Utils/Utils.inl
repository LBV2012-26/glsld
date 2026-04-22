#include "Utils.hpp"

#include <format>

namespace glsld::utils {
    inline std::string BuildQualifierParameterList(const IsQualifierArgument auto* node) {
        if (node == nullptr) {
            return {};
        }

        std::string params;
        for (auto i = 0uz; i != node->params.size(); ++i) {
            params += SerializeQualifierArguments(node->params[i].get());
            if (i + 1 != node->params.size()) {
                params += ", ";
            }
        }

        return params;
    }
}
