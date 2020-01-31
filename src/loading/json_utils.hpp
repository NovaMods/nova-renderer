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
    inline rx::optional<rx::string> get_json_string(const rx::json& json, const char* key, const bool empty_means_not_present = false) {
        const auto val = json[key];
        if(val) {
            const auto str = val.as_string();
            return (empty_means_not_present && str.is_empty()) ? rx::nullopt : rx::optional{str};
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
    template <typename ValType>
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
     *
    template <typename ValType, std::enable_if_t<std::is_same_v<ValType, rx::string>>** = nullptr>
    rx::optional<ValType> get_json_value(const rx::json& json_obj, const rx::string& key, const bool empty_means_not_present = false) {
        const auto& val = json_obj[key.data()];
        if(val) {
            const auto str = val.as_string();
            return (empty_means_not_present && str.is_empty()) ? rx::nullopt : rx::optional<rx::string>(str);
        }

        return rx::nullopt;
    }*/

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
        const auto& val = json_obj[key.data()];
        if(val) {
            return ValType::from_json(val);
        }

        NOVA_LOG(TRACE) << key.data() << " not found - using a default value";

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
    template <typename ValType, typename FuncType>
    rx::optional<ValType> get_json_value(const rx::json& json_obj, const rx::string& key, FuncType&& deserializer) {
        const auto& val_json = json_obj[key];
        if(val_json) {
            ValType val = deserializer(val);
            return rx::optional<ValType>{rx::utility::move(val)};
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
    template <typename ValType, typename FuncType>
    ValType get_json_value(const rx::json& json_obj, const rx::string& key, ValType default_value, FuncType&& deserializer) {
        const auto& val = json_obj[key.data()];
        if(val) {
            ValType value = deserializer(val);
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
        const auto& arr = json_obj[key.data()];
        if(arr && !arr.is_empty()) {
            rx::vector<ValType> vec;
            vec.reserve(arr.size());

            for(uint32_t i = 0; i < arr.size(); i++) {
                vec.push_back(ValType::from_json(arr[i]));
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
    template <typename ValType, typename FuncType>
    rx::vector<ValType> get_json_array(const rx::json& json_obj, const rx::string& key, FuncType&& deserializer) {
        const auto& arr = json_obj[key.data()];
        if(arr && !arr.is_empty()) {
            rx::vector<ValType> vec;
            vec.reserve(arr.size());

            for(uint32_t i = 0; i < arr.size(); i++) {
                vec.push_back(deserializer(arr[i]));
            }

            return vec;
        }

        return {};
    }
} // namespace nova::renderer

#endif // NOVA_RENDERER_UTILS_HPP
