/*!
 * \author ddubois 
 * \date 01-Sep-18.
 */

#ifndef NOVA_RENDERER_COMMAND_LIST_HPP
#define NOVA_RENDERER_COMMAND_LIST_HPP

#include "../../platform.hpp"

#if SUPPORT_DX12

#include <d3d12.h>

#include "../command_buffer.hpp"
#include "../../util/logger.hpp"

namespace nova {
    /*!
     * \brief The DirectX 12 implementation of a command buffer
     */
    template<typename CommandListType>
    class dx12_command_buffer : public command_buffer {
    public:
        dx12_command_buffer(ID3D12Device* device, command_buffer_type type);

        dx12_command_buffer(dx12_command_buffer&& other) noexcept = default;
        dx12_command_buffer& operator=(dx12_command_buffer&& other) noexcept = default;

        // No copying!
        dx12_command_buffer(const dx12_command_buffer& other) = delete;
        dx12_command_buffer& operator=(const dx12_command_buffer& other) = delete;

        void reset() override;

        void on_completion(std::function<void(void)> completion_handler) override;

    private:
        ID3D12CommandAllocator* allocator;
        CommandListType* command_list;
        ID3D12Fence* fence;
    };

    template<typename CommandBufferType>
    dx12_command_buffer<CommandBufferType>::dx12_command_buffer(ID3D12Device* device, const command_buffer_type type) : command_buffer(type) {
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

        hr = device->CreateCommandList(0, command_list_type, allocator, nullptr, IID_PPV_ARGS(&command_list));
        if(FAILED(hr)) {
            NOVA_LOG(ERROR) << "Could not create a command list of type " << type.to_string();
            throw std::runtime_error("Could not create command list");
        }

        if(type == command_buffer_type::GENERIC) {
            // Get the command list out of the recording state
            dynamic_cast<ID3D12GraphicsCommandList*>(command_list)->Close();
        }

        hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
        if(FAILED(hr)) {
            NOVA_LOG(ERROR) << "Could not create fence";
            throw std::runtime_error("Could not create fence");
        }
    }

    template <typename CommandBufferType>
    void dx12_command_buffer<CommandBufferType>::on_completion(std::function<void(void)> completion_handler) {

    }

    template <>
    void dx12_command_buffer<ID3D12GraphicsCommandList>::reset() {
        HRESULT hr;
        hr = allocator->Reset();
        if(FAILED(hr)) {
            NOVA_LOG(WARN) << "Could not reset command list allocator, memory usage will likely increase dramatically";
        }

        hr = command_list->Reset(allocator, nullptr);
        if(FAILED(hr)) {
            NOVA_LOG(ERROR) << "Could not reset the command list";
        }
    }

    template <>
    void dx12_command_buffer<ID3D12CommandList>::reset() {
        HRESULT hr;
        hr = allocator->Reset();
        if(FAILED(hr)) {
            NOVA_LOG(WARN) << "Could not reset command list allocator, memory usage will likely increase dramatically";
        }
    }
}

#endif

#endif //NOVA_RENDERER_COMMAND_LIST_HPP
