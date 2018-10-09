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
    struct validation_report {
        std::vector<std::string> missing_required_fields;
        std::vector<std::string> missing_optional_fields;
    };

    /*!
     * \brief Checks if the pipeline_json has all the properties that a full pipeline should have, returning a report on
     * which fields are missing from the input JSON
     * 
     * \param pipeline_json The JSON pipeline to validate and possibly fill in
     */
    validation_report validate_graphics_pipeline(nlohmann::json& pipeline_json);

    /*!
     * \brief Validates the dynamic resources that the given JSON file contains, returning a report on which fields are missing from the input JSON
     * 
     * \param resources_json The JSON dynamic resources to validate
     */
    validation_report validate_shaderpack_resources_data(nlohmann::json& resources_json);

    /*!
     * \brief Validates a single texture's JSON, returning a report on which fields are missing from the input JSON
     * 
     * \param texture_json The JSON of the texture to validate
     */
    validation_report validate_texture_data(nlohmann::json& texture_json);

    /*!
     * \brief Validates a texture format, returning a report on which fields are missing from the input JSON
     * 
     * \param format_json The JSON to validate
     */
    validation_report validate_texture_format(nlohmann::json& format_json);

    /*!
     * \brief Validates that the provided JSON has all the fields it needed, returning a report on which fields are
     * missing from the input JSON
     * 
     * \param sampler_json The JSON to validate
     */
    validation_report validate_sampler_data(nlohmann::json& sampler_json);

    /*!
     * \brief Validates that the provided JSON for has all the fields it needs, returning a report on which fields are
     * missing from the input JSON
     *
     * \param material_json The JSON to validate
     */
    validation_report validate_material(nlohmann::json& material_json);
}

#endif //NOVA_RENDERER_SHADERPACK_VALIDATOR_HPP
