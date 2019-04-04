#include "vulkan_utils.hpp"

#include "nova_renderer/render_engine.hpp"

namespace nova::renderer {
    VkImageLayout to_vk_layout(const resource_state state) {
        switch (state) {
        case resource_state::UNDEFINED:
            return VK_IMAGE_LAYOUT_UNDEFINED;

        case resource_state::GENERAL:
            return VK_IMAGE_LAYOUT_GENERAL;

        case resource_state::COLOR_ATTACHMENT:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        case resource_state::DEPTH_STENCIL_ATTACHMENT:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        case resource_state::DEPTH_READ_ONLY_STENCIL_ATTACHMENT:
            return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;

        case resource_state::DEPTH_ATTACHMENT_STENCIL_READ_ONLY:
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;

        case resource_state::DEPTH_STENCIL_READ_ONLY_ATTACHMENT:
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;

        case resource_state::PRESENT_SOURCE:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        case resource_state::NON_FRAGMENT_SHADER_READ_ONLY:
        case resource_state::FRAGMENT_SHADER_READ_ONLY:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        case resource_state::TRANSFER_SOURCE:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        case resource_state::TRANSFER_DESTINATION:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        default:
            return VK_IMAGE_LAYOUT_GENERAL;
        }
    }

    VkPrimitiveTopology to_primitive_topology(primitive_topology_enum topology) {
        switch(topology) {
            case primitive_topology_enum::Lines:
                return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case primitive_topology_enum::Triangles:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
    }

    VkBlendFactor to_blend_factor(blend_factor_enum factor) {
        switch(factor) {
            case blend_factor_enum::DstAlpha:
                return VK_BLEND_FACTOR_DST_ALPHA;
            case blend_factor_enum::DstColor:
                return VK_BLEND_FACTOR_DST_COLOR;
            case blend_factor_enum::One:
                return VK_BLEND_FACTOR_ONE;
            case blend_factor_enum::OneMinusDstAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            case blend_factor_enum::OneMinusDstColor:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            case blend_factor_enum::OneMinusSrcAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case blend_factor_enum::OneMinusSrcColor:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            case blend_factor_enum::SrcAlpha:
                return VK_BLEND_FACTOR_SRC_ALPHA;
            case blend_factor_enum::SrcColor:
                return VK_BLEND_FACTOR_SRC_COLOR;
            case blend_factor_enum::Zero:
                return VK_BLEND_FACTOR_ZERO;
            default:
                return VK_BLEND_FACTOR_ZERO;
        }
    }

    VkCompareOp to_compare_op(const compare_op_enum compare_op) {
        switch(compare_op) {
            case compare_op_enum::Never:
                return VK_COMPARE_OP_NEVER;

            case compare_op_enum::Less:
                return VK_COMPARE_OP_LESS;

            case compare_op_enum::LessEqual:
                return VK_COMPARE_OP_LESS_OR_EQUAL;

            case compare_op_enum::Greater:
                return VK_COMPARE_OP_GREATER;

            case compare_op_enum::GreaterEqual:
                return VK_COMPARE_OP_GREATER_OR_EQUAL;

            case compare_op_enum::Equal:
                return VK_COMPARE_OP_EQUAL;

            case compare_op_enum::NotEqual:
                return VK_COMPARE_OP_NOT_EQUAL;

            case compare_op_enum::Always:
                return VK_COMPARE_OP_ALWAYS;

            default:
                return VK_COMPARE_OP_NEVER;
        }
    }

    VkStencilOp to_stencil_op(stencil_op_enum stencil_op) {
        switch(stencil_op) {
            case stencil_op_enum::Keep:
                return VK_STENCIL_OP_KEEP;

            case stencil_op_enum::Zero:
                return VK_STENCIL_OP_ZERO;

            case stencil_op_enum::Replace:
                return VK_STENCIL_OP_REPLACE;

            case stencil_op_enum::Incr:
                return VK_STENCIL_OP_INCREMENT_AND_CLAMP;

            case stencil_op_enum::IncrWrap:
                return VK_STENCIL_OP_INCREMENT_AND_WRAP;

            case stencil_op_enum::Decr:
                return VK_STENCIL_OP_DECREMENT_AND_CLAMP;

            case stencil_op_enum::DecrWrap:
                return VK_STENCIL_OP_DECREMENT_AND_WRAP;

            case stencil_op_enum::Invert:
                return VK_STENCIL_OP_INVERT;

            default:
                return VK_STENCIL_OP_KEEP;
        }
    }

    VkFormat to_vk_format(const pixel_format_enum format) {
        switch(format) {
            case pixel_format_enum::RGBA8:
                return VK_FORMAT_R8G8B8A8_UNORM;

            case pixel_format_enum::RGBA16F:
                return VK_FORMAT_R16G16B16A16_SFLOAT;

            case pixel_format_enum::RGBA32F:
                return VK_FORMAT_R32G32B32A32_SFLOAT;

            case pixel_format_enum::Depth:
                return VK_FORMAT_D32_SFLOAT;

            case pixel_format_enum::DepthStencil:
                return VK_FORMAT_D24_UNORM_S8_UINT;
        }

        return VK_FORMAT_R10X6G10X6_UNORM_2PACK16;
    }

    std::string vk_result_to_string(VkResult result) {
        switch(result) {
            case VK_SUCCESS:
                return "VK_SUCCESS";
            case VK_NOT_READY:
                return "VK_NOT_READY";
            case VK_TIMEOUT:
                return "VK_TIMEOUT";
            case VK_EVENT_SET:
                return "VK_EVENT_SET";
            case VK_EVENT_RESET:
                return "VK_EVENT_RESET";
            case VK_INCOMPLETE:
                return "VK_INCOMPLETE";
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                return "VK_ERROR_OUT_OF_HOST_MEMORY";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            case VK_ERROR_INITIALIZATION_FAILED:
                return "VK_ERROR_INITIALIZATION_FAILED";
            case VK_ERROR_DEVICE_LOST:
                return "VK_ERROR_DEVICE_LOST";
            case VK_ERROR_MEMORY_MAP_FAILED:
                return "VK_ERROR_MEMORY_MAP_FAILED";
            case VK_ERROR_LAYER_NOT_PRESENT:
                return "VK_ERROR_LAYER_NOT_PRESENT";
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                return "VK_ERROR_EXTENSION_NOT_PRESENT";
            case VK_ERROR_FEATURE_NOT_PRESENT:
                return "VK_ERROR_FEATURE_NOT_PRESENT";
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                return "VK_ERROR_INCOMPATIBLE_DRIVER";
            case VK_ERROR_TOO_MANY_OBJECTS:
                return "VK_ERROR_TOO_MANY_OBJECTS";
            case VK_ERROR_FORMAT_NOT_SUPPORTED:
                return "VK_ERROR_FORMAT_NOT_SUPPORTED";
            case VK_ERROR_FRAGMENTED_POOL:
                return "VK_ERROR_FRAGMENTED_POOL";
            case VK_ERROR_OUT_OF_POOL_MEMORY:
                return "VK_ERROR_OUT_OF_POOL_MEMORY";
            case VK_ERROR_INVALID_EXTERNAL_HANDLE:
                return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
            case VK_ERROR_SURFACE_LOST_KHR:
                return "VK_ERROR_SURFACE_LOST_KHR";
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
                return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
            case VK_SUBOPTIMAL_KHR:
                return "VK_SUBOPTIMAL_KHR";
            case VK_ERROR_OUT_OF_DATE_KHR:
                return "VK_ERROR_OUT_OF_DATE_KHR";
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
                return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
            case VK_ERROR_VALIDATION_FAILED_EXT:
                return "VK_ERROR_VALIDATION_FAILED_EXT";
            case VK_ERROR_INVALID_SHADER_NV:
                return "VK_ERROR_INVALID_SHADER_NV";
            case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
                return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
            case VK_ERROR_FRAGMENTATION_EXT:
                return "VK_ERROR_FRAGMENTATION_EXT";
            case VK_ERROR_NOT_PERMITTED_EXT:
                return "VK_ERROR_NOT_PERMITTED_EXT";
            case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
                return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
            case VK_RESULT_RANGE_SIZE:
                return "VK_RESULT_RANGE_SIZE";
            default:
                return "Unknown result";
        }
    }

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
                1,                          // binding
                sizeof(full_vertex),        // stride
                VK_VERTEX_INPUT_RATE_VERTEX // input rate
            },
            VkVertexInputBindingDescription{
                2,                          // binding
                sizeof(full_vertex),        // stride
                VK_VERTEX_INPUT_RATE_VERTEX // input rate
            },
            VkVertexInputBindingDescription{
                3,                          // binding
                sizeof(full_vertex),        // stride
                VK_VERTEX_INPUT_RATE_VERTEX // input rate
            },
            VkVertexInputBindingDescription{
                4,                          // binding
                sizeof(full_vertex),        // stride
                VK_VERTEX_INPUT_RATE_VERTEX // input rate
            },
            VkVertexInputBindingDescription{
                5,                          // binding
                sizeof(full_vertex),        // stride
                VK_VERTEX_INPUT_RATE_VERTEX // input rate
            },
            VkVertexInputBindingDescription{
                6,                          // binding
                sizeof(full_vertex),        // stride
                VK_VERTEX_INPUT_RATE_VERTEX // input rate
            },
        };

        return input_descriptions;
    }

    std::vector<VkVertexInputAttributeDescription>& get_vertex_input_attribute_descriptions() {
        static std::vector<VkVertexInputAttributeDescription> attribute_descriptions = {
            // Position
            VkVertexInputAttributeDescription{
                0,                          // location
                0,                          // binding
                VK_FORMAT_R32G32B32_SFLOAT, // format
                0,                          // offset
            },

            // Normal
            VkVertexInputAttributeDescription{
                1,                          // location
                0,                          // binding
                VK_FORMAT_R32G32B32_SFLOAT, // format
                0,                          // offset
            },

            // Tangent
            VkVertexInputAttributeDescription{
                2,                          // location
                0,                          // binding
                VK_FORMAT_R32G32B32_SFLOAT, // format
                0,                          // offset
            },

            // Main UV
            VkVertexInputAttributeDescription{
                3,                      // location
                0,                      // binding
                VK_FORMAT_R16G16_UNORM, // format
                0,                      // offset
            },

            // Secondary UV
            VkVertexInputAttributeDescription{
                4,                    // location
                0,                    // binding
                VK_FORMAT_R8G8_UNORM, // format
                0,                    // offset
            },

            // Virtual texture ID
            VkVertexInputAttributeDescription{
                5,                  // location
                0,                  // binding
                VK_FORMAT_R32_UINT, // format
                0,                  // offset
            },

            // Other data
            VkVertexInputAttributeDescription{
                6,                             // location
                0,                             // binding
                VK_FORMAT_R32G32B32A32_SFLOAT, // format
                0,                             // offset
            },
        };

        return attribute_descriptions;
    }
} // namespace nova::renderer
