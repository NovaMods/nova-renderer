/*!
 * \brief Functions for determining if a JSON file from a shaderpack is reasonable, and telling the user if any fields
 * need to be filled in
 *
 * \author ddubois 
 * \date 12-Sep-18.
 */

#ifndef NOVA_RENDERER_SHADERPACK_VALIDATOR_HPP
#define NOVA_RENDERER_SHADERPACK_VALIDATOR_HPP

#include <string>
#include <nlohmann/json.hpp>

namespace nova {
    class json_field_missing : public std::runtime_error {
    public:
        explicit json_field_missing(const std::string& field_name);
    };

    /*!
     * \brief Checks if the pipeline_json has all the properties that a full pipeline should have, printing a debug 
     * message if it doesn't. Missing fields that can be filled in with a default value are filled in (pipeline_json is
     * modified) and a debug message is logged, but if a missing field is required then a `json_field_missing` 
     * exception is thrown
     * 
     * \param pipeline_json The JSON pipeline to validate and possibly fill in
     */
    void validate_pipeline(nlohmann::json& pipeline_json);

    void check_if_field_exists(nlohmann::json& j, const std::string& field_name, const std::string& context, const nlohmann::json& default_value);
}

#endif //NOVA_RENDERER_SHADERPACK_VALIDATOR_HPP
