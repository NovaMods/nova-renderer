/*!
 * \author ddubois 
 * \date 12-Sep-18.
 */

#include "shaderpack_validator.hpp"
#include "../utils.hpp"
#include "../../util/utils.hpp"
#include "../../../tests/src/general_test_setup.hpp"

namespace nova {
    /*!
     * \brief All the default values for a JSON pipeline
     * 
     * If a field is in `pipeline_data` but not in this structure, it is a required field and cannot be given a 
     * default value. It will thus cause an exception
     */
    nlohmann::json default_graphics_pipeline = {
        { "parentName", "" },
        { "defines", {} },
        { "states", {} },
        { "frontFace", {} },
        { "backFace", {} },
        { "fallback", "" },
        { "depthBias", 0 },
        { "slopeScaledDepthBias", 0 },
        { "stencilRef", 0 },
        { "stencilReadMask", 0 },
        { "msaaSupport", "None" },
        { "primitiveMode", "Triangles" },
        { "sourceBlendFactor", "One" },
        { "destinationBlendFactor", "Zero" },
        { "alphaSrc", "One" },
        { "alphaDst", "Zero" },
        { "depthFunc", "Less" },
        { "renderQueue", "Opaque" },
        { "tessellationControlShader", "" },
        { "tessellationEvaluationShader", "" },
        { "geometryShader", "" }
    };

    std::vector<std::string> required_graphics_pipeline_fields = {
        "name",
        "pass",
        "vertexFields",
        "fragmentShader",
        "geometryShader"
    };
    
    void check_if_field_exists(nlohmann::json& j, const std::string& field_name, const std::string& context, const nlohmann::json& default_value, validation_report& report);

    void validate_graphics_pipeline(nlohmann::json& pipeline_json) {
        const std::string name = get_json_value<std::string>(pipeline_json, "name", "<NAME_MISSING>");

        // Check non-required fields first 
        for(const auto& str : default_graphics_pipeline.items()) {
            check_if_field_exists(pipeline_json, str.key(), name, default_graphics_pipeline, report);
        }

        // Check required items. Collect all the missing fields so we can tell them to the user at the same time
        std::vector<std::string> missing_required_fields;
        missing_required_fields.reserve(required_graphics_pipeline_fields.size());
        for(const std::string& field_name : required_graphics_pipeline_fields) {
            const auto& itr = pipeline_json.find(field_name);
            if(itr == pipeline_json.end()) {
                missing_required_fields.emplace_back("pipeline", context, field_name);
            }
        }

        if(!missing_required_fields.empty()) {
            const auto jonied_fields = join(missing_required_fields, ", ");
            throw validation_failed("Pipeline " + name + " is missing the following required fields: " + joined_fields);
        }
    }


    void validate_shaderpack_resources_data(nlohmann::json& resources_json) {
        bool missing_textures = false;

        const auto& textures_itr = resources_json.find("textures");
        if(textures_itr == resources_json.end()) {
            missing_textures = true;

        } else {
            auto& textures_array = *textures_itr;
            if(textures_array.empty()) {
                missing_textures = true;

            } else {
                for(auto& tex : textures_array) {
                    validate_texture_data(tex);
                }
            }
        }

        if(missing_textures) {
            NOVA_LOG(WARN) << "No dynamic resources defined in this shaderpack. While this might be what you want, it probably isn't";
        }

        const auto& samplers_itr = resources_json.find("samplers");
        if(samplers_itr == resources_json.end()) {
            if(!missing_textures) {
                throw validation_failed("No samplers defined. You need at least one sampler");
            }

        } else {
            for(auto& sampler : *samplers_itr) {
                validate_sampler_data(sampler);
            }
        }
    }


    void validate_texture_data(nlohmann::json& texture_json) {
        const auto name_maybe = get_json_value<std::string>(texture_json, "name");
        std::string name;
        if(name_maybe) {
            name = name_maybe.value();

        } else {
            name = "<NAME MISSING>";
            texture_json["name"] = name;
            report.missing_required_fields.emplace_back("texture", name, "name");
        }

        const auto format_itr = texture_json.find("format");
        if(format_itr == texture_json.end()) {
            missing_format = true;
        }

        if(missing_format) {
            if(missing_name) {
                throw validation_failed("Texture data is missing both its name and its format");

            } else {
                throw validation_failed("Texture " + name + " is missing a format");
            }
        }

        validate_texture_format(*format_itr);
    }

    void validate_texture_format(nlohmann::json& format_json) {
        const std::string pixel_format = get_json_value<std::string>(format_json, "pixelFormat", "RGBA8");
        format_json["pixelFormat"] = pixel_format;

        const std::string dimension_type = get_json_value<std::string>(format_json, "dimensionType", "Absolute");
        format_json["dimensionType"] = dimension_type;

        const bool missing_width = format_json.find("width") == format_json.end();
        const bool missing_height = format_json.find("height") == format_json.end();

        if(missing_width || missing_height) {
            std::stringstream ss;
            ss << "Missing field";

            if(missing_height && missing_width) {
                ss << "s width and height";

            } else if(missing_height) {
                ss << " height";

            } else {
                ss << " width";
            }

            throw validation_failed(ss.str());
        }
    }

    void validate_sampler_data(nlohmann::json& sampler_json) {
        const std::string name = get_json_value<std::string>(sampler_json, "name", "<NAME MISSING>");

        const bool missing_filter = sampler_json.find("filter") == sampler_json.end();
        const bool missing_wrap_mode = sampler_json.find("wrapMode") == sampler_json.end();

        if(missing_filter || missing_wrap_mode) {
            std::stringstream ss;
            ss << "Sampler " << name << " missing field";

            if(missing_filter && missing_wrap_mode) {
                ss << "s filter and wrapMode";

            } else if(missing_filter) {
                ss << " filter";

            } else {
                ss << " wrap mode";
            }

            throw validation_failed(ss.str());
        }
    }

    void check_if_field_exists(nlohmann::json& j, const std::string& field_name, const std::string& context, const nlohmann::json& default_value, validation_report& report) {
        const auto& itr = j.find(field_name);
        if(itr == j.end()) {
            report.missing_optional_fields.push_back(field_name);
            j[field_name] = default_value[field_name];
        }
    }
}
