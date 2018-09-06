/*!
 * \author ddubois 
 * \date 03-Sep-18.
 */

#include "dx12_command_buffer.hpp"
#include "dx12_opaque_types.hpp"

#if SUPPORT_DX12

namespace nova {

    dx12_graphics_command_buffer::dx12_graphics_command_buffer(const ComPtr<ID3D12Device> &device, const nova::command_buffer_type &type)
            : dx12_command_buffer(device, type), command_buffer_base(type) {
        command_list->QueryInterface(IID_PPV_ARGS(&gfx_cmd_list));
        gfx_cmd_list->Close();
        fence_event = CreateEvent(nullptr, false, false, nullptr);
    }

    void dx12_graphics_command_buffer::clear_render_target(iframebuffer* framebuffer_to_clear, glm::vec4 &clear_color)  {
        for(const D3D12_CPU_DESCRIPTOR_HANDLE& resource : framebuffer_to_clear->color_attachments) {
            gfx_cmd_list->ClearRenderTargetView(resource, reinterpret_cast<FLOAT*>(&clear_color), 0, nullptr);
        }
    }

    void dx12_graphics_command_buffer::set_render_target(iframebuffer* render_target) {
        gfx_cmd_list->OMSetRenderTargets(render_target->color_attachments.size(),
                                                                       render_target->color_attachments.data(),
                                                                       false, render_target->depth_stencil_descriptor);
    }

    void dx12_graphics_command_buffer::resource_barrier(stage_flags source_stage_mask, stage_flags dest_state_mask,
                                                        const std::vector<resource_barrier_data>& memory_barriers,
                                                        const std::vector<buffer_barrier_data>& buffer_barriers,
                                                        const std::vector<image_barrier_data>& image_barriers) {
        std::vector<CD3DX12_RESOURCE_BARRIER> dx12_image_barriers;
        dx12_image_barriers.reserve(image_barriers.size());

        for(const image_barrier_data& barrier : image_barriers) {
            D3D12_RESOURCE_STATES initial_state = to_dx12_resource_state(barrier.initial_layout);
            D3D12_RESOURCE_STATES final_state = to_dx12_resource_state(barrier.final_layout);
            dx12_image_barriers.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(barrier.resource_to_barrier->descriptor.Get(), initial_state, final_state));
        }

        gfx_cmd_list->ResourceBarrier(dx12_image_barriers.size(), dx12_image_barriers.data());
    }

    void dx12_graphics_command_buffer::reset() {
        dx12_command_buffer::reset();

        HRESULT hr = gfx_cmd_list->Reset(allocator.Get(), nullptr);
        if(FAILED(hr)) {
            NOVA_LOG(ERROR) << "Could not reset the command list";
        }
    }

    void dx12_graphics_command_buffer::end_recording() {
        gfx_cmd_list->Close();
    }

    dx12_command_buffer::dx12_command_buffer(ComPtr<ID3D12Device> device, command_buffer_type type) : command_buffer_base(type) {
        HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create command buffer";
            throw std::runtime_error("Could not create command buffer");
        }

        D3D12_COMMAND_LIST_TYPE command_list_type;
        switch(type) {
            case command_buffer_type::GENERIC:
                command_list_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                break;
            case command_buffer_type::COPY:
                command_list_type = D3D12_COMMAND_LIST_TYPE_COPY;
                break;
            case command_buffer_type::COMPUTE:
                command_list_type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
                break;
            default:
                command_list_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                break;
        }

        hr = device->CreateCommandList(0, command_list_type, allocator.Get(), nullptr, IID_PPV_ARGS(&command_list));
        if(FAILED(hr)) {
            NOVA_LOG(ERROR) << "Could not create a command list of type " << (uint32_t)type;
            throw std::runtime_error("Could not create command list");
        }

        hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
        if(FAILED(hr)) {
            NOVA_LOG(ERROR) << "Could not create fence";
            throw std::runtime_error("Could not create fence");
        }
    }

    void dx12_command_buffer::reset() {
        HRESULT hr;
        hr = allocator->Reset();
        if(FAILED(hr)) {
            NOVA_LOG(WARN) << "Could not reset command list allocator, memory usage will likely increase dramatically";
        }
    }

    void dx12_command_buffer::on_completion(std::function<void(void)> completion_handler) {
        /*
         * Expected implementation:
         *
         * The completion handler gets added to a list of handlers. When the command buffer is submitted, an async task
         * will be created that waits for this command lists's fence, then executes all handlers in the order they were
         * added
         */
    }

    bool dx12_command_buffer::is_finished() const {
        return fence->GetCompletedValue() == fence_value;
    }

    void dx12_command_buffer::wait_until_completion() const {
        HRESULT hr = fence->SetEventOnCompletion(fence_value, fence_event);
        if(FAILED(hr)) {
            NOVA_LOG(ERROR) << "Could not tell the fence to signal our event";
            return;
        }

        WaitForSingleObject(fence_event, INFINITE);
    }
}

#endif
