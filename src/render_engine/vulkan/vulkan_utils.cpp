/*!
 * \author ddubois
 * \date 29-Nov-18.
 */

#include "vulkan_utils.hpp"
#include "../render_engine.hpp"

namespace nova::vulkan {
    std::vector<VkVertexInputBindingDescription>& get_vertex_input_binding_descriptions() { 
        static std::vector<VkVertexInputBindingDescription> input_descriptions = {
            VkVertexInputBindingDescription{
                0,                          // binding
                sizeof(full_vertex),        // stride
                VK_VERTEX_INPUT_RATE_VERTEX // input rate
            },
            VkVertexInputBindingDescription{
                1,                           // binding
                sizeof(full_vertex),         // stride
                VK_VERTEX_INPUT_RATE_VERTEX  // input rate
            },
            VkVertexInputBindingDescription{
                2,                           // binding
                sizeof(full_vertex),         // stride
                VK_VERTEX_INPUT_RATE_VERTEX  // input rate
            },
            VkVertexInputBindingDescription{
                3,                           // binding
                sizeof(full_vertex),         // stride
                VK_VERTEX_INPUT_RATE_VERTEX  // input rate
            },
            VkVertexInputBindingDescription{
                4,                           // binding
                sizeof(full_vertex),         // stride
                VK_VERTEX_INPUT_RATE_VERTEX  // input rate
            },
            VkVertexInputBindingDescription{
                5,                           // binding
                sizeof(full_vertex),         // stride
                VK_VERTEX_INPUT_RATE_VERTEX  // input rate
            },
            VkVertexInputBindingDescription{
                6,                           // binding
                sizeof(full_vertex),         // stride
                VK_VERTEX_INPUT_RATE_VERTEX  // input rate
            },
        };

        return input_descriptions;
    }

    std::vector<VkVertexInputAttributeDescription>& get_vertex_input_attribute_descriptions() { 
        static std::vector<VkVertexInputAttributeDescription> attribute_descriptions = {
            // Position
            VkVertexInputAttributeDescription{
                0,                              // location
                0,                              // binding
                VK_FORMAT_R32G32B32_SFLOAT,     // format
                0,                              // offset
            },

            // Normal
            VkVertexInputAttributeDescription{
                1,                              // location
                0,                              // binding
                VK_FORMAT_R32G32B32_SFLOAT,     // format
                0,                              // offset
            },

            // Tangent
            VkVertexInputAttributeDescription{
                2,                              // location
                0,                              // binding
                VK_FORMAT_R32G32B32_SFLOAT,     // format
                0,                              // offset
            },

            // Main UV
            VkVertexInputAttributeDescription{
                3,                              // location
                0,                              // binding
                VK_FORMAT_R16G16_UNORM,         // format
                0,                              // offset
            },

            // Secondary UV
            VkVertexInputAttributeDescription{
                4,                              // location
                0,                              // binding
                VK_FORMAT_R8G8_UNORM,           // format
                0,                              // offset
            },

            // Virtual texture ID
            VkVertexInputAttributeDescription{
                5,                              // location
                0,                              // binding
                VK_FORMAT_R32_UINT,             // format
                0,                              // offset
            },

            // Other data
            VkVertexInputAttributeDescription{
                6,                              // location
                0,                              // binding
                VK_FORMAT_R32G32B32A32_SFLOAT,  // format
                0,                              // offset
            },
        };

        return attribute_descriptions;
    }
}
