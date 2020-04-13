#include "nova_renderer/rhi/rhi_types.hpp"

namespace nova::renderer::rhi {
    bool RhiResourceBindingDescription::operator==(const RhiResourceBindingDescription& other) {
        return set == other.set && binding == other.binding && count == other.count && type == other.type;
    }

    bool RhiResourceBindingDescription::operator!=(const RhiResourceBindingDescription& other) { return !(*this == other); }

    RhiResourceBarrier::RhiResourceBarrier() : buffer_memory_barrier{0, 0} {};

    ShaderStage operator|=(const ShaderStage lhs, const ShaderStage rhs) {
        return static_cast<ShaderStage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }

    bool is_depth_format(const PixelFormat format) {
        switch(format) {
            case PixelFormat::Rgba8:
                [[fallthrough]];
            case PixelFormat::Rgba16F:
                [[fallthrough]];
            case PixelFormat::Rgba32F:
                return false;

            case PixelFormat::Depth32:
                [[fallthrough]];
            case PixelFormat::Depth24Stencil8:
                return true;

            default:
                return false;
        }
    }

    uint32_t get_byte_size(const VertexFieldFormat format) {
        switch(format) {
            case VertexFieldFormat::Uint:
                return 4;

            case VertexFieldFormat::Float2:
                return 8;

            case VertexFieldFormat::Float3:
                return 12;

            case VertexFieldFormat::Float4:
                return 16;

            default:
                return 16;
        }
    }

    rx::string descriptor_type_to_string(DescriptorType type) {
        switch(type) {
            case DescriptorType::CombinedImageSampler:
                return "CombinedImageSampler";

            case DescriptorType::UniformBuffer:
                return "UniformBuffer";

            case DescriptorType::StorageBuffer:
                return "StorageBuffer";

            case DescriptorType::Texture:
                return "Texture";

            case DescriptorType::Sampler:
                return "Sampler";

            default:
                return "Unknown";
        }
    }
} // namespace nova::renderer::rhi
