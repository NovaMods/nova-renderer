/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#include "d3d12_command_list.hpp"
#include "d3dx12.h"
#include "dx12_utils.hpp"

namespace nova::renderer {
    d3d12_command_list::d3d12_command_list(ComPtr<ID3D12GraphicsCommandList> cmds) : cmds(std::move(cmds)) {}

    void d3d12_command_list::resource_barriers([[maybe_unused]] pipeline_stage_flags stages_before_barrier,
                                              [[maybe_unused]] pipeline_stage_flags stages_after_barrier,
                                              const std::vector<resource_barrier_t>& barriers) {
        std::vector<D3D12_RESOURCE_BARRIER> dx12_barriers;
        dx12_barriers.reserve(barriers.size());

        for(const resource_barrier_t& barrier : barriers) {
            d3d12_resource_t* resource_to_barrier = reinterpret_cast<d3d12_resource_t*>(barrier.resource_to_barrier);

            if(barrier.access_after_barrier == 0) {
                const D3D12_RESOURCE_STATES initial_state = to_dx12_state(barrier.initial_state);
                const D3D12_RESOURCE_STATES final_state = to_dx12_state(barrier.final_state);
                dx12_barriers.push_back(
                    CD3DX12_RESOURCE_BARRIER::Transition(resource_to_barrier->resource.Get(), initial_state, final_state));

            } else {
                const CD3DX12_RESOURCE_BARRIER sync = CD3DX12_RESOURCE_BARRIER::UAV(resource_to_barrier->resource.Get());
                dx12_barriers.push_back(sync);
            }
        }

        cmds->ResourceBarrier(static_cast<UINT>(dx12_barriers.size()), dx12_barriers.data());
    }

    void d3d12_command_list::copy_buffer(resource_t* destination_buffer,
                                         const uint64_t destination_offset,
                                         resource_t* source_buffer,
                                         const uint64_t source_offset,
                                         const uint64_t num_bytes) {
        d3d12_resource_t* dst_buf = reinterpret_cast<d3d12_resource_t*>(destination_buffer);
        d3d12_resource_t* src_buf = reinterpret_cast<d3d12_resource_t*>(source_buffer);

        cmds->CopyBufferRegion(dst_buf->resource.Get(),
                               destination_offset,
            src_buf->resource.Get(),
                               source_offset,
                               num_bytes);
    }

    void d3d12_command_list::execute_command_lists(const std::vector<command_list*>& lists) {
        // Apparently D3D12 can only execute bundles from another command list, meaning that the strategy I use to
        // record command buffers in Vulkan won't work here...
        //
        // OR WILL IT??????!?!?!?!??!?!?!??!?!?!??!!?
        // 

        for(command_list* list : lists) {
            d3d12_command_list* d3d12_list = dynamic_cast<d3d12_command_list*>(list);
            cmds->ExecuteBundle(d3d12_list->cmds.Get());
        }
    }

    void d3d12_command_list::begin_renderpass([[maybe_unused]] renderpass_t* renderpass, framebuffer_t* framebuffer) {
        d3d12_framebuffer_t* d3d12_framebuffer = reinterpret_cast<d3d12_framebuffer_t*>(framebuffer);

        D3D12_CPU_DESCRIPTOR_HANDLE* depth_stencil = nullptr;
        if(d3d12_framebuffer->has_depth_stencil) {
            depth_stencil = &d3d12_framebuffer->depth_stencil_image;
        }
        cmds->OMSetRenderTargets(d3d12_framebuffer->render_targets.size(), d3d12_framebuffer->render_targets.data(), false, depth_stencil);
    }

} // namespace nova::renderer
