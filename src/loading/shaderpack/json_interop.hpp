/*!
 * \author ddubois
 * \date 07-Sep-18.
 */

#ifndef NOVA_RENDERER_JSON_INTEROP_HPP
#define NOVA_RENDERER_JSON_INTEROP_HPP

#include <nova_renderer/shaderpack_data.hpp>

namespace nova::renderer::shaderpack {
    void from_json(const nlohmann::json& j, texture_format& format);

    void from_json(const nlohmann::json& j, texture_create_into_t& tex);

    void from_json(const nlohmann::json& j, sampler_create_info_t& sampler);

    void from_json(const nlohmann::json& j, shaderpack_resources_data_t& res);

    void from_json(const nlohmann::json& j, texture_attachment_info_t& tex);

    void from_json(const nlohmann::json& j, pipeline_create_info_t& pipeline);

    void from_json(const nlohmann::json& j, stencil_op_state& stencil_op);

    void from_json(const nlohmann::json& j, vertex_field_data_t& vertex_data);

    void from_json(const nlohmann::json& j, render_pass_create_info_t& pass);

    void from_json(const nlohmann::json& j, std::vector<render_pass_create_info_t>& passes);

    void from_json(const nlohmann::json& j, material_pass_t& pass);

    void from_json(const nlohmann::json& j, material_data_t& mat);
} // namespace nova::renderer

#endif // NOVA_RENDERER_JSON_INTEROP_HPP
