#include "Config.hpp"

#include <charconv>
#include <exception>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <typeinfo>

#include "Base/Logger.hpp"

namespace glsld {
    inline std::string LexicalCast<std::string, std::string>::operator()(std::string_view str) {
        return std::string(str);
    }

    template <IsPod To>
    To LexicalCast<std::string, To>::operator()(std::string_view str) {
        To value{};
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
        if (ec != std::errc() || ptr != str.data() + str.size()) {
            throw std::invalid_argument("Failed to cast string to the target type.");
        }

        return value;
    }

    template <IsPod From>
    std::string LexicalCast<From, std::string>::operator()(const From& value) {
        std::size_t buffer_size = 0;
        if constexpr (std::is_same_v<From, float>) {
            buffer_size = 50;
        } else if constexpr (std::is_same_v<From, double>) {
            buffer_size = 330;
        } else if constexpr (std::is_same_v<From, long double>) {
            buffer_size = 4955;
        } else {
            buffer_size = std::numeric_limits<From>::digits10 + 2;
        }

        std::string result(buffer_size, '\0');
        auto [ptr, ec] = std::to_chars(result.data(), result.data() + buffer_size, value);
        if (ec == std::errc()) {
            result.resize(ptr - result.data());
            return result;
        } else {
            throw std::invalid_argument("Failed to perform lexical cast to string.");
        }
    }

    // template <IsSequenceLikeContainer Ty>
    // Ty LexicalCast<std::string, Ty>::operator()(std::string_view str) {
    //     Ty container{};
    //     YAML::Node node = YAML::Load(std::string(str));
    //     if (!node.IsSequence()) {
    //         throw std::invalid_argument("YAML content is not a sequence for a sequence-like container.");
    //     }
    // 
    //     for (const auto& item_node : node) {
    //         if constexpr (IsSequentialContainer<Ty>) {
    //             container.push_back(item_node.as<typename Ty::value_type>());
    //         } else {
    //             container.insert(item_node.as<typename Ty::value_type>());
    //         }
    //     }
    // 
    //     return container;
    // }

    // template <IsMapContainer Ty>
    // Ty LexicalCast<std::string, Ty>::operator()(std::string_view str) {
    //     Ty container{};
    //     YAML::Node node = YAML::Load(std::string(str));
    //     if (!node.IsMap()) {
    //         throw std::invalid_argument("YAML content is not a map for a map-like container.");
    //     }
    // 
    //     for (const auto& it : node) {
    //         container.insert(std::make_pair(it.first.as<typename Ty::key_type>(), it.second.as<typename Ty::mapped_type>()));
    //     }
    // 
    //     return container;
    // }

    template <typename Ty>
    std::string LexicalCast<std::span<const Ty>, std::string>::operator()(std::span<const Ty> value) {
        YAML::Node node(YAML::NodeType::Sequence);
        for (const auto& item : value) {
            node.push_back(item);
        }

        YAML::Emitter emitter;
        emitter << node;
        return emitter.c_str();
    }

    // template <IsSequenceLikeContainer Ty>
    // std::string LexicalCast<Ty, std::string>::operator()(const Ty& value) {
    //     YAML::Node node(YAML::NodeType::Sequence);
    //     for (const auto& item : value) {
    //         node.push_back(item);
    //     }
    // 
    //     YAML::Emitter emitter;
    //     emitter << node;
    //     return emitter.c_str();
    // }

    // template <IsMapContainer Ty>
    // std::string LexicalCast<Ty, std::string>::operator()(const Ty& value) {
    //     YAML::Node node(YAML::NodeType::Map);
    //     for (const auto& pair : value) {
    //         node[pair.first] = pair.second;
    //     }
    // 
    //     YAML::Emitter emitter;
    //     emitter << node;
    //     return emitter.c_str();
    // }

    inline const std::string& ConfigBase::name() const {
        return name_;
    }

    inline const std::string& ConfigBase::description() const {
        return description_;
    }

    template <typename ValueType, typename FromStrPred, typename ToStrPred>
    inline ConfigVar<ValueType, FromStrPred, ToStrPred>::ConfigVar(std::string_view name, const ValueType& default_value, std::string_view description)
        : ConfigBase(name, description)
        , value_(default_value)
    {}

    template <typename ValueType, typename FromStrPred, typename ToStrPred>
    bool ConfigVar<ValueType, FromStrPred, ToStrPred>::FromString(std::string_view str) {
        try {
            set_value(FromStrPred()(str));
            return true;
        } catch (const std::exception& e) {
            GLSLD_LOG_ERROR(GLSLD_LOG_ROOT(), "ConfigVar::FromString() exception for name='{}', value='{}': {}",
                            name(), str, e.what());
            return false;
        }
    }

