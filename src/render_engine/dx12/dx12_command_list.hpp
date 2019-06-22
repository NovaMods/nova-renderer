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
    class Dx12CommandList final : public CommandList {
    public:
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmds;

        explicit Dx12CommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmds);

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

        void bind_pipeline(const Pipeline* pipeline) override final;

        void bind_descriptor_sets(const std::vector<DescriptorSet*>& descriptor_sets,
                                  const PipelineInterface* pipeline_interface) override final;

        void bind_vertex_buffers(const std::vector<Buffer*>& buffers) override final;
        
        void bind_index_buffer(const Buffer* buffer) override final;
        
		void draw_indexed_mesh(uint32_t num_indices, uint32_t num_instances) override final;

    private:
        Microsoft::WRL::ComPtr<ID3D12Device> device;
    };
} // namespace nova::renderer::rhi

#endif // NOVA_RENDERER_D3D12_COMMAND_LIST_HPP
