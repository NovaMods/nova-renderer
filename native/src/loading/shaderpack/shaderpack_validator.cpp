/*!
 * \author ddubois 
 * \date 12-Sep-18.
 */

#include "shaderpack_validator.hpp"
#include "../utils.hpp"
#include "../../util/utils.hpp"

namespace nova {
    /*!
     * \brief All the default values for a JSON pipeline
     * 
     * If a field is in `pipeline_data` but not in this structure, it is a required field and cannot be given a 
     * default value. It will thus cause an exception
     */
    nlohmann::json default_pipeline = {
        { "parentName", "" },
        { "defines", {} },
        { "states", {} },
        { "frontFace", {} },
        { "backFace", {} },
        { "fallback", {} },
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

    std::vector<std::string> required_pipeline_fields = {
        "name",
        "pass",
        "vertexFields",
        "fragmentShader",
        "geometryShader"
    };

    json_field_missing::json_field_missing(const std::string& field_name) : std::runtime_error("Missing field " + field_name) {}

    void validate_pipeline(nlohmann::json& pipeline_json) {
        const std::string name = get_json_value<std::string>(pipeline_json, "name", "<NAME_MISSING>");

        // Check non-required fields first 
        for(const auto& str : default_pipeline.items()) {
            check_if_field_exists(pipeline_json, str.key(), name, default_pipeline);
        }

        // Check required items. Collect all the missing fields so we can tell them to the user at the same time
        std::vector<std::string> missing_required_fields;
        missing_required_fields.reserve(required_pipeline_fields.size());
        for(const std::string& field_name : required_pipeline_fields) {
            const auto& itr = pipeline_json.find(field_name);
            if(itr == pipeline_json.end()) {
                missing_required_fields.push_back(field_name);
            }
        }

        if(!missing_required_fields.empty()) {
            const std::string all_fields = join(missing_required_fields, ", ");
            throw json_field_missing(all_fields);
        }
    }

    void check_if_field_exists(nlohmann::json& j, const std::string& field_name, const std::string& context, const nlohmann::json& default_value) {
        const auto& itr = j.find(field_name);
        if(itr == j.end()) {
            NOVA_LOG(DEBUG) << context << ": Missing field " << field_name << ". Filling in default";
            j[field_name] = default_value;
        }
    }
}
