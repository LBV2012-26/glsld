#pragma once

#include <ankerl/unordered_dense.h>
#include <yaml-cpp/yaml.h>

namespace YAML {
    template <typename Ty>
    struct convert<ankerl::unordered_dense::set<Ty>> {
        static Node encode(const ankerl::unordered_dense::set<Ty>& rhs) {
            Node node(NodeType::Sequence);
            for (const auto& item : rhs) {
                node.push_back(item);
            }

            return node;
        }

        static bool decode(const Node& node, ankerl::unordered_dense::set<Ty>& rhs) {
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
