#include "vulkan_utils.hpp"

#include "nova_renderer/render_engine.hpp"
#include "nova_renderer/renderables.hpp"

#include "../../util/logger.hpp"

namespace nova::renderer::rhi {
    VkImageLayout to_vk_image_layout(const ResourceState layout) {
        switch(layout) {
            case ResourceState::Common:
                return VK_IMAGE_LAYOUT_GENERAL;

            case ResourceState::CopySource:
                return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

            case ResourceState::CopyDestination:
                return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            case ResourceState::ShaderRead:
                return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            case ResourceState::ShaderWrite:
                return VK_IMAGE_LAYOUT_GENERAL; // TODO: Reevaluate this because it can't be optimal

            case ResourceState::RenderTarget:
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            case ResourceState::DepthWrite:
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            case ResourceState::DepthRead:
                return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;

            case ResourceState::PresentSource:
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                               
            default:
                NOVA_LOG(ERROR) << static_cast<uint32_t>(layout) << " is not a valid image state";
                return VK_IMAGE_LAYOUT_GENERAL;
        }
    }

    VkAccessFlags to_vk_access_flags(const AccessFlags access) {
        switch(access) {
            case AccessFlags::IndirectCommandRead:
                return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;

            case AccessFlags::IndexRead:
                return VK_ACCESS_INDEX_READ_BIT;

            case AccessFlags::VertexAttributeRead:
                return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

            case AccessFlags::UniformRead:
                return VK_ACCESS_UNIFORM_READ_BIT;

            case AccessFlags::InputAttachmentRead:
                return VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

            case AccessFlags::ShaderRead:
                return VK_ACCESS_SHADER_READ_BIT;

            case AccessFlags::ShaderWrite:
                return VK_ACCESS_SHADER_WRITE_BIT;

            case AccessFlags::ColorAttachmentRead:
                return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

            case AccessFlags::ColorAttachmentWrite:
                return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            case AccessFlags::DepthStencilAttachmentRead:
                return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

            case AccessFlags::DepthStencilAttachmentWrite:
                return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            case AccessFlags::CopyRead:
                return VK_ACCESS_TRANSFER_READ_BIT;

            case AccessFlags::CopyWrite:
                return VK_ACCESS_TRANSFER_WRITE_BIT;

            case AccessFlags::HostRead:
                return VK_ACCESS_HOST_READ_BIT;

            case AccessFlags::HostWrite:
                return VK_ACCESS_HOST_WRITE_BIT;

            case AccessFlags::MemoryRead:
                return VK_ACCESS_MEMORY_READ_BIT;

            case AccessFlags::MemoryWrite:
                return VK_ACCESS_MEMORY_WRITE_BIT;

            case AccessFlags::ShadingRateImageRead:
                return VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV;

            case AccessFlags::AccelerationStructureRead:
                return VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;

            case AccessFlags::AccelerationStructureWrite:
                return VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;

            case AccessFlags::FragmentDensityMapRead:
                return VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
        }
    }

