//
// Created by jannis on 30.08.18.
//

#ifndef NOVA_RENDERER_VULKAN_UTILS_HPP
#define NOVA_RENDERER_VULKAN_UTILS_HPP

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace nova::renderer {
    std::string vk_result_to_string(VkResult result);

    std::string to_string(VkObjectType obj_type);

    std::vector<VkVertexInputBindingDescription>& get_vertex_input_binding_descriptions();

    std::vector<VkVertexInputAttributeDescription>& get_vertex_input_attribute_descriptions();
} // namespace nova::renderer::vulkan

// Only validate errors in debug mode
// Release mode needs to be fast A F
#ifndef NDEBUG
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NOVA_CHECK_ERROR(expr)                                                                                                             \
    {                                                                                                                                      \
        const VkResult result = (expr);                                                                                                    \
        if(result != VK_SUCCESS) {                                                                                                         \
            throw NOVA_LOG(ERROR) << __FILE__ << ":" << __LINE__ + "=> " << vk_result_to_string(result);                                   \
        }                                                                                                                                  \
    }
#else
#define NOVA_CHECK_ERROR(expr) expr
#endif
#endif // NOVA_RENDERER_VULKAN_UTILS_HPP
