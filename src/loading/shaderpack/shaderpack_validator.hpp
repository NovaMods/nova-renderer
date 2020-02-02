#pragma once

#include <rx/core/json.h>
#include <rx/core/string.h>
#include <rx/core/vector.h>

namespace nova::renderer::shaderpack {
    struct ValidationReport {
        rx::vector<rx::string> warnings;
        rx::vector<rx::string> errors;

        void merge_in(const ValidationReport& other);
    };

    void print(const ValidationReport& report);

    /*!
     * \brief Checks if the pipeline_json has all the properties that a full pipeline should have, printing a debug
     * message if it doesn't. Missing fields that can be filled in with a default value are filled in (pipeline_json is
     * modified) and a debug message is logged, but if a missing field is required then a `json_validation_error`
     * exception is thrown
     *
     * \param pipeline_json The JSON pipeline to validate and possibly fill in
     */
    ValidationReport validate_graphics_pipeline(rx::json& pipeline_json);

    /*!
     * \brief Validates the dynamic resources that the given JSON file contains. Any warnings about missing fields with
     * default values are sent to the warning logger, while information about missing required fields is sent out as an
     * exception
     *
     * \param resources_json The JSON dynamic resources to validate
     */
    ValidationReport validate_shaderpack_resources_data(rx::json& resources_json);

    /*!
     * \brief Validates a single texture's JSON, printing warnings to the warning logger and throwing an exception with
     * any missing required fields
     *
     * \param texture_json The JSON of the texture to validate
     */
    ValidationReport validate_texture_data(const rx::json& texture_json);

    /*!
     * \brief Validates a texture format, printing warnings to the warning logger and throwing an exception with any
     * missing required fields
     *
     * \param format_json The JSON to validate
     */
    ValidationReport validate_texture_format(const rx::json& format_json, const rx::string& texture_name);

    /*!
     * \brief Validates that the provided JSON has all the fields it needed. Warnings about optional fields are
     * printed to the warning logger, errors are thrown together in an exception
     *
     * \param sampler_json The JSON to validate
     */
    ValidationReport validate_sampler_data(const rx::json& sampler_json);

    /*!
     * \brief Validates that the provided JSON for has all the fields it needs. Optional fields that are missing
     * generate a warning, required fields that are missing generate an exception
     *
     * \param material_json The JSON to validate
     *
     * \return A list of all errors encountered when validating this material
     */
    ValidationReport validate_material(const rx::json& material_json);
} // namespace nova::renderer::shaderpack