    VkPrimitiveTopology to_primitive_topology(const shaderpack::PrimitiveTopologyEnum topology) {
        switch(topology) {
            case shaderpack::PrimitiveTopologyEnum::Lines:
                return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case shaderpack::PrimitiveTopologyEnum::Triangles:
            default: // else the compiler complains
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
    }

    VkBlendFactor to_blend_factor(shaderpack::BlendFactorEnum factor) {
        switch(factor) {
            case shaderpack::BlendFactorEnum::DstAlpha:
                return VK_BLEND_FACTOR_DST_ALPHA;
            case shaderpack::BlendFactorEnum::DstColor:
                return VK_BLEND_FACTOR_DST_COLOR;
            case shaderpack::BlendFactorEnum::One:
                return VK_BLEND_FACTOR_ONE;
            case shaderpack::BlendFactorEnum::OneMinusDstAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            case shaderpack::BlendFactorEnum::OneMinusDstColor:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            case shaderpack::BlendFactorEnum::OneMinusSrcAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case shaderpack::BlendFactorEnum::OneMinusSrcColor:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            case shaderpack::BlendFactorEnum::SrcAlpha:
                return VK_BLEND_FACTOR_SRC_ALPHA;
            case shaderpack::BlendFactorEnum::SrcColor:
                return VK_BLEND_FACTOR_SRC_COLOR;
            case shaderpack::BlendFactorEnum::Zero:
                return VK_BLEND_FACTOR_ZERO;
            default:
                return VK_BLEND_FACTOR_ZERO;
        }
    }

    VkCompareOp to_compare_op(const shaderpack::CompareOpEnum compare_op) {
        switch(compare_op) {
            case shaderpack::CompareOpEnum::Never:
                return VK_COMPARE_OP_NEVER;

            case shaderpack::CompareOpEnum::Less:
                return VK_COMPARE_OP_LESS;

            case shaderpack::CompareOpEnum::LessEqual:
                return VK_COMPARE_OP_LESS_OR_EQUAL;

            case shaderpack::CompareOpEnum::Greater:
                return VK_COMPARE_OP_GREATER;

            case shaderpack::CompareOpEnum::GreaterEqual:
                return VK_COMPARE_OP_GREATER_OR_EQUAL;

            case shaderpack::CompareOpEnum::Equal:
                return VK_COMPARE_OP_EQUAL;

            case shaderpack::CompareOpEnum::NotEqual:
                return VK_COMPARE_OP_NOT_EQUAL;

            case shaderpack::CompareOpEnum::Always:
                return VK_COMPARE_OP_ALWAYS;

            default:
                return VK_COMPARE_OP_NEVER;
        }
    }

    VkStencilOp to_stencil_op(shaderpack::StencilOpEnum stencil_op) {
        switch(stencil_op) {
            case shaderpack::StencilOpEnum::Keep:
                return VK_STENCIL_OP_KEEP;

            case shaderpack::StencilOpEnum::Zero:
                return VK_STENCIL_OP_ZERO;

            case shaderpack::StencilOpEnum::Replace:
                return VK_STENCIL_OP_REPLACE;

            case shaderpack::StencilOpEnum::Incr:
                return VK_STENCIL_OP_INCREMENT_AND_CLAMP;

            case shaderpack::StencilOpEnum::IncrWrap:
                return VK_STENCIL_OP_INCREMENT_AND_WRAP;

            case shaderpack::StencilOpEnum::Decr:
                return VK_STENCIL_OP_DECREMENT_AND_CLAMP;

            case shaderpack::StencilOpEnum::DecrWrap:
                return VK_STENCIL_OP_DECREMENT_AND_WRAP;

            case shaderpack::StencilOpEnum::Invert:
                return VK_STENCIL_OP_INVERT;

            default:
                return VK_STENCIL_OP_KEEP;
        }
    }

    VkFormat to_vk_format(const shaderpack::PixelFormatEnum format) {
        switch(format) {
            case shaderpack::PixelFormatEnum::RGBA8:
                return VK_FORMAT_R8G8B8A8_UNORM;

            case shaderpack::PixelFormatEnum::RGBA16F:
                return VK_FORMAT_R16G16B16A16_SFLOAT;

            case shaderpack::PixelFormatEnum::RGBA32F:
                return VK_FORMAT_R32G32B32A32_SFLOAT;

            case shaderpack::PixelFormatEnum::Depth:
                return VK_FORMAT_D32_SFLOAT;

            case shaderpack::PixelFormatEnum::DepthStencil:
                return VK_FORMAT_D24_UNORM_S8_UINT;

            default:
                NOVA_LOG(ERROR) << "Unknown pixel format, returning RGBA8";
                return VK_FORMAT_R8G8B8A8_UNORM;
        }
    }

    VkDescriptorType to_vk_descriptor_type(const DescriptorType type) {
        switch(type) {
            case DescriptorType::CombinedImageSampler:
                return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

            case DescriptorType::UniformBuffer:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            case DescriptorType::StorageBuffer:
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

            default:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
    }

    VkShaderStageFlags to_vk_shader_stage_flags(const ShaderStageFlags flags) {
        VkShaderStageFlags vk_flags = 0;

        if(flags & ShaderStageFlags::Vertex) {
            vk_flags |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if(flags & ShaderStageFlags::TessellationControl) {
            vk_flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }
        if(flags & ShaderStageFlags::TessellationEvaluation) {
            vk_flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        }
        if(flags & ShaderStageFlags::Geometry) {
            vk_flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
        }
        if(flags & ShaderStageFlags::Fragment) {
            vk_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if(flags & ShaderStageFlags::Compute) {
            vk_flags |= VK_SHADER_STAGE_COMPUTE_BIT;
        }
        if(flags & ShaderStageFlags::Raygen) {
            vk_flags |= VK_SHADER_STAGE_RAYGEN_BIT_NV;
        }
        if(flags & ShaderStageFlags::AnyHit) {
            vk_flags |= VK_SHADER_STAGE_ANY_HIT_BIT_NV;
        }
        if(flags & ShaderStageFlags::ClosestHit) {
            vk_flags |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        }
        if(flags & ShaderStageFlags::Miss) {
            vk_flags |= VK_SHADER_STAGE_MISS_BIT_NV;
        }
        if(flags & ShaderStageFlags::Intersection) {
            vk_flags |= VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        }
        if(flags & ShaderStageFlags::Task) {
            vk_flags |= VK_SHADER_STAGE_TASK_BIT_NV;
        }
        if(flags & ShaderStageFlags::Mesh) {
            vk_flags |= VK_SHADER_STAGE_MESH_BIT_NV;
        }

        return vk_flags;
    }

    std::string to_string(VkResult result) {
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
                sizeof(FullVertex),         // stride
                VK_VERTEX_INPUT_RATE_VERTEX // input rate
            },
            VkVertexInputBindingDescription{
                1,                          // binding
                sizeof(FullVertex),         // stride
                VK_VERTEX_INPUT_RATE_VERTEX // input rate
            },
            VkVertexInputBindingDescription{
                2,                          // binding
                sizeof(FullVertex),         // stride
                VK_VERTEX_INPUT_RATE_VERTEX // input rate
            },
            VkVertexInputBindingDescription{
                3,                          // binding
                sizeof(FullVertex),         // stride
                VK_VERTEX_INPUT_RATE_VERTEX // input rate
            },
            VkVertexInputBindingDescription{
                4,                          // binding
                sizeof(FullVertex),         // stride
                VK_VERTEX_INPUT_RATE_VERTEX // input rate
            },
            VkVertexInputBindingDescription{
                5,                          // binding
                sizeof(FullVertex),         // stride
                VK_VERTEX_INPUT_RATE_VERTEX // input rate
            },
            VkVertexInputBindingDescription{
                6,                          // binding
                sizeof(FullVertex),         // stride
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

    bool operator&(const ShaderStageFlags& lhs, const ShaderStageFlags& rhs) {
        return static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs);
    }
} // namespace nova::renderer::rhi
