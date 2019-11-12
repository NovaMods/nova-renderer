/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#ifndef NOVA_RENDERER_D3D12_COMMAND_LIST_HPP
#define NOVA_RENDERER_D3D12_COMMAND_LIST_HPP

#pragma warning(push, 0)
#include <d3d12.h>
#pragma warning(pop)

#include <nova_renderer/command_list.hpp>

#pragma warning(push, 0)
#include <wrl/client.h>
#pragma warning(pop)

#include "dx12_structs.hpp"

namespace nova::renderer::rhi {
    class Dx12CommandList final : public CommandList {
    public:
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmds;

        explicit Dx12CommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmds);

        void resource_barriers(PipelineStageFlags stages_before_barrier,
                               PipelineStageFlags stages_after_barrier,
                               const std::vector<ResourceBarrier>& barriers) override;

        void copy_buffer(Buffer* destination_buffer,
                         uint64_t destination_offset,
                         Buffer* source_buffer,
                         uint64_t source_offset,
                         uint64_t num_bytes) override;

        void execute_command_lists(const std::vector<CommandList*>& lists) override;

        void begin_renderpass(Renderpass* renderpass, Framebuffer* framebuffer) override;

        void end_renderpass() override;

        void bind_pipeline(const Pipeline* pipeline) override;

        void bind_descriptor_sets(const std::vector<DescriptorSet*>& descriptor_sets,
                                  const PipelineInterface* pipeline_interface) override;

        void bind_vertex_buffers(const std::vector<Buffer*>& buffers) override;
        
        void bind_index_buffer(const Buffer* buffer) override;
        
		void draw_indexed_mesh(uint32_t num_indices, uint32_t num_instances) override;

    private:
        Microsoft::WRL::ComPtr<ID3D12Device> device;
    };
} // namespace nova::renderer::rhi

#endif // NOVA_RENDERER_D3D12_COMMAND_LIST_HPP
