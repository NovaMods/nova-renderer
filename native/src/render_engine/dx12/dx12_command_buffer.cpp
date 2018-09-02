/*!
 * \author ddubois 
 * \date 01-Sep-18.
 */

#include "dx12_command_buffer.hpp"
#include "../../util/logger.hpp"

#if SUPPORT_DX12

namespace nova {
    dx12_command_buffer::dx12_command_buffer(ID3D12Device* device, const command_buffer_type type) : command_buffer(type) {
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

    void dx12_command_buffer::on_completion(std::function<void(void)> completion_handler) {

    }
}

#endif