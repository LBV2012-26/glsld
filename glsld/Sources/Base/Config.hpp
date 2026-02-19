#pragma once

#include <cstdint>
#include <concepts>
#include <functional>
#include <memory>
#include <span>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <yaml-cpp/yaml.h>
#include "Base/Hash.hpp"

namespace glsld {
    template <typename Ty>
    concept IsPod = std::is_trivial_v<Ty> && std::is_standard_layout_v<Ty> && std::is_arithmetic_v<Ty>;

    template <typename From, typename To>
    struct LexicalCast {};

    template <>
    struct LexicalCast<std::string, std::string> {
        std::string operator()(std::string_view str);
    };

    template <IsPod To>
    struct LexicalCast<std::string, To> {
        To operator()(std::string_view str);
    };

    template <IsPod From>
    struct LexicalCast<From, std::string> {
        std::string operator()(const From& value);
    };

    template <typename Ty>
    concept IsSequentialContainer = requires(Ty container, typename Ty::value_type value) {
        typename Ty::value_type;
        requires std::default_initializable<Ty>;
        { container.push_back(value) } -> std::same_as<void>;
    } && !std::same_as<Ty, std::string>;

    template <typename Ty>
    concept IsSimpleAssociativeContainer = requires(Ty container, typename Ty::value_type value) {
        typename Ty::key_type;
        typename Ty::value_type;
        requires std::default_initializable<Ty>;
        requires requires { { container.insert(value) } -> std::same_as<std::pair<typename Ty::iterator, bool>>; } ||
                 requires { { container.insert(value) } -> std::same_as<typename Ty::iterator>; };
    } && !requires { typename Ty::mapped_type; };

    template <typename Ty>
    concept IsSequenceLikeContainer = IsSequentialContainer<Ty> || IsSimpleAssociativeContainer<Ty>;

    template <typename Ty>
    concept IsMapContainer = requires(Ty container, typename Ty::value_type value) {
        typename Ty::key_type;
        typename Ty::mapped_type;
        requires std::default_initializable<Ty>;
        { container.insert(value) } -> std::same_as<std::pair<typename Ty::iterator, bool>>;
    };

    // implement in hpp because a MSVC bug
    template <IsSequenceLikeContainer Ty>
    struct LexicalCast<std::string, Ty> {
        Ty operator()(std::string_view str) {
            Ty container{};
            YAML::Node node = YAML::Load(std::string(str));
            if (!node.IsSequence()) {
                throw std::invalid_argument("YAML content is not a sequence for a sequence-like container.");
            }

            for (const auto& item_node : node) {
                if constexpr (IsSequentialContainer<Ty>) {
                    container.push_back(item_node.as<typename Ty::value_type>());
                } else {
                    container.insert(item_node.as<typename Ty::value_type>());
                }
            }

            return container;
        }
    };

    template <IsMapContainer Ty>
    struct LexicalCast<std::string, Ty> {
        Ty operator()(std::string_view str) {
            Ty container{};
            YAML::Node node = YAML::Load(std::string(str));
            if (!node.IsMap()) {
                throw std::invalid_argument("YAML content is not a map for a map-like container.");
            }

            for (const auto& it : node) {
                container.insert(std::make_pair(it.first.as<typename Ty::key_type>(), it.second.as<typename Ty::mapped_type>()));
            }

            return container;
        }
    };

    template <typename Ty>
    struct LexicalCast<std::span<const Ty>, std::string> {
        std::string operator()(std::span<const Ty> value);
    };

    template <IsSequenceLikeContainer Ty>
    struct LexicalCast<Ty, std::string> {
        std::string operator()(const Ty& value) {
            YAML::Node node(YAML::NodeType::Sequence);
            for (const auto& item : value) {
                node.push_back(item);
            }

            YAML::Emitter emitter;
            emitter << node;
            return emitter.c_str();
        }
    };

    template <IsMapContainer Ty>
    struct LexicalCast<Ty, std::string> {
        std::string operator()(const Ty& value) {
            YAML::Node node(YAML::NodeType::Map);
            for (const auto& pair : value) {
                node[pair.first] = pair.second;
            }

            YAML::Emitter emitter;
            emitter << node;
            return emitter.c_str();
        }
    };

    class ConfigBase {
    public:
        ConfigBase(std::string_view name, std::string_view description);
        virtual ~ConfigBase() = default;

        virtual bool FromString(std::string_view str) = 0;
        virtual std::string ToString() const          = 0;
        virtual std::string GetTypeName() const       = 0;

        const std::string& name() const;
        const std::string& description() const;

    protected:
        std::string name_;
        std::string description_;
    };

    template <typename ValueType,
              typename FromStrPred = LexicalCast<std::string, ValueType>,
              typename ToStrPred   = LexicalCast<ValueType, std::string>>
    class ConfigVar : public ConfigBase {
    public:
        using OnChangeCallback = std::function<void(const ValueType& old_value, const ValueType& new_value)>;

        ConfigVar(std::string_view name, const ValueType& default_value, std::string_view description = "");

        bool FromString(std::string_view str) override;
        std::string ToString() const override;
        std::string GetTypeName() const override;

        void AddListener(std::uint64_t key, OnChangeCallback callback);
        void RemoveListener(std::uint64_t key);
        const OnChangeCallback& GetListener(std::uint64_t key) const;

        ValueType value() const;
        void set_value(const ValueType& value);

    private:
        std::unordered_map<std::uint64_t, OnChangeCallback> callbacks_;
        ValueType                                           value_;
    };

    class Config {
    public:
        using ConfigVarMap = StringHeteroHashTable<std::string, std::shared_ptr<ConfigBase>>;
        using PendingMap   = StringHeteroHashTable<std::string, YAML::Node>;

        static void LoadFromFile(std::string_view filename);
        static void LoadFromYaml(const YAML::Node& root);

        template <typename ValueType>
        static std::shared_ptr<ConfigVar<ValueType>>
        Lookup(std::string_view name, const ValueType& default_value, std::string_view description = "");

        template <typename ValueType>
        static std::shared_ptr<ConfigVar<ValueType>> Lookup(std::string_view name);

        static std::shared_ptr<ConfigBase> LookupBase(std::string_view name);
        static void ForEach(std::function<void(std::shared_ptr<ConfigBase>)> callback);

    private:
        static ConfigVarMap&      ConfigVarData();
        static PendingMap&        PendingData();
        static std::shared_mutex& MainMutex();
        static std::shared_mutex& PendingMutex();
    };
}

#include "Config.inl"
