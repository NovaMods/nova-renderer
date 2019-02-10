/*!
 * \author ddubois
 * \date 29-Nov-18.
 */

#include "vulkan_utils.hpp"
#include "../render_engine.hpp"

namespace nova::vulkan {
    std::string to_string(VkObjectType obj_type) {
        switch(obj_type) {
            case VK_OBJECT_TYPE_UNKNOWN:
                return "Unknown";
            case VK_OBJECT_TYPE_INSTANCE:
                return "Instance";
            case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
                return "Physical Device";
            case VK_OBJECT_TYPE_DEVICE:
                return "Device";
            case VK_OBJECT_TYPE_QUEUE:
                return "Queue";
            case VK_OBJECT_TYPE_SEMAPHORE:
                return "Semaphore";
            case VK_OBJECT_TYPE_COMMAND_BUFFER:
                return "Command Buffer";
            case VK_OBJECT_TYPE_FENCE:
                return "Fence";
            case VK_OBJECT_TYPE_DEVICE_MEMORY:
                return "Device Memory";
            case VK_OBJECT_TYPE_BUFFER:
                return "Buffer";
            case VK_OBJECT_TYPE_IMAGE:
                return "Image ";
            case VK_OBJECT_TYPE_EVENT:
                return "Event";
            case VK_OBJECT_TYPE_QUERY_POOL:
                return "Query Pool";
            case VK_OBJECT_TYPE_BUFFER_VIEW:
                return "Buffer View";
            case VK_OBJECT_TYPE_IMAGE_VIEW:
                return "Image View";
            case VK_OBJECT_TYPE_SHADER_MODULE:
                return "Shader Module";
            case VK_OBJECT_TYPE_PIPELINE_CACHE:
                return "Pipeline Cache";
            case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
                return "Pipeline Layout";
            case VK_OBJECT_TYPE_RENDER_PASS:
                return "Render Pass";
            case VK_OBJECT_TYPE_PIPELINE:
                return "Pipeline";
            case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
                return "Descriptor Set Layout";
            case VK_OBJECT_TYPE_SAMPLER:
                return "Sampler";
            case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
                return "Descriptor Pool";
            case VK_OBJECT_TYPE_DESCRIPTOR_SET:
                return "Descriptor Set";
            case VK_OBJECT_TYPE_FRAMEBUFFER:
                return "Framebuffer";
            case VK_OBJECT_TYPE_COMMAND_POOL:
                return "Command Pool";
            case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:
                return "YCBCR Conversion";
            case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:
                return "Descriptor Update Template";
            case VK_OBJECT_TYPE_SURFACE_KHR:
                return "Surface";
            case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
                return "Swapchain";
            case VK_OBJECT_TYPE_DISPLAY_KHR:
                return "Display KHR";
            case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:
                return "Display Mode KHR";
            case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:
                return "Debug Report Callback EXT";
            case VK_OBJECT_TYPE_OBJECT_TABLE_NVX:
                return "Object Table NVX";
            case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX:
                return "Indirect Commands Layout NVX";
            case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
                return "Debug Utils Messenger EXT";
            case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:
                return "Validation Cache EXT";
            case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV:
                return "Acceleration Structure NV";
            default:
                return "Unknown";
        }
    }

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
} // namespace nova::vulkan
