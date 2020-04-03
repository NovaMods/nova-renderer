#pragma once

#include <D3D12MemAlloc.h>
#include <d3d12.h>
#include <d3d12shader.h>
#include <dxc/dxcapi.h>
#include <wrl/client.h>

#include "nova_renderer/rhi/pipeline_create_info.hpp"
#include "nova_renderer/rhi/rhi_types.hpp"

namespace nova::renderer::rhi {
    constexpr uint32_t CPU_FENCE_UNSIGNALED = 0;
    constexpr uint32_t CPU_FENCE_SIGNALED = 16;
    constexpr uint32_t GPU_FENCE_SIGNALED = 32;

    struct D3D12Renderpass : RhiRenderpass {
        rx::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> render_target_descriptions;
        rx::optional<D3D12_RENDER_PASS_DEPTH_STENCIL_DESC> depth_stencil_description;

        D3D12_RENDER_PASS_FLAGS flags;

        rx::map<rx::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> cached_pipelines;
    };

    struct D3D12Framebuffer : RhiFramebuffer {
        rx::vector<D3D12_CPU_DESCRIPTOR_HANDLE> render_target_descriptors;
        rx::optional<D3D12_CPU_DESCRIPTOR_HANDLE> depth_stencil_descriptor;
    };

    struct D3D12Image : RhiImage {
        DXGI_FORMAT format;
        D3D12MA::Allocation* allocation;
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    };

    struct D3D12Buffer : RhiBuffer {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12MA::Allocation* alloc;
    };

    struct D3D12Sampler : RhiSampler {
        D3D12_SAMPLER_DESC desc;
    };

    struct D3D12Pipeline : RhiPipeline {
        Microsoft::WRL::ComPtr<IDxcBlob> vertex_shader_bytecode{};
        Microsoft::WRL::ComPtr<IDxcBlob> geometry_shader_bytecode{};
        Microsoft::WRL::ComPtr<IDxcBlob> pixel_shader_bytecode{};

        RhiGraphicsPipelineState create_info;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature;

        rx::map<rx::string, D3D12_SHADER_INPUT_BIND_DESC> bindings;
        rx::map<rx::string, D3D12_CPU_DESCRIPTOR_HANDLE> descriptors;
    };

    struct D3D12Semaphore : RhiSemaphore {
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    };

    struct D3D12Fence : RhiFence {
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    };
} // namespace nova::renderer::rhi
