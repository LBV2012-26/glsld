#include "stdafx.h"
#include "Config.hpp"

#include <utility>
#include <vector>

#include "Base/Logger.hpp"

namespace glsld {
    namespace {
        void ListAllMember(std::string_view prefix, const YAML::Node& node,
                           std::vector<std::pair<std::string, YAML::Node>>& output)
        {
            if (prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string_view::npos) {
                VALKY_LOG_ERROR(VALKY_LOG_ROOT(), "Config invalid name: {}", prefix);
                return;
            }

            output.emplace_back(prefix, node);

            if (node.IsMap()) {
                for (auto it = node.begin(); it != node.end(); ++it) {
                    ListAllMember(prefix.empty() ? it->first.Scalar() : std::string(prefix) + "." + it->first.Scalar(),
                                  it->second, output);
                }
            }
        }
    }

    ConfigBase::ConfigBase(std::string_view name, std::string_view description)
        : name_{ name }
        , description_{ description }
    {}

    void Config::LoadFromFile(std::string_view filename) {
        YAML::Node root = YAML::LoadFile(std::string(filename));
        LoadFromYaml(root);
    }

    void Config::LoadFromYaml(const YAML::Node& root) {
        std::vector<std::pair<std::string, YAML::Node>> yaml_nodes;
        ListAllMember("", root, yaml_nodes);

        PendingMap pending;
        {
            std::shared_lock main_lock(MainMutex());
            for (const auto& [key, node] : yaml_nodes) {
                if (key.empty()) {
                    continue;
                }

                auto it = ConfigVarData().find(key);
                if (it != ConfigVarData().end()) {
                    ConfigVarData().at(key)->FromString(node.IsScalar() ? node.Scalar() : YAML::Dump(node));
                } else {
                    pending.emplace(key, node);
                }
            }
        }

        if (!pending.empty()) {
            std::unique_lock pending_lock(PendingMutex());
            PendingData().merge(pending);
        }
    }

    std::shared_ptr<ConfigBase> Config::LookupBase(std::string_view name) {
        std::shared_lock lock(MainMutex());
        auto it = ConfigVarData().find(name);;
        return it == ConfigVarData().end() ? nullptr : it->second;
    }

    void Config::ForEach(std::function<void(std::shared_ptr<ConfigBase>)> callback) {
        std::shared_lock lock(MainMutex());
        for (const auto& [_, var] : ConfigVarData()) {
            callback(var);
        }
    }
}
