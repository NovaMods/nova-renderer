#include "renderpack_validator.hpp"

#include <array>

#include <array>
#include <rx/core/log.h>

#include "nova_renderer/util/utils.hpp"

#include "../json_utils.hpp"

namespace nova::renderer::renderpack {
    RX_LOG("RenderpackValidator", logger);

    constexpr uint32_t NUM_REQUIRED_FIELDS = 23;

    /*!
     * \brief All the default values for a JSON pipeline
     *
     * If a field is in `pipeline_data` but not in this structure, it is a required field and cannot be given a
     * default value. It will thus cause an exception
     */
    const char* required_fields[NUM_REQUIRED_FIELDS] = {"parentName",
                                                        "defines",
                                                        "states",
                                                        "frontFace",
                                                        "backFace",
                                                        "fallback",
                                                        "depthBias",
                                                        "slopeScaledDepthBias",
                                                        "stencilRef",
                                                        "stencilReadMask",
                                                        "stencilWriteMask",
                                                        "msaaSupport",
                                                        "primitiveMode",
                                                        "sourceBlendFactor",
                                                        "destinationBlendFactor",
                                                        "alphaSrc",
                                                        "alphaDst",
                                                        "depthFunc",
                                                        "renderQueue",
                                                        "fragmentShader",
                                                        "tessellationControlShader",
                                                        "tessellationEvaluationShader",
                                                        "geometryShader"};
    ;

    const std::array<std::string[3]> required_graphics_pipeline_fields = {"name", "pass", "vertexShader"};

    const std::array<std::string[2]> required_texture_fields = {"pixelFormat", "dimensionType"};

    void ensure_field_exists(
        nlohmann::json& j, const std::string& field_name, const std::string& context, const nlohmann::json& default_value, ValidationReport& report);

    static std::string pipeline_msg(const std::string& name, const std::string& field_name) {
        return std::string::format("Pipeline %s: Missing field %s", name, field_name);
    }

    ValidationReport validate_graphics_pipeline(nlohmann::json& pipeline_json) {
        ValidationReport report;
        const auto name_json = pipeline_json["name"];
        const auto name = name_json ? name_json.as_string() : "<NAME_MISSING>";
        // Don't need to check for the name's existence here, it'll be checked with the rest of the required fields

        const std::string pipeline_context = std::string::format("Pipeline %s", name);
        // Check non-required fields first
        for(uint32_t i = 0; i < NUM_REQUIRED_FIELDS; i++) {
            if(!pipeline_json[required_fields[i]]) {
                report.warnings.emplace_back(std::string::format("%s: Missing optional field %s", pipeline_context, required_fields[i]));
            }
        }

        // Check required items
        report.errors.reserve(required_graphics_pipeline_fields.size());
        for(uint32_t i = 0; i < required_graphics_pipeline_fields.size(); i++) {
            const auto& field_name = required_graphics_pipeline_fields[i];
            if(!pipeline_json[field_name.data()]) {
                report.errors.emplace_back(pipeline_msg(name, field_name));
            }
        }

        return report;
    }

    static std::string resources_msg(const std::string& msg) { return std::string::format("Resources file: %s", msg); }

    ValidationReport validate_renderpack_resources_data(nlohmann::json& resources_json) {
        ValidationReport report;
        bool missing_textures = false;

        const auto textures_itr = resources_json["textures"];
        if(!textures_itr) {
            missing_textures = true;
        } else {
            if(!textures_itr.is_array() || textures_itr.is_empty()) {
                missing_textures = true;
            } else {
                textures_itr.each([&](const nlohmann::json& tex) {
                    const ValidationReport texture_report = validate_texture_data(tex);
                    report.merge_in(texture_report);
                });
            }
        }

        if(missing_textures) {
            report.warnings.emplace_back(
                resources_msg("Missing dynamic resources. If you ONLY use the backbuffer in your renderpack, you can ignore this message"));
        }

        const nlohmann::json samplers_itr = resources_json["samplers"];
        if(samplers_itr) {
            if(!samplers_itr.is_array()) {
                report.errors.emplace_back(resources_msg("Samplers array must be an array, but like it isn't"));
            } else {
                samplers_itr.each([&](const nlohmann::json& sampler) {
                    const ValidationReport sampler_report = validate_sampler_data(sampler);
                    report.merge_in(sampler_report);
                });
            }
        }

        return report;
    }

    static std::string texture_msg(const std::string& name, const std::string& msg) { return std::string::format("Texture %s: %s", name, msg); }

    ValidationReport validate_texture_data(const nlohmann::json& texture_json) {
        ValidationReport report;
        const auto name_json = texture_json["name"];
        std::string name;
        if(name_json) {
            name = name_json.as_string();
        } else {
            name = "<NAME_MISSING>";
            texture_json["name"] = name.data();
            report.errors.emplace_back(texture_msg(name, "Missing field name"));
        }

        auto format = texture_json["format"];
        if(!format) {
            report.errors.emplace_back(texture_msg(name, "Missing field format"));
        } else {
            const ValidationReport format_report = validate_texture_format(format, name);
            report.merge_in(format_report);
        }

        return report;
    }

