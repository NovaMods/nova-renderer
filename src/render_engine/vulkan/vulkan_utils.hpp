//
// Created by jannis on 30.08.18.
//

#ifndef NOVA_RENDERER_VULKAN_UTILS_HPP
#define NOVA_RENDERER_VULKAN_UTILS_HPP

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include <nova_renderer/command_list.hpp>
#include <nova_renderer/shaderpack_data.hpp>

#ifdef ENABLE_VULKAN
namespace nova::renderer::rhi {
    VkImageLayout to_vk_layout(ResourceState state);

	VkPrimitiveTopology to_primitive_topology(shaderpack::PrimitiveTopologyEnum topology);

	VkBlendFactor to_blend_factor(shaderpack::BlendFactorEnum factor);

	VkCompareOp to_compare_op(const shaderpack::CompareOpEnum compare_op);

	VkStencilOp to_stencil_op(shaderpack::StencilOpEnum stencil_op);

	VkFormat to_vk_format(const shaderpack::PixelFormatEnum format);

	VkDescriptorType to_vk_descriptor_type(const DescriptorType type);

	VkShaderStageFlags to_vk_shader_stage_flags(const ShaderStageFlags flags);

    std::string vk_result_to_string(VkResult result);

    std::string to_string(VkObjectType obj_type);

    std::vector<VkVertexInputBindingDescription>& get_vertex_input_binding_descriptions();

    std::vector<VkVertexInputAttributeDescription>& get_vertex_input_attribute_descriptions();
} // namespace nova::renderer

// Only validate errors in debug mode
// Release mode needs to be fast A F
#ifndef NDEBUG
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NOVA_CHECK_RESULT(expr)                                                                                                            \
    {                                                                                                                                      \
        const VkResult result = (expr);                                                                                                    \
        if(result != VK_SUCCESS) {                                                                                                         \
            NOVA_LOG(ERROR) << __FILE__ << ":" << __LINE__ << "=> " << #expr << "=" << vk_result_to_string(result);                        \
        }                                                                                                                                  \
    }
#else
#define NOVA_CHECK_RESULT(expr) expr
#endif
#endif // NOVA_RENDERER_VULKAN_UTILS_HPP
#endif
