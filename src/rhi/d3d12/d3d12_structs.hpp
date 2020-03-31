#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "nova_renderer/rhi/rhi_types.hpp"

namespace nova::renderer::rhi {
    struct D3D12RenderPass : RhiRenderpass {
        rx::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> render_target_descriptions;
        rx::optional<D3D12_RENDER_PASS_DEPTH_STENCIL_DESC> depth_stencil_description;

        D3D12_RENDER_PASS_FLAGS flags;
    };

    struct D3D12Framebuffer : RhiFramebuffer {
        rx::vector<D3D12_CPU_DESCRIPTOR_HANDLE> render_target_descriptors;
        rx::optional<D3D12_CPU_DESCRIPTOR_HANDLE> depth_stencil_descriptor;
    };

    struct D3D12Image : RhiImage {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        DXGI_FORMAT format;
    };

    struct D3D12Pipeline : RhiPipeline {
        Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader_bytecode{};
        Microsoft::WRL::ComPtr<ID3DBlob> geometry_shader_bytecode{};
        Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader_bytecode{};
    };
} // namespace nova::renderer::rhi
