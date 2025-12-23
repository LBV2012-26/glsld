#pragma once

#include <unordered_set>
#include <yaml-cpp/yaml.h>

namespace YAML {
    template <typename Ty>
    struct convert<std::unordered_set<Ty>> {
        static Node encode(const std::unordered_set<Ty>& rhs) {
            Node node(NodeType::Sequence);
            for (const auto& item : rhs) {
                node.push_back(item);
            }

            return node;
        }

        static bool decode(const Node& node, std::unordered_set<Ty>& rhs) {
            if (!node.IsSequence()) {
                return false;
            }

            rhs.clear();
            for (const auto& item_node : node) {
                rhs.insert(item_node.as<Ty>());
            }

            return true;
        }
    };
} // namespace YAML
