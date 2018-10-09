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
        { "stencilWriteMask", 0 },
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
        { "geometryShader", "" },
        { "fragmentShader", "" }
    };

    std::vector<std::string> required_graphics_pipeline_fields = {
        "name",
        "pass",
        "vertexFields",
        "vertexShader"
    };

    nlohmann::json default_texture_format = {
            { "pixelFormat", "RGBA8" },
            { "dimensionType", "Absolute"}
    };
    
    void ensure_field_exists(nlohmann::json &j, const std::string &field_name, const std::string &context, const nlohmann::json &default_value, validation_report &report);

#define PIPELINE_MSG(name, msg) "Pipeline " + name + ": " + msg

    validation_report validate_graphics_pipeline(nlohmann::json& pipeline_json) {
        validation_report report;
        const std::string name = get_json_value<std::string>(pipeline_json, "name", "<NAME_MISSING>");
        if(name == "<NAME_MISSING>") {
            report.errors.emplace_back(PIPELINE_MSG(name, "Missing field name"));
        }

        const std::string pipeline_context = "Pipeline " + name;
        // Check non-required fields first 
        for(const auto& str : default_graphics_pipeline.items()) {
            ensure_field_exists(pipeline_json, str.key(), pipeline_context, default_graphics_pipeline, report);
        }

        // Check required items
        report.errors.reserve(required_graphics_pipeline_fields.size());
        for(const std::string& field_name : required_graphics_pipeline_fields) {
            const auto& itr = pipeline_json.find(field_name);
            if(itr == pipeline_json.end()) {
                report.errors.emplace_back(PIPELINE_MSG(name, "Missing field " + field_name));
            }
        }

        return report;
    }


#define RESOURCES_MSG(msg) "Resources file: " + msg

    void validate_shaderpack_resources_data(nlohmann::json& resources_json) {
        validation_report report;
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
                    const validation_report texture_report = validate_texture_data(tex);
                    report.merge_in(texture_report);
                }
            }
        }

        if(missing_textures) {
            report.warnings.emplace_back(RESOURCES_MSG("Missing dynamic resources. If you ONLY use the backbuffer in your shaderpack, you can ignore this message"));
        }

        const auto& samplers_itr = resources_json.find("samplers");
        if(samplers_itr == resources_json.end()) {
            if(!missing_textures) {
                report.errors.emplace_back(RESOURCES_MSG("No samplers defined, but dynamic textures are defined. You need to define your own samplers to access a texture with"));
            }

        } else {
            for(auto& sampler : *samplers_itr) {
                const validation_report sampler_report = validate_sampler_data(sampler);
                report.merge_in(validation_report);
            }
        }

        return report;
    }

#define TEXTURE_MSG(name, msg) "Texture " + name + ": " + msg

    validation_report validate_texture_data(nlohmann::json& texture_json) {
        validation_report report;
        const auto name_maybe = get_json_value<std::string>(texture_json, "name");
        std::string name;
        if(name_maybe) {
            name = name_maybe.value();

        } else {
            name = "<NAME_MISSING>";
            texture_json["name"] = name;
            report.errors.emplace_back(TEXTURE_MSG(name, "Missing field name"));
        }

        const auto format_itr = texture_json.find("format");
        if(format_itr == texture_json.end()) {
            report.errors.emplace_back(TEXTURE_MSG(name, "Missing field format"));

        } else {
            const validation_report format_report = validate_texture_format(*format_itr, name);
            report.merge_in(format_report);
        }

        return report;
    }

#define FORMAT_MSG(tex_name, msg) "Format of texture " + tex_name + ": " + msg

    validation_report validate_texture_format(nlohmann::json& format_json, const std::string& texture_name) {
        validation_report report;

        ensure_field_exists(format_json, "pixelFormat", "Format of texture " + texture_name, default_texture_format, report);
        ensure_field_exists(format_json, "dimensionType", "Format of texture " + texture_name, default_texture_format, report);

        const bool missing_width = format_json.find("width") == format_json.end();
        if(missing_width) {
            report.errors.emplace_back(FORMAT_MSG(texture_name, "Missing field width"));
        }

        const bool missing_height = format_json.find("height") == format_json.end();
        if(missing_height) {
            report.errors.emplace_back(FORMAT_MSG(texture_name, "Missing field height"));
        }

        return report;
    }

