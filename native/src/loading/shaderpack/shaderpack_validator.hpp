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
    /*!
     * \brief Checks if the pipeline_json has all the properties that a full pipeline should have, printing a debug 
     * message if it doesn't. Missing fields that can be filled in with a default value are filled in (pipeline_json is
     * modified) and a debug message is logged, but if a missing field is required then a `json_validation_error` 
     * exception is thrown
     * 
     * \param pipeline_json The JSON pipeline to validate and possibly fill in
     */
    void validate_graphics_pipeline(nlohmann::json& pipeline_json);

    /*!
     * \brief Validates the dynamic resources that the given JSON file contains. Any warnings about missing fields with 
     * default values are sent to the warning logger, while information about missing required fields is sent out as an 
     * exception
     * 
     * \param resources_json The JSON dynamic resources to validate
     */
    void validate_shaderpack_resources_data(nlohmann::json& resources_json);

    /*!
     * \brief Validates a single texture's JSON, printing warnings to the warning logger and throwing an exception with
     * any missing required fields
     * 
     * \param texture_json The JSON of the texture to validate
     */
    void validate_texture_data(nlohmann::json& texture_json);

    /*!
     * \brief Validates a texture format, printing warnings to the warning logger and throwing an exception with any 
     * missing required fields
     * 
     * \param format_json The JSON to validate
     */
    void validate_texture_format(nlohmann::json& format_json);

    /*!
     * \brief Validates that the provided JSON has all the fields it needed. Warnings about optional fields are 
     * printed to the warning logger, errors are thrown together in an exception
     * 
     * \param sampler_json The JSON to validate
     */
    void validate_sampler_data(nlohmann::json& sampler_json);
}

#endif //NOVA_RENDERER_SHADERPACK_VALIDATOR_HPP
