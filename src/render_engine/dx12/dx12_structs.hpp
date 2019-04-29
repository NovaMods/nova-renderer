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

        ID3D12DescriptorHeap* descriptor_heap = nullptr;
    };

    struct DX12PipelineInterface : PipelineInterface {
        std::unordered_map<uint32_t, std::vector<ResourceBindingDescription>> table_layouts;

        D3D12_ROOT_SIGNATURE_DESC1 root_sig_desc = {};

        std::vector<shaderpack::TextureAttachmentInfo> color_attachments;

        std::optional<shaderpack::TextureAttachmentInfo> depth_texture;
    };

    struct DX12Pipeline : Pipeline {
        ID3D12PipelineState* pso = nullptr;
        ID3D12RootSignature* root_signature = nullptr;
    };
} // namespace nova::renderer::rhi
