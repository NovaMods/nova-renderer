#include "shaderpack_validator.hpp"

#include <array>

#include <fmt/format.h>

#include "nova_renderer/util/utils.hpp"

#include "../json_utils.hpp"

namespace nova::renderer::shaderpack {
    // Uses std data types because nlohmann::json uses them. It's not what I want but it's a thing. Maybe one day I'll switch to the Rex
    // JSON thingie

    /*!
     * \brief All the default values for a JSON pipeline
     *
     * If a field is in `pipeline_data` but not in this structure, it is a required field and cannot be given a
     * default value. It will thus cause an exception
     */
    nlohmann::json default_graphics_pipeline = {{"parentName", ""},
                                                {"defines", std::array<std::string, 0>{}},
                                                {"states", std::array<std::string, 0>{}},
                                                {"frontFace", nlohmann::json::object_t()},
                                                {"backFace", nlohmann::json::object_t()},
                                                {"fallback", ""},
                                                {"depthBias", 0},
                                                {"slopeScaledDepthBias", 0},
                                                {"stencilRef", 0},
                                                {"stencilReadMask", 0},
                                                {"stencilWriteMask", 0},
                                                {"msaaSupport", "None"},
                                                {"primitiveMode", "Triangles"},
                                                {"sourceBlendFactor", "One"},
                                                {"destinationBlendFactor", "Zero"},
                                                {"alphaSrc", "One"},
                                                {"alphaDst", "Zero"},
                                                {"depthFunc", "Less"},
                                                {"renderQueue", "Opaque"},
                                                {"fragmentShader", ""},
                                                {"tessellationControlShader", ""},
                                                {"tessellationEvaluationShader", ""},
                                                {"geometryShader", ""}};

    rx::vector<rx::string> required_graphics_pipeline_fields = {"name", "pass", "vertexShader"};

    nlohmann::json default_texture_format = {{"pixelFormat", "RGBA8"}, {"dimensionType", "Absolute"}};

    void ensure_field_exists(nlohmann::json& j,
                             const rx::string& field_name,
                             const rx::string& context,
                             const nlohmann::json& default_value,
                             ValidationReport& report);

    static rx::string pipeline_msg(const rx::string& name, const rx::string& field_name) {
        return rx::string::format("Pipeline %s: Missing field %s", name, field_name);
    }

    ValidationReport validate_graphics_pipeline(nlohmann::json& pipeline_json) {
        ValidationReport report;
        const std::string name = get_json_value<std::string>(pipeline_json, "name").value_or("<NAME_MISSING>");
        // Don't need to check for the name's existence here, it'll be checked with the rest of the required fields

        const rx::string pipeline_context = rx::string::format("Pipeline %s", name.c_str());
        // Check non-required fields first
        for(const auto& str : default_graphics_pipeline.items()) {
            ensure_field_exists(pipeline_json, str.key().c_str(), pipeline_context, default_graphics_pipeline, report);
        }

        // Check required items
        report.errors.reserve(required_graphics_pipeline_fields.size());
        required_graphics_pipeline_fields.each_fwd([&](const rx::string& field_name) {
            const auto& itr = pipeline_json.find(field_name.data());
            if(itr == pipeline_json.end()) {
                report.errors.emplace_back(pipeline_msg(name.c_str(), field_name));
            }
        });

        return report;
    }

    static rx::string resources_msg(const rx::string& msg) { return rx::string::format("Resources file: %s", msg); }

