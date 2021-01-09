#pragma once

#include "nova_renderer/renderpack_data.hpp"
#include "nova_renderer/rhi/command_list.hpp"

// idk maybe this header is included in places that already include Vulkan? Either way I want this include here and not anywhere else
// ReSharper disable once CppUnusedIncludeDirective
#include "vulkan.hpp"

namespace nova {
    namespace renderer {
        enum class BlendOp;
        enum class BlendFactor;
        enum class StencilOp;
        enum class CompareOp;
    } // namespace renderer
} // namespace nova

namespace nova::renderer::rhi {
    class VulkanRenderDevice;
    vk::ImageLayout to_vk_image_layout(ResourceState layout);

    vk::AccessFlags to_vk_access_flags(ResourceAccess access);

    vk::PrimitiveTopology to_primitive_topology(renderpack::RPPrimitiveTopology topology);

    vk::BlendFactor to_blend_factor(BlendFactor factor);

    vk::BlendOp to_blend_op(const BlendOp blend_op);

    vk::CompareOp to_compare_op(CompareOp compare_op);

    vk::StencilOp to_stencil_op(StencilOp stencil_op);

    vk::Format to_vk_format(PixelFormat format);

    vk::Filter to_vk_filter(TextureFilter filter);

    vk::SamplerAddressMode to_vk_address_mode(TextureCoordWrapMode wrap_mode);

    vk::DescriptorType to_vk_descriptor_type(DescriptorType type);

    vk::ShaderStageFlags to_vk_shader_stage_flags(ShaderStage flags);

    std::string to_string(vk::Result result);

    std::string to_string(vk::ObjectType obj_type);

    [[nodiscard]] vk::Format to_vk_vertex_format(VertexFieldFormat field);

    [[nodiscard]] std::vector<vk::DescriptorSetLayout> create_descriptor_set_layouts(
        const std::unordered_map<std::string, RhiResourceBindingDescription>& all_bindings, VulkanRenderDevice& render_device);

    bool operator&(const ShaderStage& lhs, const ShaderStage& rhs);
} // namespace nova::renderer::rhi

// Only validate errors in debug mode
// Release mode needs to be fast A F
#ifdef NOVA_DEBUG
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NOVA_CHECK_RESULT(expr)                                                                                                            \
    {                                                                                                                                      \
        const vk::Result result = (expr);                                                                                                    \
        if(result != VK_SUCCESS) {                                                                                                         \
            logger->error("{}:{}=>{}={}", __FILE__, __LINE__, #expr, to_string(result));                                                   \
        }                                                                                                                                  \
    }
#else
#define NOVA_CHECK_RESULT(expr) expr
#endif
