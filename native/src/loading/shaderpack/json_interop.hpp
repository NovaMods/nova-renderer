/*!
 * \author ddubois 
 * \date 07-Sep-18.
 */

#ifndef NOVA_RENDERER_JSON_INTEROP_HPP
#define NOVA_RENDERER_JSON_INTEROP_HPP

#include "shaderpack_data.hpp"

namespace nova {
    void from_json(const nlohmann::json& j, texture_format& format);

    void from_json(const nlohmann::json& j, texture_resource& tex);

    void from_json(const nlohmann::json& j, shaderpack_resources& res);

    void from_json(const nlohmann::json& j, texture_attachment& tex);

    void from_json(const nlohmann::json& j, input_textures& inputs);

    void from_json(const nlohmann::json& j, pipeline_data& pipeline);

    void from_json(const nlohmann::json& j, stencil_op_state& stencil_op);

    void from_json(const nlohmann::json& j, bound_resource& resource);

    void from_json(const nlohmann::json& j, render_pass& pass);

    void from_json(const nlohmann::json& j, std::vector<render_pass>& passes);
}

#endif //NOVA_RENDERER_JSON_INTEROP_HPP