    ValidationReport validate_shaderpack_resources_data(nlohmann::json& resources_json) {
        ValidationReport report;
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
                    const ValidationReport texture_report = validate_texture_data(tex);
                    report.merge_in(texture_report);
                }
            }
        }

        if(missing_textures) {
            report.warnings.emplace_back(
                resources_msg("Missing dynamic resources. If you ONLY use the backbuffer in your shaderpack, you can ignore this message"));
        }

        const nlohmann::json::iterator& samplers_itr = resources_json.find("samplers");
        if(samplers_itr != resources_json.end()) {
            nlohmann::json& all_samplers = *samplers_itr;
            if(!all_samplers.is_array()) {
                report.errors.emplace_back(resources_msg("Samplers array must be an array, but like it isn't"));
            } else {
                for(nlohmann::json& sampler : all_samplers) {
                    const ValidationReport sampler_report = validate_sampler_data(sampler);
                    report.merge_in(sampler_report);
                }
            }
        }

        return report;
    }

    static rx::string texture_msg(const rx::string& name, const rx::string& msg) { return rx::string::format("Texture %s: %s", name, msg); }

    ValidationReport validate_texture_data(nlohmann::json& texture_json) {
        ValidationReport report;
        const auto name_maybe = get_json_value<std::string>(texture_json, "name");
        rx::string name;
        if(name_maybe) {
            name = name_maybe->c_str();
        } else {
            name = "<NAME_MISSING>";
            texture_json["name"] = name.data();
            report.errors.emplace_back(texture_msg(name, "Missing field name"));
        }

        const auto format_itr = texture_json.find("format");
        if(format_itr == texture_json.end()) {
            report.errors.emplace_back(texture_msg(name, "Missing field format"));
        } else {
            const ValidationReport format_report = validate_texture_format(*format_itr, name);
            report.merge_in(format_report);
        }

        return report;
    }

    static rx::string format_msg(const rx::string& tex_name, const rx::string& msg) {
        return rx::string::format("Format of texture %s: %s", tex_name, msg);
    }

    ValidationReport validate_texture_format(nlohmann::json& format_json, const rx::string& texture_name) {
        ValidationReport report;

        const rx::string context = rx::string::format("Format of texture %s", texture_name);
        ensure_field_exists(format_json, "pixelFormat", context, default_texture_format, report);
        ensure_field_exists(format_json, "dimensionType", context, default_texture_format, report);

        const bool missing_width = format_json.find("width") == format_json.end();
        if(missing_width) {
            report.errors.emplace_back(format_msg(texture_name, "Missing field width"));
        }

        const bool missing_height = format_json.find("height") == format_json.end();
        if(missing_height) {
            report.errors.emplace_back(format_msg(texture_name, "Missing field height"));
        }

        return report;
    }

    static rx::string sampler_msg(const rx::string& name, const rx::string& msg) { return rx::string::format("Sampler %s: %s", name, msg); }

    ValidationReport validate_sampler_data(nlohmann::json& sampler_json) {
        ValidationReport report;
        const rx::string name = get_json_value<std::string>(sampler_json, "name").value_or("<NAME_MISSING>").c_str();
        if(name == "<NAME_MISSING>") {
            report.errors.emplace_back(sampler_msg(name, "Missing field name"));
        }

        const bool missing_filter = sampler_json.find("filter") == sampler_json.end();
        if(missing_filter) {
            report.errors.emplace_back(sampler_msg(name, "Missing field filter"));
        }

        const bool missing_wrap_mode = sampler_json.find("wrapMode") == sampler_json.end();
        if(missing_wrap_mode) {
            report.errors.emplace_back(sampler_msg(name, "Missing field wrapMode"));
        }

        return report;
    }

    static rx::string material_msg(const rx::string& name, const rx::string& msg) {
        return rx::string::format("Material %s: %s", name, msg);
    }
    static rx::string material_pass_msg(const rx::string& mat_name, const rx::string& pass_name, const rx::string& error) {
        return rx::string::format("Material pass %s in material %s: %s", pass_name, mat_name, error);
    }

    ValidationReport validate_material(nlohmann::json& material_json) {
        ValidationReport report;

        const rx::string name = get_json_value<std::string>(material_json, "name").value_or("<NAME_MISSING>").c_str();
        if(name == "<NAME_MISSING>") {
            report.errors.emplace_back(material_msg(name, "Missing material name"));
        }

        const bool missing_geometry_filter = material_json.find("filter") == material_json.end();
        if(missing_geometry_filter) {
            report.errors.emplace_back(material_msg(name, "Missing geometry filter"));
        }

        const bool missing_passes = material_json.find("passes") == material_json.end();
        if(missing_passes) {
            report.errors.emplace_back(material_msg(name, "Missing material passes"));
        } else {
            const nlohmann::json& passes_json = material_json.at("passes");
            if(!passes_json.is_array()) {
                report.errors.emplace_back(material_msg(name, "Passes field must be an array"));
                return report;
            }
            if(passes_json.empty()) {
                report.errors.emplace_back(material_msg(name, "Passes field must have at least one item"));
                return report;
            }

            for(const auto& pass_json : passes_json) {
                const rx::string pass_name = get_json_value<std::string>(pass_json, "name").value_or("<NAME_MISSING>").c_str();
                if(pass_name == "<NAME_MISSING>") {
                    report.errors.emplace_back(material_pass_msg(name, pass_name, "Missing field name"));
                }

                const auto pipeline_maybe = get_json_value<std::string>(pass_json, "pipeline");
                if(!pipeline_maybe) {
                    report.errors.emplace_back(material_pass_msg(name, pass_name, "Missing field pipeline"));
                }

                const auto bindings_itr = pass_json.find("bindings");
                if(bindings_itr == pass_json.end()) {
                    report.warnings.emplace_back(material_pass_msg(name, pass_name, "Missing field bindings"));
                } else {
                    const nlohmann::json& bindings = *bindings_itr;
                    if(bindings.empty()) {
                        report.warnings.emplace_back(material_pass_msg(name, pass_name, "Field bindings exists but it's empty"));
                    }
                }
            }
        }

        return report;
    }

    void ensure_field_exists(nlohmann::json& j,
                             const rx::string& field_name,
                             const rx::string& context,
                             const nlohmann::json& default_value,
                             ValidationReport& report) {
        std::string field_name_std = field_name.data(); // STD string to easily interface with nlohmann::json

        if(j.find(field_name_std) == j.end()) {
            j[field_name_std] = default_value[field_name_std];
            report.warnings.emplace_back(context + ": Missing field " + field_name + ". A default value of '" +
                                         j[field_name_std].dump().c_str() + "' will be used");
        }
    }

    void print(const ValidationReport& report) {
        report.errors.each_fwd([&](const rx::string& error) { NOVA_LOG(ERROR) << error.data(); });

        report.warnings.each_fwd([&](const rx::string& warning) { NOVA_LOG(DEBUG) << warning.data(); });
    }

    void ValidationReport::merge_in(const ValidationReport& other) {
        errors += other.errors;
        warnings += other.warnings;
    }
} // namespace nova::renderer::shaderpack