#define SAMPLER_MSG(name, msg) "Sampler " + name + ": " + msg

    validation_report validate_sampler_data(nlohmann::json& sampler_json) {
        validation_report report;
        const std::string name = get_json_value<std::string>(sampler_json, "name", "<NAME MISSING>");
        if(name == "<NAME_MISSING>") {
            report.errors.emplace_back(SAMPLER_MSG(name, "Missing field name"));
        }

        const bool missing_filter = sampler_json.find("filter") == sampler_json.end();
        if(missing_filter) {
            report.errors.emplace_back(SAMPLER_MSG(name, "Missing field filter"));
        }

        const bool missing_wrap_mode = sampler_json.find("wrapMode") == sampler_json.end();
        if(missing_wrap_mode) {
            report.errors.emplace_back(SAMPLER_MSG(name, "Missing field wrapMode"));
        }

        return report;
    }


#define MATERIAL_MSG(name, error) "Material " + name + ": " + error
#define MATERIAL_PASS_MSG(mat_name, pass_name, error) "Material pass " + pass_name + " in material " + mat_name + ": " + error
    
    std::vector<std::string> validate_material(nlohmann::json& material_json) {
        validation_report report;
        
        const std::string name = get_json_value<std::string>(material_json, "name", "<NAME_MISSING>");
        if(name == "<NAME_MISSING>") {
            report.errors.emplace_back(MATERIAL_MSG(name, "Missing material name"));
        }
        
        const bool missing_geometry_filter = material_json.find("filter") == material_json.end();
        if(missing_geometry_filter) {
            report.errors.emplace_back(MATERIAL_MSG(name, "Missing geometry filter"));
        }
        
        const auto passes_maybe = get_json_value(material_json, "passes");
        bool missing_passes = !passes_maybe;
        if(missing_passes) {
            report.errors.emplace_back(MATERIAL_MSG(name, "Missing material passes"));
        }
        
        if(passes_maybe) {
            const nlohmann::json& passes_json = passes_maybe.value();
            if(!passes_json.is_array()) {
                report.errors.emplace_back(MATERIAL_MSG(name, "Passes field must be an array"));
                
            } else if(passes_json.empty()) {
                report.errors.emplace_back(MATERIAL_MSG(name, "Passes field must have at least one item"));
            }
            
            for(const auto& pass_json : passes_json) {
                const std::string pass_name = get_json_value<std::string>(pass_json, "name", "<NAME_MISSING>");
                if(pass_name == "<NAME_MISSING>") {
                    report.errors.emplace_back(MATERIAL_PASS_MSG(name, pass_name, "Missing field name"));
                }

                const auto pipeline_maybe = get_json_value<std::string>(pass_json, "pipeline");
                if(!pipeline_maybe) {
                    report.errors.emplace_back(MATERIAL_PASS_MSG(name, pass_name, "Missing field pipeline"));
                }
                
                const auto bindings_maybe = get_json_value<std::unordered_map<std::string, std::string>>(pass_json, "bindings");
                if(!bindings_maybe) {
                    report.warnings.emplace_back(MATERIAL_PASS_MSG(name, pass_name, "No bindings defined"));
                }
            }
        }
        
        return validation_errors;
    }

    void ensure_field_exists(nlohmann::json& j, const std::string& field_name, const std::string& context, const nlohmann::json& default_value, validation_report& report) {
        if(j.find(field_name) == j.end()) {
            j[field_name] = default_value[field_name];
            report.warnings.emplace_back(context + ": Missing field " + field_name + ". A default value of '" + j[field_name].get<std::string>() + "' will be used");
        }
    }

    void print(const validation_report& report) {
        for(const auto& error : report.errors) {
            NOVA_LOG(ERROR) << error;
        }

        for(const auto& warning : report.warnings) {
            NOVA_LOG(WARN) << warning;
        }
    }

    void validation_report::merge_in(const validation_report& other) {
        errors.insert(errors.end(), other.errors.begin(), other.errors.end());
        warnings.insert(warnings.begin(), other.warnings.begin(), other.warnings.end());
    }
}
