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
            if(barrier.access_after_barrier == 0) {
                const D3D12_RESOURCE_STATES initial_state = to_dx12_state(barrier.initial_state);
                const D3D12_RESOURCE_STATES final_state = to_dx12_state(barrier.final_state);
                dx12_barriers.push_back(
                    CD3DX12_RESOURCE_BARRIER::Transition(barrier.resource_to_barrier->resource.Get(), initial_state, final_state));

            } else {
                const CD3DX12_RESOURCE_BARRIER sync = CD3DX12_RESOURCE_BARRIER::UAV(barrier.resource_to_barrier->resource.Get());
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
        cmds->CopyBufferRegion(destination_buffer->resource.Get(),
                               destination_offset,
                               source_buffer->resource.Get(),
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

} // namespace nova::renderer
