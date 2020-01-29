/*!
 * \author ddubois
 * \date 07-Sep-18.
 */

#ifndef NOVA_RENDERER_JSON_INTEROP_HPP
#define NOVA_RENDERER_JSON_INTEROP_HPP

#include <rx/core/json.h>

#include "nova_renderer/shaderpack_data.hpp"

namespace nova::renderer::shaderpack {
    template<typename ValType>
    ValType from_json(const rx::json& json);



    void from_json(const nlohmann::json& j, TextureFormat& format);

    void from_json(const nlohmann::json& j, TextureCreateInfo& tex);

    void from_json(const nlohmann::json& j, SamplerCreateInfo& sampler);

    void from_json(const nlohmann::json& j, ShaderpackResourcesData& res);

    void from_json(const nlohmann::json& j, TextureAttachmentInfo& tex);

    void from_json(const nlohmann::json& j, PipelineCreateInfo& pipeline);

    void from_json(const nlohmann::json& j, StencilOpState& stencil_op);

    void from_json(const nlohmann::json& j, RenderPassCreateInfo& pass);

    void from_json(const nlohmann::json& j, rx::vector<RenderPassCreateInfo>& passes);

    void from_json(const nlohmann::json& j, RendergraphData& data);

    void from_json(const nlohmann::json& j, MaterialPass& pass);

    void from_json(const nlohmann::json& j, MaterialData& mat);
} // namespace nova::renderer::shaderpack

#endif // NOVA_RENDERER_JSON_INTEROP_HPP
