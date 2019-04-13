/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#include "dx12_command_list.hpp"
#include "d3dx12.h"
#include "dx12_utils.hpp"

namespace nova::renderer::rhi {
    DX12CommandList::DX12CommandList(ComPtr<ID3D12GraphicsCommandList> cmds) : cmds(std::move(cmds)) {}

    void DX12CommandList::resource_barriers([[maybe_unused]] PipelineStageFlags stages_before_barrier,
                                               [[maybe_unused]] PipelineStageFlags stages_after_barrier,
                                               const std::vector<ResourceBarrier>& barriers) {
        std::vector<D3D12_RESOURCE_BARRIER> dx12_barriers;
        dx12_barriers.reserve(barriers.size());

        for(const ResourceBarrier& barrier : barriers) {
            ID3D12Resource* resource_to_barrier;
            switch(barrier.resource_to_barrier->type) {
                case Resource::Buffer: {
                    DX12Buffer* d3d12_buffer = static_cast<DX12Buffer*>(barrier.resource_to_barrier);
                    resource_to_barrier = d3d12_buffer->resource;
                } break;

                case Resource::Image: {
                    DX12Image* d3d12_image = static_cast<DX12Image*>(barrier.resource_to_barrier);
                    resource_to_barrier = d3d12_image->resource;
                } break;
                default:;
            }

            if(barrier.access_after_barrier == 0) {
                const D3D12_RESOURCE_STATES initial_state = to_dx12_state(barrier.initial_state);
                const D3D12_RESOURCE_STATES final_state = to_dx12_state(barrier.final_state);
                // ReSharper disable once CppLocalVariableMightNotBeInitialized
                dx12_barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(resource_to_barrier, initial_state, final_state));

            } else {
                // ReSharper disable once CppLocalVariableMightNotBeInitialized
                const CD3DX12_RESOURCE_BARRIER sync = CD3DX12_RESOURCE_BARRIER::UAV(resource_to_barrier);
                dx12_barriers.push_back(sync);
            }
        }

        cmds->ResourceBarrier(static_cast<UINT>(dx12_barriers.size()), dx12_barriers.data());
    }

    void DX12CommandList::copy_buffer(Buffer* destination_buffer,
                                         const uint64_t destination_offset,
                                         Buffer* source_buffer,
                                         const uint64_t source_offset,
                                         const uint64_t num_bytes) {
        DX12Buffer* dst_buf = reinterpret_cast<DX12Buffer*>(destination_buffer);
        DX12Buffer* src_buf = reinterpret_cast<DX12Buffer*>(source_buffer);

        cmds->CopyBufferRegion(dst_buf->resource, destination_offset, src_buf->resource, source_offset, num_bytes);
    }

    void DX12CommandList::execute_command_lists(const std::vector<CommandList*>& lists) {
        // Apparently D3D12 can only execute bundles from another command list, meaning that the strategy I use to
        // record command buffers in Vulkan won't work here...
        //
        // OR WILL IT??????!?!?!?!??!?!?!??!?!?!??!!?
        //

        for(CommandList* list : lists) {
            DX12CommandList* d3d12_list = dynamic_cast<DX12CommandList*>(list);
            cmds->ExecuteBundle(d3d12_list->cmds.Get());
        }
    }

    void DX12CommandList::begin_renderpass([[maybe_unused]] Renderpass* renderpass, Framebuffer* framebuffer) {
        DX12Framebuffer* d3d12_framebuffer = reinterpret_cast<DX12Framebuffer*>(framebuffer);

        D3D12_CPU_DESCRIPTOR_HANDLE* depth_stencil = nullptr;
        if(d3d12_framebuffer->has_depth_stencil) {
            depth_stencil = &d3d12_framebuffer->depth_stencil_image;
        }
        cmds->OMSetRenderTargets(d3d12_framebuffer->render_targets.size(), d3d12_framebuffer->render_targets.data(), false, depth_stencil);
    }

} // namespace nova::renderer::rhi
