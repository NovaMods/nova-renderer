#include "vulkan_utils.hpp"

#include <rx/core/algorithm/max.h>
#include <rx/core/log.h>

#include "nova_renderer/renderables.hpp"
#include "nova_renderer/rhi/pipeline_create_info.hpp"
#include "nova_renderer/rhi/render_device.hpp"

#include "vulkan_render_device.hpp"

namespace nova::renderer::rhi {
    RX_LOG("VulkanUtil", logger);

    vk::ImageLayout to_vk_image_layout(const ResourceState layout) {
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

            case ResourceState::Undefined:
                return VK_IMAGE_LAYOUT_UNDEFINED;

            default:
                logger->error("%u is not a valid image state", static_cast<uint32_t>(layout));
                return VK_IMAGE_LAYOUT_GENERAL;
        }
    }

    vk::AccessFlags to_vk_access_flags(const ResourceAccess access) {
        switch(access) {
            case ResourceAccess::IndirectCommandRead:
                return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;

            case ResourceAccess::IndexRead:
                return VK_ACCESS_INDEX_READ_BIT;

            case ResourceAccess::VertexAttributeRead:
                return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

            case ResourceAccess::UniformRead:
                return VK_ACCESS_UNIFORM_READ_BIT;

            case ResourceAccess::InputAttachmentRead:
                return VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

            case ResourceAccess::ShaderRead:
                return VK_ACCESS_SHADER_READ_BIT;

            case ResourceAccess::ShaderWrite:
                return VK_ACCESS_SHADER_WRITE_BIT;

            case ResourceAccess::ColorAttachmentRead:
                return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

            case ResourceAccess::ColorAttachmentWrite:
                return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            case ResourceAccess::DepthStencilAttachmentRead:
                return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

            case ResourceAccess::DepthStencilAttachmentWrite:
                return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            case ResourceAccess::CopyRead:
                return VK_ACCESS_TRANSFER_READ_BIT;

            case ResourceAccess::CopyWrite:
                return VK_ACCESS_TRANSFER_WRITE_BIT;

            case ResourceAccess::HostRead:
                return VK_ACCESS_HOST_READ_BIT;

            case ResourceAccess::HostWrite:
                return VK_ACCESS_HOST_WRITE_BIT;

            case ResourceAccess::MemoryRead:
                return VK_ACCESS_MEMORY_READ_BIT;

            case ResourceAccess::MemoryWrite:
                return VK_ACCESS_MEMORY_WRITE_BIT;

            case ResourceAccess::ShadingRateImageRead:
                return VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV;

            case ResourceAccess::AccelerationStructureRead:
                return VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;

            case ResourceAccess::AccelerationStructureWrite:
                return VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;

            case ResourceAccess::FragmentDensityMapRead:
                return VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
        }

        return {};
    }

    vk::PrimitiveTopology to_primitive_topology(const renderpack::RPPrimitiveTopology topology) {
        switch(topology) {
            case renderpack::RPPrimitiveTopology::Lines:
                return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case renderpack::RPPrimitiveTopology::Triangles:
            default: // else the compiler complains
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
    }

    vk::BlendFactor to_blend_factor(const BlendFactor factor) {
        switch(factor) {
            case BlendFactor::DstAlpha:
                return VK_BLEND_FACTOR_DST_ALPHA;
            case BlendFactor::DstColor:
                return VK_BLEND_FACTOR_DST_COLOR;
            case BlendFactor::One:
                return VK_BLEND_FACTOR_ONE;
            case BlendFactor::OneMinusDstAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            case BlendFactor::OneMinusDstColor:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            case BlendFactor::OneMinusSrcAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case BlendFactor::OneMinusSrcColor:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            case BlendFactor::SrcAlpha:
                return VK_BLEND_FACTOR_SRC_ALPHA;
            case BlendFactor::SrcColor:
                return VK_BLEND_FACTOR_SRC_COLOR;
            case BlendFactor::Zero:
                return VK_BLEND_FACTOR_ZERO;
            default:
                return VK_BLEND_FACTOR_ZERO;
        }
    }

    vk::BlendOp to_blend_op(const BlendOp blend_op) {
        switch(blend_op) {
            case BlendOp::Add:
                return VK_BLEND_OP_ADD;

            case BlendOp::Subtract:
                return VK_BLEND_OP_SUBTRACT;

            case BlendOp::ReverseSubtract:
                return VK_BLEND_OP_REVERSE_SUBTRACT;

            case BlendOp::Min:
                return VK_BLEND_OP_MIN;

            case BlendOp::Max:
                return VK_BLEND_OP_MAX;

            default:
                return VK_BLEND_OP_ADD;
        }
    }

    vk::CompareOp to_compare_op(const CompareOp compare_op) {
        switch(compare_op) {
            case CompareOp::Never:
                return VK_COMPARE_OP_NEVER;

            case CompareOp::Less:
                return VK_COMPARE_OP_LESS;

            case CompareOp::LessEqual:
                return VK_COMPARE_OP_LESS_OR_EQUAL;

            case CompareOp::Greater:
                return VK_COMPARE_OP_GREATER;

            case CompareOp::GreaterEqual:
                return VK_COMPARE_OP_GREATER_OR_EQUAL;

            case CompareOp::Equal:
                return VK_COMPARE_OP_EQUAL;

            case CompareOp::NotEqual:
                return VK_COMPARE_OP_NOT_EQUAL;

            case CompareOp::Always:
                return VK_COMPARE_OP_ALWAYS;

            default:
                return VK_COMPARE_OP_NEVER;
        }
    }

    vk::StencilOp to_stencil_op(const StencilOp stencil_op) {
        switch(stencil_op) {
            case StencilOp::Keep:
                return VK_STENCIL_OP_KEEP;

            case StencilOp::Zero:
                return VK_STENCIL_OP_ZERO;

            case StencilOp::Replace:
                return VK_STENCIL_OP_REPLACE;

            case StencilOp::Increment:
                return VK_STENCIL_OP_INCREMENT_AND_CLAMP;

            case StencilOp::IncrementAndWrap:
                return VK_STENCIL_OP_INCREMENT_AND_WRAP;

            case StencilOp::Decrement:
                return VK_STENCIL_OP_DECREMENT_AND_CLAMP;

            case StencilOp::DecrementAndWrap:
                return VK_STENCIL_OP_DECREMENT_AND_WRAP;

            case StencilOp::Invert:
                return VK_STENCIL_OP_INVERT;

            default:
                return VK_STENCIL_OP_KEEP;
        }
    }

    vk::Format to_vk_format(const PixelFormat format) {
        switch(format) {
            case PixelFormat::Rgba8:
                return VK_FORMAT_R8G8B8A8_UNORM;

            case PixelFormat::Rgba16F:
                return VK_FORMAT_R16G16B16A16_SFLOAT;

            case PixelFormat::Rgba32F:
                return VK_FORMAT_R32G32B32A32_SFLOAT;

            case PixelFormat::Depth32:
                return VK_FORMAT_D32_SFLOAT;

            case PixelFormat::Depth24Stencil8:
                return VK_FORMAT_D24_UNORM_S8_UINT;

            default:
                logger->error("Unknown pixel format, returning RGBA8");
                return VK_FORMAT_R8G8B8A8_UNORM;
        }
    }

    vk::Filter to_vk_filter(const TextureFilter filter) {
        switch(filter) {
            case TextureFilter::Point:
                return VK_FILTER_NEAREST;

            case TextureFilter::Bilinear:
                return VK_FILTER_LINEAR;

            case TextureFilter::Trilinear:
                return VK_FILTER_CUBIC_IMG;

            default:
                return VK_FILTER_NEAREST;
        }
    }

    vk::SamplerAddressMode to_vk_address_mode(const TextureCoordWrapMode wrap_mode) {
        switch(wrap_mode) {
            case TextureCoordWrapMode::Repeat:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;

            case TextureCoordWrapMode::MirroredRepeat:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

            case TextureCoordWrapMode::ClampToEdge:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

            case TextureCoordWrapMode::ClampToBorder:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

            case TextureCoordWrapMode::MirrorClampToEdge:
                return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;

            default:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }
    }

    vk::DescriptorType to_vk_descriptor_type(const DescriptorType type) {
        switch(type) {
            case DescriptorType::CombinedImageSampler:
                return vk::DescriptorType::eCombinedImageSampler;

            case DescriptorType::UniformBuffer:
                return vk::DescriptorType::eUniformBuffer;

            case DescriptorType::StorageBuffer:
                return vk::DescriptorType::eStorageBuffer;

            case DescriptorType::Texture:
                return vk::DescriptorType::eSampledImage;

            case DescriptorType::Sampler:
                return vk::DescriptorType::eSampler;

            default:
                return vk::DescriptorType::eUniformBuffer;
        }
    }

    vk::ShaderStageFlags to_vk_shader_stage_flags(const ShaderStage flags) {
        vk::ShaderStageFlags vk_flags{};

        if(flags & ShaderStage::Vertex) {
            vk_flags |= vk::ShaderStageFlagBits::eVertex;
        }
        if(flags & ShaderStage::TessellationControl) {
            vk_flags |= vk::ShaderStageFlagBits::eTessellationControl;
        }
        if(flags & ShaderStage::TessellationEvaluation) {
            vk_flags |= vk::ShaderStageFlagBits::eTessellationEvaluation;
        }
        if(flags & ShaderStage::Geometry) {
            vk_flags |= vk::ShaderStageFlagBits::eGeometry;
        }
        if(flags & ShaderStage::Pixel) {
            vk_flags |= vk::ShaderStageFlagBits::eFragment;
        }
        if(flags & ShaderStage::Compute) {
            vk_flags |= vk::ShaderStageFlagBits::eCompute;
        }
        if(flags & ShaderStage::Raygen) {
            vk_flags |= vk::ShaderStageFlagBits::eRaygenNV;
        }
        if(flags & ShaderStage::AnyHit) {
            vk_flags |= vk::ShaderStageFlagBits::eAnyHitNV;
        }
        if(flags & ShaderStage::ClosestHit) {
            vk_flags |= vk::ShaderStageFlagBits::eClosestHitNV;
        }
        if(flags & ShaderStage::Miss) {
            vk_flags |= vk::ShaderStageFlagBits::eMissNV;
        }
        if(flags & ShaderStage::Intersection) {
            vk_flags |= vk::ShaderStageFlagBits::eIntersectionNV;
        }
        if(flags & ShaderStage::Task) {
            vk_flags |= vk::ShaderStageFlagBits::eTaskNV;
        }
        if(flags & ShaderStage::Mesh) {
            vk_flags |= vk::ShaderStageFlagBits::eMeshNV;
        }

        return vk_flags;
    }

    std::string to_string(vk::Result result) {
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

    std::string to_string(vk::ObjectType obj_type) {
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

    vk::Format to_vk_vertex_format(const VertexFieldFormat field) {
        switch(field) {
            case VertexFieldFormat::Uint:
                return VK_FORMAT_R32_UINT;

            case VertexFieldFormat::Float2:
                return VK_FORMAT_R32G32_SFLOAT;

            case VertexFieldFormat::Float3:
                return VK_FORMAT_R32G32B32_SFLOAT;

            case VertexFieldFormat::Float4:
                return VK_FORMAT_R32G32_SFLOAT;

            default:
                return VK_FORMAT_R32G32B32_SFLOAT;
        }
    }

    std::vector<vk::DescriptorSetLayout> create_descriptor_set_layouts(
        const std::unordered_map<std::string, RhiResourceBindingDescription>& all_bindings,
        VulkanRenderDevice& render_device,
        rx::memory::allocator& allocator) {
        const auto max_sets = render_device.gpu.props.limits.maxBoundDescriptorSets;

        uint32_t num_sets = 0;
        all_bindings.each_value([&](const RhiResourceBindingDescription& desc) {
            if(desc.set >= max_sets) {
                logger->error("Descriptor set %u is out of range - your GPU only supports %u sets!", desc.set, max_sets);
            } else {
                num_sets = rx::algorithm::max(num_sets, desc.set + 1);
            }
        });

        std::vector<uint32_t> variable_descriptor_counts{&allocator};
        variable_descriptor_counts.resize(num_sets, 0);

        // Some precalculations so we know how much room we actually need
        std::vector<uint32_t> num_bindings_per_set{&allocator};
        num_bindings_per_set.resize(num_sets);

        all_bindings.each_value([&](const RhiResourceBindingDescription& desc) {
            num_bindings_per_set[desc.set] = rx::algorithm::max(num_bindings_per_set[desc.set], desc.binding + 1);
        });

        std::vector<std::vector<vk::DescriptorSetLayoutBinding>> bindings_by_set{&allocator, num_sets};
        std::vector<std::vector<vk::DescriptorBindingFlags>> binding_flags_by_set{&allocator, num_sets};

        all_bindings.each_value([&](const RhiResourceBindingDescription& binding) {
            if(binding.set >= bindings_by_set.size()) {
                logger->error("You've skipped one or more descriptor sets! Don't do that, Nova can't handle it");
                return true;
            }

            const auto descriptor_binding = vk::DescriptorSetLayoutBinding()
                                                .setBinding(binding.binding)
                                                .setDescriptorType(to_vk_descriptor_type(binding.type))
                                                .setDescriptorCount(binding.count)
                                                .setStageFlags(to_vk_shader_stage_flags(binding.stages));

            logger->debug("Descriptor %u.%u is type %s", binding.set, binding.binding, descriptor_type_to_string(binding.type));

            if(binding.is_unbounded) {
                binding_flags_by_set[binding.set].push_back(vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
                                                                     vk::DescriptorBindingFlagBits::ePartiallyBound);

                // Record the maximum number of descriptors in the variable size array in this set
                variable_descriptor_counts[binding.set] = binding.count;

                logger->debug("Descriptor %u.%u is unbounded", binding.set, binding.binding);

            } else {
                binding_flags_by_set[binding.set].push_back({});
            }

            bindings_by_set[binding.set].push_back(descriptor_binding);

            return true;
        });

        std::vector<vk::DescriptorSetLayoutCreateInfo> dsl_create_infos{&allocator};
        dsl_create_infos.reserve(bindings_by_set.size());

        std::vector<vk::DescriptorSetLayoutBindingFlagsCreateInfo> flag_infos{&allocator};
        flag_infos.reserve(bindings_by_set.size());

        // We may make bindings_by_set much larger than it needs to be is there's multiple descriptor bindings per set. Thus, only iterate
        // through the sets we actually care about
        bindings_by_set.each_fwd([&](const std::vector<vk::DescriptorSetLayoutBinding>& bindings) {
            vk::DescriptorSetLayoutCreateInfo create_info = {};
            create_info.bindingCount = static_cast<uint32_t>(bindings.size());
            create_info.pBindings = bindings.data();

            const auto& flags = binding_flags_by_set[dsl_create_infos.size()];
            vk::DescriptorSetLayoutBindingFlagsCreateInfo binding_flags = {};
            binding_flags.bindingCount = static_cast<uint32_t>(flags.size());
            binding_flags.pBindingFlags = flags.data();
            flag_infos.emplace_back(binding_flags);

            create_info.pNext = &flag_infos[flag_infos.size() - 1];

            dsl_create_infos.push_back(create_info);
        });

        std::vector<vk::DescriptorSetLayout> ds_layouts{&allocator};
        ds_layouts.resize(dsl_create_infos.size());
        auto vk_allocator = wrap_allocator(allocator);
        for(size_t i = 0; i < dsl_create_infos.size(); i++) {
            render_device.device.createDescriptorSetLayout(&dsl_create_infos[i], &vk_allocator, &ds_layouts[i]);
        }

        return ds_layouts;
    }

    bool operator&(const ShaderStage& lhs, const ShaderStage& rhs) { return static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs); }
} // namespace nova::renderer::rhi
