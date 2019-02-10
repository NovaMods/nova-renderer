//
// Created by jannis on 30.08.18.
//

#ifndef NOVA_RENDERER_VULKAN_UTILS_HPP
#define NOVA_RENDERER_VULKAN_UTILS_HPP

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace nova::vulkan {
    class vulkan_utils {
    public:
        static std::string vk_result_to_string(VkResult result) {
#define ERROR_CASE(r)                                                                                                                                                                                                                                    \
    case VK_##r:                                                                                                                                                                                                                                         \
        return #r
            switch(result) {
                ERROR_CASE(SUCCESS);
                ERROR_CASE(NOT_READY);
                ERROR_CASE(TIMEOUT);
                ERROR_CASE(EVENT_SET);
                ERROR_CASE(INCOMPLETE);
                ERROR_CASE(ERROR_OUT_OF_HOST_MEMORY);
                ERROR_CASE(ERROR_OUT_OF_DEVICE_MEMORY);
                ERROR_CASE(ERROR_INITIALIZATION_FAILED);
                ERROR_CASE(ERROR_DEVICE_LOST);

                default:
                    return std::string("UNKNOWN_ERROR");
            }
#undef ERROR_CASE
        }
    };

    std::string to_string(VkObjectType obj_type);

    std::vector<VkVertexInputBindingDescription> &get_vertex_input_binding_descriptions();

    std::vector<VkVertexInputAttributeDescription> &get_vertex_input_attribute_descriptions();
} // namespace nova::vulkan

// Only validate errors in debug mode
// Release mode needs to be fast A F
#ifndef NDEBUG
#define NOVA_THROW_IF_VK_ERROR(expr, exception)                                                                                                                                                                                                          \
    do {                                                                                                                                                                                                                                                 \
        const VkResult result = (expr);                                                                                                                                                                                                                  \
        if(result != VK_SUCCESS) {                                                                                                                                                                                                                       \
            throw exception(std::string(__FILE__) + ":" + std::to_string(__LINE__) + "=> " + ::nova::vulkan::vulkan_utils::vk_result_to_string(result));                                                                                                 \
        }                                                                                                                                                                                                                                                \
    } while(false);
#else
#define NOVA_THROW_IF_VK_ERROR(expr, exception) expr
#endif
#endif // NOVA_RENDERER_VULKAN_UTILS_HPP
