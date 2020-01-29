/*!
 * \author ddubois
 * \date 05-Sep-18.
 */

#ifndef NOVA_RENDERER_UTILS_HPP
#define NOVA_RENDERER_UTILS_HPP

#include <rx/core/json.h>
#include <rx/core/optional.h>

#include "nova_renderer/util/has_method.hpp"
#include "nova_renderer/util/logger.hpp"

namespace nova::renderer {
    inline rx::optional<rx::string> get_json_string(const rx::json& json, const char* key) {
        const auto val = json[key];
        if(val) {
            return val.as_string();
        } else {
            return rx::nullopt;
        }
    }

    /*!
     * \brief Retrieves an individual value from the provided JSON structure
     * \tparam ValType The type of the value to retrieve
     * \param json_obj The JSON object where your value might be found
     * \param key The name of the value
     * \return An optional that contains the value, if it can be found, or an empty optional if the value cannot be found
     *
     * \note Only enabled if we're not getting a string. Use `get_json_string` to get a string
     */
    template <typename ValType,
              rx::traits::enable_if<has_from_json<ValType>::Has>,
              std::enable_if_t<std::is_same_v<ValType, rx::string>>** = nullptr>
    rx::optional<ValType> get_json_value(const rx::json& json_obj, const char* key) {
        const auto val = json_obj[key];
        if(val) {
            return rx::optional<ValType>(ValType::from_json(json_obj));
        }

        return rx::nullopt;
    }

    /*!
     * \brief Retrieves an individual string value from the provided JSON structure
     * \tparam ValType The type of the value to retrieve (always std::string here)
     * \param json_obj The JSON object where your string might be found
     * \param key The name of the string
     * \param empty_means_not_present If set to true an empty string will be interpreted as not found
     * \return An optional that contains the value, if it can be found, or an empty optional if the value cannot be found
     *
     * \note Special case for strings
     */
    template <typename ValType, std::enable_if_t<std::is_same_v<ValType, rx::string>>** = nullptr>
    rx::optional<ValType> get_json_value(const rx::json& json_obj, const rx::string& key, const bool empty_means_not_present = false) {
        const std::string key_std = key.data();
        const auto& itr = json_obj.find(key_std);
        if(itr != json_obj.end()) {
            const auto str = itr->get<std::string>();
            return (empty_means_not_present && str.empty()) ? rx::nullopt : rx::optional<rx::string>(str.c_str());
        }

        return rx::optional<rx::string>{};
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
    ValType get_json_value(const rx::json& json_obj, const rx::string& key, ValType default_value) {
        const std::string key_std = key.data();
        const auto& itr = json_obj.find(key_std);
        if(itr != json_obj.end()) {
            return itr->get<ValType>();
        }

        NOVA_LOG(TRACE) << key_std << " not found - using a default value";
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
    rx::optional<ValType> get_json_value(const rx::json& json_obj,
                                         const rx::string& key,
                                         std::function<ValType(const rx::json&)> deserializer) {
        const std::string key_std = key.data();
        const auto& itr = json_obj.find(key_std);
        if(itr != json_obj.end()) {
            ValType val = deserializer(*itr);
            return rx::optional<ValType>{std::move(val)};
        }

        return rx::nullopt;
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
    ValType get_json_value(const rx::json& json_obj,
                           const rx::string& key,
                           ValType default_value,
                           std::function<ValType(const rx::json&)> deserializer) {
        const std::string key_std = key.data();
        const auto& itr = json_obj.find(key_std);
        if(itr != json_obj.end()) {
            ValType value = deserializer(*itr);
            return value;
        }

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
    rx::vector<ValType> get_json_array(const rx::json& json_obj, const rx::string& key) {
        const std::string key_std = key.data();
        const auto& itr = json_obj.find(key_std);
        if(itr != json_obj.end()) {
            rx::vector<ValType> vec;
            vec.reserve(itr->size());

            for(const rx::basic_json<>& elem : *itr) {
                vec.push_back(elem.get<ValType>());
            }

            return vec;
        }

        return {};
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
    rx::vector<ValType> get_json_array(const rx::json& json_obj,
                                       const rx::string& key,
                                       std::function<ValType(const rx::json&)> deserializer) {
        const std::string key_std = key.data();
        const auto& itr = json_obj.find(key_std);
        if(itr != json_obj.end()) {
            rx::vector<ValType> vec;
            vec.reserve(itr->size());

            for(auto& elem : *itr) {
                vec.push_back(deserializer(elem));
            }

            return vec;
        }

        return rx::vector<ValType>{};
    }
} // namespace nova::renderer

#endif // NOVA_RENDERER_UTILS_HPP
