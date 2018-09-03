/*!
 * \author ddubois 
 * \date 01-Sep-18.
 */

#ifndef NOVA_RENDERER_COMMAND_LIST_HPP
#define NOVA_RENDERER_COMMAND_LIST_HPP

#include "../../platform.hpp"

#if SUPPORT_DX12

#include <d3d12.h>
#include <wrl.h>

#include "../command_buffer_base.hpp"
#include "dx_12_framebuffer.hpp"
#include "dx_12_resource_barrier_helpers.hpp"
#include "d3dx12.h"

#include "../../util/logger.hpp"

using Microsoft::WRL::ComPtr;

namespace nova {
    struct iresource {
        CD3DX12_CPU_DESCRIPTOR_HANDLE descriptor;
    };

    /*!
     * \brief The DirectX 12 implementation of a command buffer
     */
    template<typename CommandListType>
    class dx12_command_buffer : public virtual command_buffer_base {
    public:
        dx12_command_buffer(ComPtr<ID3D12Device> device, command_buffer_type type) : command_buffer_base(type) {
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
                NOVA_LOG(ERROR) << "Could not create a command list of type " << type.to_string();
                throw std::runtime_error("Could not create command list");
            }

            if(type == command_buffer_type::GENERIC) {
                // Get the command list out of the recording state
                dynamic_cast<ID3D12GraphicsCommandList*>(command_list.Get())->Close();
            }

            hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
            if(FAILED(hr)) {
                NOVA_LOG(ERROR) << "Could not create fence";
                throw std::runtime_error("Could not create fence");
            }
        }

        dx12_command_buffer(dx12_command_buffer&& other) noexcept = default;
        dx12_command_buffer& operator=(dx12_command_buffer&& other) noexcept = default;

        // No copying!
        dx12_command_buffer(const dx12_command_buffer& other) = delete;
        dx12_command_buffer& operator=(const dx12_command_buffer& other) = delete;

        void end_recording() override {};

        void reset() override {
            HRESULT hr;
            hr = allocator->Reset();
            if(FAILED(hr)) {
                NOVA_LOG(WARN) << "Could not reset command list allocator, memory usage will likely increase dramatically";
            }
        };

        void on_completion(std::function<void(void)> completion_handler) override {
            /*
             * Expected implementation:
             *
             * The completion handler gets added to a list of handlers. When the command buffer is submitted, an async task
             * will be created that waits for this command lists's fence, then executes all handlers in the order they were
             * added
             */
        };

    protected:
        ComPtr<ID3D12CommandAllocator> allocator;
        ComPtr<CommandListType> command_list;
        ComPtr<ID3D12Fence> fence;
    };

    class dx12_graphics_command_buffer : public dx12_command_buffer<ID3D12GraphicsCommandList>, public virtual graphics_command_buffer_base {
    public:
        dx12_graphics_command_buffer(const ComPtr<ID3D12Device> &device, const command_buffer_type &type);

        void resource_barrier(const std::vector<resource_barrier_data>& barriers) override;

        void clear_render_target(const iframebuffer* framebuffer_to_clear, glm::vec4& clear_color) override;

        void set_render_target(framebuffer_ptr render_target) override;

        void end_recording() override;

        void reset() override;
    };
}

#endif

#endif //NOVA_RENDERER_COMMAND_LIST_HPP
