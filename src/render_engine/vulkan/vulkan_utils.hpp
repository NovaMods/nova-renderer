//
// Created by jannis on 30.08.18.
//

#ifndef NOVA_RENDERER_VULKAN_UTILS_HPP
#define NOVA_RENDERER_VULKAN_UTILS_HPP

#include <array>
#include <string>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace nova::vulkan {
    class vulkan_utils {
    public:
        static std::string vk_result_to_string(VkResult result) {
#define ERROR_CASE(r)                                                                                                                                                                                  \
    case VK_##r: return #r
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

                default: return std::string("UNKNOWN_ERROR");
            }
#undef ERROR_CASE
        }
    };

    struct vulkan_vertex {
        glm::vec2 pos;
        glm::vec3 color;

        static VkVertexInputBindingDescription get_binding_description() {
            VkVertexInputBindingDescription description;
            description.binding = 0;
            description.stride = sizeof(vulkan_vertex);
            description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return description;
        }

        static std::array<VkVertexInputAttributeDescription, 2> get_attribute_description() {
            std::array<VkVertexInputAttributeDescription, 2> descriptions = {};
            descriptions[0].binding = 0;
            descriptions[0].location = 0;
            descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            descriptions[0].offset = static_cast<uint32_t>(offsetof(vulkan_vertex, pos));

            descriptions[1].binding = 0;
            descriptions[1].location = 1;
            descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            descriptions[1].offset = static_cast<uint32_t>(offsetof(vulkan_vertex, color));
            return descriptions;
        }
    };
}  // namespace nova::vulkan

#define NOVA_THROW_IF_VK_ERROR(expr, exception)                                                                                                                                                        \
    do {                                                                                                                                                                                               \
        VkResult val = (expr);                                                                                                                                                                         \
        if(val != VK_SUCCESS) {                                                                                                                                                                        \
            throw exception(std::string(__FILE__) + ":" + std::to_string(__LINE__) + "=> " + ::nova::vulkan::vulkan_utils::vk_result_to_string(val));                                                  \
        }                                                                                                                                                                                              \
    } while(false);

#endif  // NOVA_RENDERER_VULKAN_UTILS_HPP