    static std::string format_msg(const std::string& tex_name, const std::string& msg) {
        return std::string::format("Format of texture %s: %s", tex_name, msg);
    }

    ValidationReport validate_texture_format(const nlohmann::json& format_json, const std::string& texture_name) {
        ValidationReport report;

        const std::string context = std::string::format("Format of texture %s", texture_name);
        for(uint32_t i = 0; i < required_texture_fields.size(); i++) {
            if(!format_json[required_texture_fields[i].data()]) {
                report.warnings.emplace_back(std::string::format("%s: Missing required field %s", context, required_texture_fields[i]));
            }
        }

        const bool missing_width = !format_json["width"];
        if(missing_width) {
            report.errors.emplace_back(format_msg(texture_name, "Missing field width"));
        }

        const bool missing_height = !format_json["height"];
        if(missing_height) {
            report.errors.emplace_back(format_msg(texture_name, "Missing field height"));
        }

        return report;
    }

    static std::string sampler_msg(const std::string& name, const std::string& msg) { return std::string::format("Sampler %s: %s", name, msg); }

    ValidationReport validate_sampler_data(const nlohmann::json& sampler_json) {
        ValidationReport report;
        const std::string name = get_json_value<std::string>(sampler_json, "name", "<NAME_MISSING>");
        if(name == "<NAME_MISSING>") {
            report.errors.emplace_back(sampler_msg(name, "Missing field name"));
        }

        const bool missing_filter = !sampler_json["filter"];
        if(missing_filter) {
            report.errors.emplace_back(sampler_msg(name, "Missing field filter"));
        }

        const bool missing_wrap_mode = !sampler_json["wrapMode"];
        if(missing_wrap_mode) {
            report.errors.emplace_back(sampler_msg(name, "Missing field wrapMode"));
        }

        return report;
    }

    static std::string material_msg(const std::string& name, const std::string& msg) {
        return std::string::format("Material %s: %s", name, msg);
    }
    static std::string material_pass_msg(const std::string& mat_name, const std::string& pass_name, const std::string& error) {
        return std::string::format("Material pass %s in material %s: %s", pass_name, mat_name, error);
    }

    ValidationReport validate_material(const nlohmann::json& material_json) {
        ValidationReport report;

        const auto name_maybe = material_json["name"];
        std::string name = "<NAME_MISSING>";
        if(!name_maybe) {
            report.errors.emplace_back(material_msg("<NAME_MISSING>", "Missing material name"));
        } else {
            name = name_maybe.as_string();
        }

        const bool missing_geometry_filter = !material_json["filter"];
        if(missing_geometry_filter) {
            report.errors.emplace_back(material_msg(name, "Missing geometry filter"));
        }

        const bool missing_passes = !material_json["passes"];
        if(missing_passes) {
            report.errors.emplace_back(material_msg(name, "Missing material passes"));
        } else {
            const nlohmann::json& passes_json = material_json["passes"];
            if(!passes_json.is_array()) {
                report.errors.emplace_back(material_msg(name, "Passes field must be an array"));
                return report;
            }
            if(passes_json.is_empty()) {
                report.errors.emplace_back(material_msg(name, "Passes field must have at least one item"));
                return report;
            }

            passes_json.each([&](const nlohmann::json& pass_json) {
                const auto pass_name_maybe = pass_json["name"];
                std::string pass_name = "<NAME_MISSING>";
                if(!pass_name_maybe) {
                    report.errors.emplace_back(material_pass_msg(name, pass_name, "Missing field name"));
                } else {
                    pass_name = pass_name_maybe.as_string();
                }

                if(!pass_json["pipeline"]) {
                    report.errors.emplace_back(material_pass_msg(name, pass_name, "Missing field pipeline"));
                }

                const auto bindings = pass_json["bindings"];
                if(!bindings) {
                    report.warnings.emplace_back(material_pass_msg(name, pass_name, "Missing field bindings"));
                } else {
                    if(bindings.is_empty()) {
                        report.warnings.emplace_back(material_pass_msg(name, pass_name, "Field bindings exists but it's empty"));
                    }
                }
            });
        }

        return report;
    }

    void ensure_field_exists(
        nlohmann::json& j, const char* field_name, const std::string& context, const nlohmann::json& default_value, ValidationReport& report) {
        if(!j[field_name]) {
            j[field_name] = default_value[field_name];
            size_t out_size;
            // const char* json_string = reinterpret_cast<const char*>(json_write_minified(j[field_name].raw(), &out_size));
            report.warnings.emplace_back(context + ": Missing field " + field_name + ". A default value will be used");
        }
    }

    void print(const ValidationReport& report) {
        report.errors.each_fwd([&](const std::string& error) { logger->error("%s", error); });

        report.warnings.each_fwd([&](const std::string& warning) { logger->verbose("%s", warning); });
    }

    void ValidationReport::merge_in(const ValidationReport& other) {
        errors += other.errors;
        warnings += other.warnings;
    }
} // namespace nova::renderer::renderpack
