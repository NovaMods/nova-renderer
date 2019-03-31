/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#include "d3d12_command_list.hpp"
#include "d3dx12.h"
#include "dx12_utils.hpp"

namespace nova::renderer {
    d3d12_graphics_command_list::d3d12_graphics_command_list(ComPtr<ID3D12GraphicsCommandList> cmds) : cmds(std::move(cmds)) {}

    void d3d12_graphics_command_list::resource_barrier(const std::vector<resource_barrier_t>& barriers) {
        std::vector<D3D12_RESOURCE_BARRIER> dx12_barriers;
        dx12_barriers.reserve(barriers.size());

        for(const resource_barrier_t& barrier : barriers) {
            switch(barrier.barrier_type) {
                case resource_barrier_t::type::RESOURCE_TRANSITION:
                    const D3D12_RESOURCE_STATES initial_state = to_dx12_state(barrier.initial_state);
                    const D3D12_RESOURCE_STATES final_state = to_dx12_state(barrier.final_state);
                    const CD3DX12_RESOURCE_BARRIER transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(barrier.resource_to_barrier->resource.Get(), initial_state, final_state);
                    dx12_barriers.push_back(transition_barrier);
                    break;

                case resource_barrier_t::type::SYNCHRONIZATION:
                    const CD3DX12_RESOURCE_BARRIER sync_barrier = CD3DX12_RESOURCE_BARRIER::UAV(barrier.resource_to_barrier->resource.Get());
                    dx12_barriers.push_back(sync_barrier);
                    break;
            }
        }
    }

} // namespace nova::renderer
