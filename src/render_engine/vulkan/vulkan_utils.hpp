#pragma once

#include <string>
#include <vector>

#include "vulkan.hpp"

#include <nova_renderer/command_list.hpp>

namespace nova::renderer {
    VkImageLayout to_vk_layout(resource_state state);

	VkPrimitiveTopology to_primitive_topology(primitive_topology_enum topology);

	VkBlendFactor to_blend_factor(blend_factor_enum factor);

	VkCompareOp to_compare_op(const compare_op_enum compare_op);

	VkStencilOp to_stencil_op(stencil_op_enum stencil_op);

	VkFormat to_vk_format(const pixel_format_enum format);

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