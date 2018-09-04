/*!
 * \author ddubois 
 * \date 03-Sep-18.
 */

#include "dx12_command_buffer.hpp"

#if SUPPORT_DX12

namespace nova {

    dx12_graphics_command_buffer::dx12_graphics_command_buffer(const ComPtr<ID3D12Device> &device, const nova::command_buffer_type &type)
            : dx12_command_buffer<ID3D12GraphicsCommandList>(device, type), command_buffer_base(type) {
    }

    void dx12_graphics_command_buffer::clear_render_target(const std::shared_ptr<iframebuffer> framebuffer_to_clear, glm::vec4 &clear_color)  {
        for(const D3D12_CPU_DESCRIPTOR_HANDLE& resource : framebuffer_to_clear->color_attachments) {
            command_list->ClearRenderTargetView(resource, reinterpret_cast<FLOAT*>(&clear_color), 0, nullptr);
        }
    }

    void dx12_graphics_command_buffer::set_render_target(std::shared_ptr<iframebuffer> render_target) {
        command_list->OMSetRenderTargets(render_target->color_attachments.size(), render_target->color_attachments.data(),
                                         false, render_target->depth_stencil_descriptor);
    }

    void dx12_graphics_command_buffer::resource_barrier(const std::vector<resource_barrier_data> &barriers) {
        std::vector<CD3DX12_RESOURCE_BARRIER> dx12_barriers;
        dx12_barriers.reserve(barriers.size());

        for(const resource_barrier_data& barrier : barriers) {
            D3D12_RESOURCE_STATES initial_state = to_dx12_resource_state(barrier.initial_layout);
            D3D12_RESOURCE_STATES final_state = to_dx12_resource_state(barrier.final_layout);
            dx12_barriers.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(
                    reinterpret_cast<ID3D12Resource *>(barrier.resource_to_barrier.get()), initial_state, final_state));
        }

        command_list->ResourceBarrier(dx12_barriers.size(), dx12_barriers.data());
    }

    void dx12_graphics_command_buffer::reset() {
        dx12_command_buffer::reset();

        HRESULT hr = command_list->Reset(allocator.Get(), nullptr);
        if(FAILED(hr)) {
            NOVA_LOG(ERROR) << "Could not reset the command list";
        }
    }

    void dx12_graphics_command_buffer::end_recording() {
        command_list->Close();
    }
}

#endif
