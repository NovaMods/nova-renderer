/*!
 * \author ddubois
 * \date 07-Sep-18.
 */

#ifndef NOVA_RENDERER_JSON_INTEROP_HPP
#define NOVA_RENDERER_JSON_INTEROP_HPP

#include "shaderpack_data.hpp"

namespace nova {
    void from_json(const nlohmann::json &j, texture_format &format);

    void from_json(const nlohmann::json &j, texture_resource_data &tex);

    void from_json(const nlohmann::json &j, sampler_state_data &sampler);

    void from_json(const nlohmann::json &j, shaderpack_resources_data &res);

    void from_json(const nlohmann::json &j, texture_attachment &tex);

    void from_json(const nlohmann::json &j, pipeline_data &pipeline);

    void from_json(const nlohmann::json &j, stencil_op_state &stencil_op);

    void from_json(const nlohmann::json &j, vertex_field_data &vertex_data);

    void from_json(const nlohmann::json &j, render_pass_data &pass);

    void from_json(const nlohmann::json &j, std::vector<render_pass_data> &passes);

    void from_json(const nlohmann::json &j, material_pass &pass);

    void from_json(const nlohmann::json &j, material_data &pass);
} // namespace nova

#endif // NOVA_RENDERER_JSON_INTEROP_HPP
