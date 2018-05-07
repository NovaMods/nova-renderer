/*!
 * \author ddubois 
 * \date 29-Mar-18.
 */

#include "render_passes.h"
#include "../../../utils/utils.h"

namespace nova {
    render_pass::render_pass(const nlohmann::json &pass_json) : name(pass_json["name"].get<std::string>()) {
        dependencies = get_json_value<std::vector<std::string>>(pass_json, "dependencies", [&](const nlohmann::json& dependency_json) {
            auto vec = std::vector<std::string>{};
            for(const auto& val : dependency_json) {
                vec.push_back(val.get<std::string>());
            }
            return vec;
        });
        texture_inputs = get_json_value<std::vector<std::string>>(pass_json, "textureInputs", [&](const nlohmann::json& texture_inputs_json) {
            auto vec = std::vector<std::string>{};
            for(const auto& val : texture_inputs_json) {
                vec.push_back(val.get<std::string>());
            }
            return vec;
        });
        texture_outputs = get_json_value<std::vector<std::string>>(pass_json, "textureOutputs", [&](const nlohmann::json& texture_outputs_json) {
            auto vec = std::vector<std::string>{};
            for(const auto& val : texture_outputs_json) {
                vec.push_back(val.get<std::string>());
            }
            return vec;
        });
        depth_texture = get_json_value<std::string>(pass_json, "depthTexture");
    }

    texture_resource::texture_resource(const nlohmann::json &json) {
        name = json["name"].get<std::string>();
        const auto& format_json = json["format"];
        format.pixel_format = pixel_format_enum::from_string(format_json["pixelFormat"]);
        format.dimension_type = texture_dimension_type_enum::from_string(format_json["dimensionType"]);
        format.width = format_json["width"];
        format.height = format_json["height"];
    }

    bool operator==(const texture_format& rhs, const texture_format& lhs) {
        return rhs.pixel_format == lhs.pixel_format && rhs.width == lhs.width && rhs.height == lhs.height;
    }

    bool operator!=(const texture_format& rhs, const texture_format& lhs) {
        return !(rhs == lhs);
    }
}