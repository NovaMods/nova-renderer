/*!
 * \author ddubois
 * \date 05-Sep-18.
 */

#ifndef NOVA_RENDERER_UTILS_HPP
#define NOVA_RENDERER_UTILS_HPP

#include <nlohmann/json.hpp>
#include <EASTL/optional.h>
#include "../util/logger.hpp"

namespace nova::renderer {
    // Keeps the compiler happy
    eastl::string to_string(const eastl::string& str);

    /*!
     * \brief Retrieves an individual value from the provided JSON structure
     * \tparam ValType The type of the value to retrieve
     * \param json_obj The JSON object where your value might be found
     * \param key The name of the value
     * \return An optional that contains the value, if it can be found, or an empty optional if the value cannot be found
     */
    template <typename ValType, eastl::enable_if_t<!eastl::is_same_v<ValType, eastl::string>>** = nullptr>
    eastl::optional<ValType> get_json_value(const nlohmann::json& json_obj, const eastl::string& key) {
        const auto& itr = json_obj.find(key.c_str());
        if(itr != json_obj.end()) {
            auto& json_node = json_obj.at(key.c_str());
            return eastl::optional<ValType>(json_node.get<ValType>());
        }

        return eastl::optional<ValType>{};
    }

    /*!
     * \brief Retrieves an individual string value from the provided JSON structure
     * \tparam ValType The type of the value to retrieve (always eastl::string here)
     * \param json_obj The JSON object where your string might be found
     * \param key The name of the string
     * \param empty_means_not_present If set to true an empty string will be interpreted as not found
     * \return An optional that contains the value, if it can be found, or an empty optional if the value cannot be found
     */
    template <typename ValType, eastl::enable_if_t<eastl::is_same_v<ValType, eastl::string>>** = nullptr>
    eastl::optional<ValType> get_json_value(const nlohmann::json& json_obj, const eastl::string& key, bool empty_means_not_present = false) {
        const auto& itr = json_obj.find(key.c_str());
        if(itr != json_obj.end()) {
            auto str = json_obj.at(key.c_str()).get<eastl::string>();
            return (empty_means_not_present && str.empty()) ? eastl::optional<eastl::string>{} : eastl::optional<eastl::string>(str);
        }

        return eastl::optional<eastl::string>{};
    }

    /*!
     * \brief Retrieves an individual value from the provided JSON structure
     * \tparam ValType The type of the value to retrieve
     * \param json_obj The JSON object where your value might be found
     * \param key The name of the value
     * \param default_value The value to use if the requested key isn't present in the JSON
     * \return The value from the JSON if the key exists in the JSON, or `default_value` if it does not
     */
    template <typename ValType>
    ValType get_json_value(const nlohmann::json& json_obj, const eastl::string& key, ValType default_value) {
        const auto& itr = json_obj.find(ke.c_str() y);
        if(itr != json_obj.end()) {
            auto& json_node = json_obj.at(key.c_str());
            return json_node.get<ValType>();
        }

        NOVA_LOG(DEBUG) << key.c_str() << " not found - using a default value";
        return default_value;
    }

    /*!
     * \brief Retrieves an individual value from the provided JSON structure
     * \tparam ValType The type of the value to retrieve
     * \param json_obj The JSON object where your value might be found
     * \param key The name of the value
     * \param deserializer A function that deserializes the JSON value
     * \return An optional that contains the value, if it can be found, or an empty optional if the value cannot be found
     */
    template <typename ValType>
    eastl::optional<ValType> get_json_value(const nlohmann::json& json_obj,
                                          const eastl::string& key,
                                          eastl::function<ValType(const nlohmann::json&)> deserializer) {
        const auto& itr = json_obj.find(key.c_str());
        if(itr != json_obj.end()) {
            auto& json_node = json_obj.at(key.c_str());
            ValType val = deserializer(json_node);
            return eastl::optional<ValType>{eastl::move(val)};
        }

        return eastl::optional<ValType>{};
    }

    /*!
     * \brief Retrieves an individual value from the provided JSON structure
     * \tparam ValType The type of the value to retrieve
     * \param json_obj The JSON object where your value might be found
     * \param key The name of the value
     * \param default_value The value to use if the requested key isn't present in the JSON
     * \param deserializer A function that deserializes the JSON value
     * \return The value from the JSON if the key exists in the JSON, or `default_value` if it does not
     */
    template <typename ValType>
    ValType get_json_value(const nlohmann::json& json_obj,
                           const eastl::string& key,
                           ValType default_value,
                           eastl::function<ValType(const nlohmann::json&)> deserializer) {
        const auto& itr = json_obj.find(key.c_str());
        if(itr != json_obj.end()) {
            auto& json_node = json_obj.at(key.c_str());
            ValType value = deserializer(json_node);
            return value;
        }

        using eastl::to_string;
        NOVA_LOG(DEBUG) << key.c_str() << " not found - defaulting to " << to_string(default_value).c_str();

        return default_value;
    }

    /*!
     * \brief Retrieves an array of values from the provided JSON object
     * \tparam ValType The type fo the values in the array
     * \param json_obj The JSON object where the values might be found
     * \param key The name fo the array that has your values
     * \return An array of values, if the value can be found, or an empty vector if the values cannot be found
     */
    template <typename ValType>
    eastl::vector<ValType> get_json_array(const nlohmann::json& json_obj, const eastl::string& key) {
        const auto& itr = json_obj.find(key.c_str());
        if(itr != json_obj.end()) {
            auto& json_node = json_obj.at(key.c_str());
            eastl::vector<ValType> vec;
            vec.reserve(json_node.size());

            for(auto& elem : json_node) {
                vec.push_back(elem.get<ValType>());
            }

            return vec;
        }

        return eastl::vector<ValType>{};
    }

    /*!
     * \brief Retrieves an array of values from the provided JSON object
     * \tparam ValType The type fo the values in the array
     * \param json_obj The JSON object where the values might be found
     * \param key The name fo the array that has your values
     * \param deserializer A function that can deserialize each value from JSON
     * \return An array of values, if the value can be found, or an empty vector if the values cannot be found
     */
    template <typename ValType>
    eastl::vector<ValType> get_json_array(const nlohmann::json& json_obj,
                                        const eastl::string& key,
                                        eastl::function<ValType(const nlohmann::json&)> deserializer) {
        const auto& itr = json_obj.find(key.c_str());
        if(itr != json_obj.end()) {
            auto& json_node = json_obj.at(key.c_str());
            eastl::vector<ValType> vec;
            vec.reserve(json_node.size());

            for(auto& elem : json_node) {
                vec.push_back(deserializer(elem));
            }

            return vec;
        }

        return eastl::vector<ValType>{};
    }
} // namespace nova::renderer

#endif // NOVA_RENDERER_UTILS_HPP
