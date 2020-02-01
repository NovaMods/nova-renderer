#pragma once

#include <string>

#include "nova_renderer/rhi/command_list.hpp"
#include "nova_renderer/shaderpack_data.hpp"

// idk maybe this header is included in places that already include Vulkan? Either way I want this include here and not anywhere else
// ReSharper disable once CppUnusedIncludeDirective
#include "vulkan.hpp"

namespace nova::renderer::rhi {
    VkImageLayout to_vk_image_layout(ResourceState layout);

    VkAccessFlags to_vk_access_flags(ResourceAccess access);

    VkPrimitiveTopology to_primitive_topology(shaderpack::PrimitiveTopologyEnum topology);

    VkBlendFactor to_blend_factor(shaderpack::BlendFactorEnum factor);

    VkCompareOp to_compare_op(shaderpack::CompareOpEnum compare_op);

    VkStencilOp to_stencil_op(shaderpack::StencilOpEnum stencil_op);

    VkFormat to_vk_format(shaderpack::PixelFormatEnum format);

    VkDescriptorType to_vk_descriptor_type(DescriptorType type);

    VkShaderStageFlags to_vk_shader_stage_flags(ShaderStage flags);

    std::string to_string(VkResult result);

    std::string to_string(VkObjectType obj_type);

    VkFormat to_vk_vertex_format(VertexFieldFormat field);

    bool operator&(const ShaderStage& lhs, const ShaderStage& rhs);
} // namespace nova::renderer::rhi

// Only validate errors in debug mode
// Release mode needs to be fast A F
#ifdef NOVA_DEBUG
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NOVA_CHECK_RESULT(expr)                                                                                                            \
    {                                                                                                                                      \
        const VkResult result = (expr);                                                                                                    \
        if(result != VK_SUCCESS) {                                                                                                         \
            logger(rx::log::level::k_error, "%s:%u=>%s=%s", __FILE__, __LINE__, #expr, to_string(result));                                 \
        }                                                                                                                                  \
    }
#else
#define NOVA_CHECK_RESULT(expr) expr
#endif
