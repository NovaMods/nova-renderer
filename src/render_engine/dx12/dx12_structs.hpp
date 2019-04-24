#pragma once

#include "d3dx12.h"
#include "nova_renderer/rhi_types.hpp"
#include "nova_renderer/shaderpack_data.hpp"

namespace nova::renderer::rhi {
    struct DX12Renderpass : Renderpass {};

    struct DX12Buffer : Buffer {
        ID3D12Resource* resource = nullptr;
    };

    struct DX12Image : Image {
        ID3D12Resource* resource = nullptr;
    };

    struct DX12Framebuffer : Framebuffer {
        std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> render_targets;

        bool has_depth_stencil = false;
        D3D12_CPU_DESCRIPTOR_HANDLE depth_stencil_image = {};

        ID3D12DescriptorHeap* descriptor_heap;
    };

    struct DX12PipelineInterface : PipelineInterface {
        // TODO: Information about descriptors

        std::vector<shaderpack::TextureAttachmentInfo> texture_outputs;
    };

    struct DX12Pipeline : Pipeline {
        ID3D12PipelineState* pso;
        ID3D12RootSignature* root_signature;
    };
} // namespace nova::renderer::rhi
