/*!
 * \author ddubois 
 * \date 23-Aug-18.
 */

#include "shaderpack_data.hpp"
#include "utils.hpp"

namespace nova {
    /*!
    * \brief If a data member isn't in the JSON (which is fully supported and is 100% fine) then we use this to fill in
    * any missing values
    */
    pipeline_data default_pipeline;

    void from_json(const nlohmann::json& j, texture_format& format) {
        format.pixel_format = get_json_value<pixel_format_enum>(j, "format", pixel_format_enum::from_string).value();
        format.dimension_type = get_json_value<texture_dimension_type_enum>(j, "dimensionType", texture_dimension_type_enum::from_string).value();
        format.width = get_json_value<float>(j, "width").value_or(0);
        format.height = get_json_value<float>(j, "height").value_or(0);
    }

    void from_json(const nlohmann::json& j, texture_resource& tex) {
        tex.name = get_json_value<std::string>(j, "name").value_or("NAME_MISSING");
        tex.format = get_json_value<texture_format>(j, "format").value();
    }

    void from_json(const nlohmann::json& j, shaderpack_resources& res) {
        res.textures = get_json_array<texture_resource>(j, "textures");
    }
}
