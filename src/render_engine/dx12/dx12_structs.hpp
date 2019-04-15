/*!
 * \brief D3D12 implementation of the structs forward-declared in render_engine.hpp
 *
 * \author ddubois 
 * \date 01-Apr-19.
 */

#pragma once

#include "nova_renderer/command_list.hpp"
#include "d3dx12.h"

namespace nova::renderer::rhi {
    struct DX12Renderpass : Renderpass {};

    struct DX12Buffer : Buffer {
        ID3D12Resource* resource;
    };

    struct DX12Image : Image {
        ID3D12Resource* resource;
    };

    struct DX12Framebuffer : Framebuffer {
        std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> render_targets;

        bool has_depth_stencil = false;
        D3D12_CPU_DESCRIPTOR_HANDLE depth_stencil_image = {};

        ID3D12DescriptorHeap* descriptor_heap;
    };

    struct DX12Pipeline : Pipeline {};
}
