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

namespace nova {
    /*!
     * \brief The DirectX 12 implementation of a command buffer
     */
    class dx12_command_buffer : public command_buffer {
    public:
        dx12_command_buffer(ID3D12Device* device, command_buffer_type type);

        dx12_command_buffer(dx12_command_buffer&& other) noexcept = default;
        dx12_command_buffer& operator=(dx12_command_buffer&& other) noexcept = default;

        // No copying!
        dx12_command_buffer(const dx12_command_buffer& other) = delete;
        dx12_command_buffer& operator=(const dx12_command_buffer& other) = delete;

        void on_completion(std::function<void(void)> completion_handler) override;

    private:
        ID3D12CommandAllocator* allocator;
        ID3D12CommandList* command_list;
        ID3D12Fence* fence;
    };
}

#endif

#endif //NOVA_RENDERER_COMMAND_LIST_HPP
