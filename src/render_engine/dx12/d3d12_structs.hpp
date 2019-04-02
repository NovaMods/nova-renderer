/*!
 * \brief D3D12 implementation of the structs forward-declared in render_engine.hpp
 *
 * \author ddubois 
 * \date 01-Apr-19.
 */

#pragma once

#include <nova_renderer/command_list.hpp>

namespace nova::renderer {
    using namespace Microsoft::WRL;

    struct d3d12_resource_t : resource_t {
        ComPtr<ID3D12Resource> resource;
    };

    struct d3d12_framebuffer_t : framebuffer_t {
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> render_targets;

        bool has_depth_stencil = false;
        D3D12_CPU_DESCRIPTOR_HANDLE depth_stencil_image = {};
    };
}
