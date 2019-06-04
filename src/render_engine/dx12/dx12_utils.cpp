/*!
 * \author ddubois
 * \date 20-Oct-18.
 */

#include "dx12_utils.hpp"

#ifdef ENABLE_D3D12
namespace nova::renderer::rhi {
    D3D12_RESOURCE_STATES to_dx12_state(rhi::ResourceState state) {
        switch(state) {
            case rhi::ResourceState::Undefined:
            case rhi::ResourceState::General:
                return D3D12_RESOURCE_STATE_COMMON;

            case rhi::ResourceState::ColorAttachment:
                return D3D12_RESOURCE_STATE_RENDER_TARGET;

            case rhi::ResourceState::DepthStencilAttachment:
            case rhi::ResourceState::DepthAttachmentStencilReadOnly:
                return D3D12_RESOURCE_STATE_DEPTH_WRITE;

            case rhi::ResourceState::DepthReadOnlyStencilAttachment:
            case rhi::ResourceState::DepthStencilReadOnlyAttachment:
                return D3D12_RESOURCE_STATE_DEPTH_READ;

            case rhi::ResourceState::PresentSource:
                return D3D12_RESOURCE_STATE_PRESENT;

            case rhi::ResourceState::NonFragmentShaderReadOnly:
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

            case rhi::ResourceState::FragmentShaderReadOnly:
                return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

            case rhi::ResourceState::TransferSource:
                return D3D12_RESOURCE_STATE_COPY_SOURCE;

            case rhi::ResourceState::TransferDestination:
                return D3D12_RESOURCE_STATE_COPY_DEST;

            default:
                return D3D12_RESOURCE_STATE_COMMON;
        }
    }

    D3D12_BLEND to_dx12_blend(const shaderpack::BlendFactorEnum blend_factor) {
        switch(blend_factor) {
            case shaderpack::BlendFactorEnum::One:
                return D3D12_BLEND_ONE;

            case shaderpack::BlendFactorEnum::Zero:
                return D3D12_BLEND_ZERO;

            case shaderpack::BlendFactorEnum::SrcColor:
                return D3D12_BLEND_SRC_COLOR;

            case shaderpack::BlendFactorEnum::DstColor:
                return D3D12_BLEND_DEST_COLOR;

            case shaderpack::BlendFactorEnum::OneMinusSrcColor:
                return D3D12_BLEND_INV_SRC_COLOR;

            case shaderpack::BlendFactorEnum::OneMinusDstColor:
                return D3D12_BLEND_INV_DEST_COLOR;

            case shaderpack::BlendFactorEnum::SrcAlpha:
                return D3D12_BLEND_SRC_ALPHA;

            case shaderpack::BlendFactorEnum::DstAlpha:
                return D3D12_BLEND_DEST_ALPHA;

            case shaderpack::BlendFactorEnum::OneMinusSrcAlpha:
                return D3D12_BLEND_INV_SRC_ALPHA;

            case shaderpack::BlendFactorEnum::OneMinusDstAlpha:
                return D3D12_BLEND_INV_DEST_ALPHA;

            default:
                return D3D12_BLEND_ONE;
        }
    }

    D3D12_COMPARISON_FUNC to_dx12_compare_func(const shaderpack::CompareOpEnum depth_func) {
        switch(depth_func) {
            case shaderpack::CompareOpEnum::Never:
                return D3D12_COMPARISON_FUNC_NEVER;

            case shaderpack::CompareOpEnum::Less:
                return D3D12_COMPARISON_FUNC_LESS;

            case shaderpack::CompareOpEnum::LessEqual:
                return D3D12_COMPARISON_FUNC_LESS_EQUAL;

            case shaderpack::CompareOpEnum::Greater:
                return D3D12_COMPARISON_FUNC_GREATER;

            case shaderpack::CompareOpEnum::GreaterEqual:
                return D3D12_COMPARISON_FUNC_GREATER_EQUAL;

            case shaderpack::CompareOpEnum::Equal:
                return D3D12_COMPARISON_FUNC_EQUAL;

            case shaderpack::CompareOpEnum::NotEqual:
                return D3D12_COMPARISON_FUNC_NOT_EQUAL;

            case shaderpack::CompareOpEnum::Always:
                return D3D12_COMPARISON_FUNC_ALWAYS;

            default:
                return D3D12_COMPARISON_FUNC_ALWAYS;
        }
    }

    D3D12_STENCIL_OP to_dx12_stencil_op(const shaderpack::StencilOpEnum op) {
        switch(op) {
            case shaderpack::StencilOpEnum::Keep:
                return D3D12_STENCIL_OP_KEEP;
            case shaderpack::StencilOpEnum::Zero:
                return D3D12_STENCIL_OP_ZERO;
            case shaderpack::StencilOpEnum::Replace:
                return D3D12_STENCIL_OP_REPLACE;
            case shaderpack::StencilOpEnum::Incr:
                return D3D12_STENCIL_OP_INCR;
            case shaderpack::StencilOpEnum::IncrWrap:
                return D3D12_STENCIL_OP_INCR_SAT;
            case shaderpack::StencilOpEnum::Decr:
                return D3D12_STENCIL_OP_DECR;
            case shaderpack::StencilOpEnum::DecrWrap:
                return D3D12_STENCIL_OP_DECR_SAT;
            case shaderpack::StencilOpEnum::Invert:
                return D3D12_STENCIL_OP_INVERT;
            default:
                return D3D12_STENCIL_OP_KEEP;
        }
    }

    D3D12_PRIMITIVE_TOPOLOGY_TYPE to_dx12_topology(const shaderpack::PrimitiveTopologyEnum primitive_mode) {
        switch(primitive_mode) {
            case shaderpack::PrimitiveTopologyEnum::Triangles:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

            case shaderpack::PrimitiveTopologyEnum::Lines:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

            default:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        }
    }

    enum DXGI_FORMAT to_dxgi_format(const shaderpack::PixelFormatEnum pixel_format) {
        switch(pixel_format) {
            case shaderpack::PixelFormatEnum::RGBA8:
                return DXGI_FORMAT_R8G8B8A8_UNORM;

            case shaderpack::PixelFormatEnum::RGBA16F:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;

            case shaderpack::PixelFormatEnum::RGBA32F:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;

            case shaderpack::PixelFormatEnum::Depth:
                return DXGI_FORMAT_D32_FLOAT;

            case shaderpack::PixelFormatEnum::DepthStencil:
                return DXGI_FORMAT_D24_UNORM_S8_UINT;

            default:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
        }
    }

    D3D12_DESCRIPTOR_RANGE_TYPE to_dx12_range_type(DescriptorType type) {
        switch(type) {
            case DescriptorType::CombinedImageSampler:
                return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

            case DescriptorType::UniformBuffer:
                return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;

            case DescriptorType::StorageBuffer:
                return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;

            default:
                return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        }
    }
} // namespace nova::renderer
#endif
