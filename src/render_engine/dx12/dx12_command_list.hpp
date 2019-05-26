/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#ifndef NOVA_RENDERER_D3D12_COMMAND_LIST_HPP
#define NOVA_RENDERER_D3D12_COMMAND_LIST_HPP
#include <d3d12.h>
#include <nova_renderer/command_list.hpp>
#include <wrl/client.h>

#include "dx12_structs.hpp"

namespace nova::renderer::rhi {
    using namespace Microsoft::WRL;

    class Dx12CommandList final : public CommandList {
    public:
        ComPtr<ID3D12GraphicsCommandList> cmds;

        explicit Dx12CommandList(ComPtr<ID3D12GraphicsCommandList> cmds);

        void resource_barriers(PipelineStageFlags stages_before_barrier,
                               PipelineStageFlags stages_after_barrier,
                               const std::vector<ResourceBarrier>& barriers) override final;

        void copy_buffer(Buffer* destination_buffer,
                         uint64_t destination_offset,
                         Buffer* source_buffer,
                         uint64_t source_offset,
                         uint64_t num_bytes) override final;

        void execute_command_lists(const std::vector<CommandList*>& lists) override final;

        void begin_renderpass(Renderpass* renderpass, Framebuffer* framebuffer) override final;

        void end_renderpass() override final;
        void bind_pipeline() override final;
        void bind_material() override final;

        void bind_vertex_buffers() override final;
        void bind_index_buffer() override final;
        void draw_indexed_mesh() override final;
    };
} // namespace nova::renderer::rhi

#endif // NOVA_RENDERER_D3D12_COMMAND_LIST_HPP
