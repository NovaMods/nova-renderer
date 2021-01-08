#pragma once

#include <rx/core/json.h>
#include  <optional>

namespace nova::renderer {
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
    std::optional<ValType> get_json_opt(const nlohmann::json& json_obj, const char* key);

    /*!
     * \brief Retrieves an individual value from the provided JSON structure
     * \tparam ValType The type of the value to retrieve
     * \param json_obj The JSON object where your value might be found
     * \param key The name of the value
     * \param deserializer A function that deserializes the JSON value
     * \return An optional that contains the value, if it can be found, or an empty optional if the value cannot be found
     */
    template <typename ValType, typename FuncType>
    std::optional<ValType> get_json_opt(const nlohmann::json& json_obj, const char* key, FuncType&& deserializer);

    /*!
     * \brief Retrieves an individual value from the provided JSON structure
     * \tparam ValType The type of the value to retrieve
     * \param json_obj The JSON object where your value might be found
     * \param key The name of the value
     * \param default_value The value to use if the requested key isn't present in the JSON
     * \return The value from the JSON if the key exists in the JSON, or `default_value` if it does not
     */
    template <typename ValType>
    ValType get_json_value(const nlohmann::json& json_obj, const char* key, ValType default_value);

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
    ValType get_json_value(const nlohmann::json& json_obj, const char* key, ValType default_value, FuncType&& deserializer);

    /*!
     * \brief Retrieves an array of values from the provided JSON object
     * \tparam ValType The type fo the values in the array
     * \param json_obj The JSON object where the values might be found
     * \param key The name fo the array that has your values
     * \return An array of values, if the value can be found, or an empty vector if the values cannot be found
     */
    template <typename ValType>
    std::vector<ValType> get_json_array(const nlohmann::json& json_obj, const char* key);

    /*!
     * \brief Retrieves an array of values from the provided JSON object
     * \tparam ValType The type fo the values in the array
     * \param json_obj The JSON object where the values might be found
     * \param key The name fo the array that has your values
     * \param deserializer A function that can deserialize each value from JSON
     * \return An array of values, if the value can be found, or an empty vector if the values cannot be found
     */
    template <typename ValType, typename FuncType>
    std::vector<ValType> get_json_array(const nlohmann::json& json_obj, const char* key, FuncType&& deserializer);

    template <typename ValType>
    std::optional<ValType> get_json_opt(const nlohmann::json& json_obj, const char* key) {
        const auto& val_json = json_obj[key];
        if(val_json) {
            return val_json.decode<ValType>({});
        }

        return rx::nullopt;
    }

    template <typename ValType, typename FuncType>
    std::optional<ValType> get_json_opt(const nlohmann::json& json_obj, const char* key, FuncType&& deserializer) {
        const auto& val_json = json_obj[key];
        if(val_json) {
            return deserializer(val_json);
        }

        return rx::nullopt;
    }

    template <typename ValType>
    ValType get_json_value(const nlohmann::json& json_obj, const char* key, ValType default_value) {
        const auto& json_val = json_obj[key];
        if(json_val) {
            return json_val.decode<ValType>(default_value);
        }

        return default_value;
    }

    template <typename ValType, typename FuncType>
    ValType get_json_value(const nlohmann::json& json_obj, const char* key, ValType default_value, FuncType&& deserializer) {
        const auto& val = json_obj[key];
        if(val) {
            ValType value = deserializer(val);
            return value;
        }

        return default_value;
    }

    template <typename ValType>
    std::vector<ValType> get_json_array(const nlohmann::json& json_obj, const char* key) {
        const auto& arr = json_obj[key];
        if(arr && !arr.is_empty()) {
            std::vector<ValType> vec;
            vec.reserve(arr.size());

            for(uint32_t i = 0; i < arr.size(); i++) {
                vec.push_back(arr[i].decode<ValType>({}));
            }

            return vec;
        }

        return {};
    }

    template <typename ValType, typename FuncType>
    std::vector<ValType> get_json_array(const nlohmann::json& json_obj, const char* key, FuncType&& deserializer) {
        const auto& arr = json_obj[key];
        if(arr && !arr.is_empty()) {
            std::vector<ValType> vec;
            vec.reserve(arr.size());

            for(uint32_t i = 0; i < arr.size(); i++) {
                vec.push_back(deserializer(arr[i]));
            }

            return vec;
        }

        return {};
    }
} // namespace nova::renderer
