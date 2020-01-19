#pragma once

#include "nova_renderer/memory//bytes.hpp"
#include "nova_renderer/rhi/rhi_types.hpp"
#include "nova_renderer/shaderpack_data.hpp"

#include "d3dx12.h"

namespace nova::renderer::rhi {
    struct DX12DeviceMemory : DeviceMemory {
        Microsoft::WRL::ComPtr<ID3D12Heap> heap;
    };

    struct DX12Renderpass : Renderpass {};

    struct DX12Buffer : Buffer {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
    };

    struct DX12Image : Image {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
    };

    struct DX12Framebuffer : Framebuffer {
        std::pmr::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> rtv_descriptors;

        std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> dsv_descriptor = {};
    };

    struct DX12PipelineInterface : PipelineInterface {
        std::pmr::unordered_map<uint32_t, std::pmr::vector<ResourceBindingDescription>> table_layouts;

        std::pmr::vector<shaderpack::TextureAttachmentInfo> color_attachments;

        std::optional<shaderpack::TextureAttachmentInfo> depth_texture;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> root_sig;
    };

    struct DX12DescriptorPool : DescriptorPool {
        // DX12 has a separate descriptor heap for each descriptor set in a material, meaning this struct can be
        // completely empty
    };

    struct DX12DescriptorSet : DescriptorSet {
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;

        std::pmr::vector<D3D12_CPU_DESCRIPTOR_HANDLE> descriptors;
    };

    struct DX12Pipeline : Pipeline {
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pso = nullptr;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature = nullptr;
    };

    struct DX12Fence : Fence {
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
        HANDLE event;
    };

    struct DX12Semaphore : Semaphore {
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    };
} // namespace nova::renderer::rhi