    template <typename ValueType, typename FromStrPred, typename ToStrPred>
    inline std::string ConfigVar<ValueType, FromStrPred, ToStrPred>::ToString() const {
        return ToStrPred()(value_);
    }

    template <typename ValueType, typename FromStrPred, typename ToStrPred>
    inline std::string ConfigVar<ValueType, FromStrPred, ToStrPred>::GetTypeName() const {
        return typeid(ValueType).name();
    }

    template <typename ValueType, typename FromStrPred, typename ToStrPred>
    inline void ConfigVar<ValueType, FromStrPred, ToStrPred>::AddListener(std::uint64_t key, OnChangeCallback callback) {
        callbacks_[key] = std::move(callback);
    }

    template <typename ValueType, typename FromStrPred, typename ToStrPred>
    inline void ConfigVar<ValueType, FromStrPred, ToStrPred>::RemoveListener(std::uint64_t key) {
        callbacks_.erase(key);
    }

    template <typename ValueType, typename FromStrPred, typename ToStrPred>
    inline const ConfigVar<ValueType, FromStrPred, ToStrPred>::OnChangeCallback&
    ConfigVar<ValueType, FromStrPred, ToStrPred>::GetListener(std::uint64_t key) const {
        auto it = callbacks_.find(key);
        return it == callbacks_.end() ? nullptr : it->second;
    }

    template <typename ValueType, typename FromStrPred, typename ToStrPred>
    inline ValueType ConfigVar<ValueType, FromStrPred, ToStrPred>::value() const {
        return value_;
    }

    template <typename ValueType, typename FromStrPred, typename ToStrPred>
    void ConfigVar<ValueType, FromStrPred, ToStrPred>::set_value(const ValueType& value) {
        if (value == value_) {
            return;
        }

        for (auto& [_, callback] : callbacks_) {
            callback(value_, value);
        }

        value_ = value;
    }

    template <typename ValueType>
    std::shared_ptr<ConfigVar<ValueType>>
    Config::Lookup(std::string_view name, const ValueType& default_value, std::string_view description) {
        if (name.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos) {
            GLSLD_LOG_ERROR(GLSLD_LOG_ROOT(), "Lookup config name={} invalid", name);
            throw std::invalid_argument(std::string(name));
        }

        {
            std::shared_lock lock(MainMutex());
            auto it = ConfigVarData().find(name);
            if (it != ConfigVarData().end()) {
                auto typed_var = Lookup<ValueType>(name);
                if (typed_var != nullptr) {
                    GLSLD_LOG_INFO(GLSLD_LOG_ROOT(), "Lookup config name={} already exists", name);
                    return typed_var;
                } else {
                    GLSLD_LOG_ERROR(GLSLD_LOG_ROOT(), "Lookup config name={} exists but type not {}, real_type={}",
                                    name, typeid(ValueType).name(), it->second->GetTypeName());
                    return nullptr;
                }
            }
        }

        YAML::Node pending_node;
        bool found_in_pending = false;
        {
            std::unique_lock lock(PendingMutex());
            auto it = PendingData().find(name);
            if (it != PendingData().end()) {
                pending_node = std::move(it->second);
                found_in_pending = true;
                PendingData().erase(it);
            }
        }

        std::unique_lock lock(MainMutex());
        auto it = ConfigVarData().find(name);
        if (it != ConfigVarData().end()) {
            return std::dynamic_pointer_cast<ConfigVar<ValueType>>(it->second);
        }

        ValueType final_value = default_value;
        if (found_in_pending) {
            try {
                final_value = pending_node.as<ValueType>();
                GLSLD_LOG_INFO(GLSLD_LOG_ROOT(), "Load config name={} from pending table", name);
            } catch (const std::exception& e) {
                GLSLD_LOG_ERROR(GLSLD_LOG_ROOT(), "Failed to cast pending YAML node for key '{}': {}", name, e.what());
            }
        }

        auto new_var = std::make_shared<ConfigVar<ValueType>>(name, final_value, description);
        ConfigVarData().emplace(name, new_var);
        return new_var;
    }

    template <typename ValueType>
    std::shared_ptr<ConfigVar<ValueType>> Config::Lookup(std::string_view name) {
        std::shared_lock lock(MainMutex());
        auto it = ConfigVarData().find(name);;
        return it == ConfigVarData().end() ? nullptr : std::dynamic_pointer_cast<ConfigVar<ValueType>>(it->second);
    }

    inline Config::ConfigVarMap& Config::ConfigVarData() {
        static Config::ConfigVarMap data;
        return data;
    }

    inline Config::PendingMap& Config::PendingData() {
        static Config::PendingMap data;
        return data;
    }

    inline std::shared_mutex& Config::MainMutex() {
        static std::shared_mutex mutex;
        return mutex;
    }

    inline std::shared_mutex& Config::PendingMutex() {
        static std::shared_mutex mutex;
        return mutex;
    }
} // namespace valky
