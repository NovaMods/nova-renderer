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
#include "dx12_resource_barrier_helpers.hpp"
#include "d3dx12.h"

#include "dx12_opaque_types.hpp"

#include "../../util/logger.hpp"

using Microsoft::WRL::ComPtr;

namespace nova {
    /*!
     * \brief The DirectX 12 implementation of a command buffer
     */
    class dx12_command_buffer : public virtual command_buffer_base {
    public:
        ComPtr<ID3D12CommandList> command_list;
        ComPtr<ID3D12Fence> fence;
        uint64_t fence_value = 0;
        HANDLE fence_event;

        dx12_command_buffer(ComPtr<ID3D12Device> device, command_buffer_type type);

        dx12_command_buffer(dx12_command_buffer&& other) noexcept = default;
        dx12_command_buffer& operator=(dx12_command_buffer&& other) noexcept = default;

        // No copying!
        dx12_command_buffer(const dx12_command_buffer& other) = delete;
        dx12_command_buffer& operator=(const dx12_command_buffer& other) = delete;

        void end_recording() override {};

        void reset() override;

        void on_completion(std::function<void(void)> completion_handler) override;

        bool is_finished() const override;

        void wait_until_completion() const override;

    protected:
        ComPtr<ID3D12CommandAllocator> allocator;
    };

    class dx12_graphics_command_buffer : public dx12_command_buffer, public virtual graphics_command_buffer_base {
    public:
        dx12_graphics_command_buffer(const ComPtr<ID3D12Device> &device, const command_buffer_type &type);

        void resource_barrier(stage_flags source_stage_mask, stage_flags dest_state_mask,
                              const std::vector<resource_barrier_data>& memory_barriers,
                              const std::vector<buffer_barrier_data>& buffer_barriers,
                              const std::vector<image_barrier_data>& image_barriers) override;

        void clear_render_target(iframebuffer* framebuffer_to_clear, glm::vec4& clear_color) override;

        void set_render_target(iframebuffer* render_target) override;

        void end_recording() override;

        void reset() override;

    private:
        ComPtr<ID3D12GraphicsCommandList> gfx_cmd_list;
    };
}

#endif

#endif //NOVA_RENDERER_COMMAND_LIST_HPP
