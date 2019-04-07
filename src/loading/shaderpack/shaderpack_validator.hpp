/*!
 * \brief Functions for determining if a JSON file from a shaderpack is reasonable, and telling the user if any fields
 * need to be filled in
 *
 * \author ddubois
 * \date 12-Sep-18.
 */

#ifndef NOVA_RENDERER_SHADERPACK_VALIDATOR_HPP
#define NOVA_RENDERER_SHADERPACK_VALIDATOR_HPP

#include <nlohmann/json.hpp>
#include <string>

namespace nova::renderer::shaderpack {
    struct validation_report {
        std::vector<std::string> warnings;
        std::vector<std::string> errors;

        void merge_in(const validation_report& other);
    };

    void print(const validation_report& report);

    /*!
     * \brief Checks if the pipeline_json has all the properties that a full pipeline should have, printing a debug
     * message if it doesn't. Missing fields that can be filled in with a default value are filled in (pipeline_json is
     * modified) and a debug message is logged, but if a missing field is required then a `json_validation_error`
     * exception is thrown
     *
     * \param pipeline_json The JSON pipeline to validate and possibly fill in
     */
    validation_report validate_graphics_pipeline(nlohmann::json& pipeline_json);

    /*!
     * \brief Validates the dynamic resources that the given JSON file contains. Any warnings about missing fields with
     * default values are sent to the warning logger, while information about missing required fields is sent out as an
     * exception
     *
     * \param resources_json The JSON dynamic resources to validate
     */
    validation_report validate_shaderpack_resources_data(nlohmann::json& resources_json);

    /*!
     * \brief Validates a single texture's JSON, printing warnings to the warning logger and throwing an exception with
     * any missing required fields
     *
     * \param texture_json The JSON of the texture to validate
     */
    validation_report validate_texture_data(nlohmann::json& texture_json);

    /*!
     * \brief Validates a texture format, printing warnings to the warning logger and throwing an exception with any
     * missing required fields
     *
     * \param format_json The JSON to validate
     */
    validation_report validate_texture_format(nlohmann::json& format_json, const std::string& texture_name);

    /*!
     * \brief Validates that the provided JSON has all the fields it needed. Warnings about optional fields are
     * printed to the warning logger, errors are thrown together in an exception
     *
     * \param sampler_json The JSON to validate
     */
    validation_report validate_sampler_data(nlohmann::json& sampler_json);

    /*!
     * \brief Validates that the provided JSON for has all the fields it needs. Optional fields that are missing
     * generate a warning, required fields that are missing generate an exception
     *
     * \param material_json The JSON to validate
     *
     * \return A list of all errors encountered when validating this material
     */
    validation_report validate_material(nlohmann::json& material_json);
} // namespace nova::renderer

#endif // NOVA_RENDERER_SHADERPACK_VALIDATOR_HPP
