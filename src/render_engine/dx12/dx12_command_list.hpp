#pragma once

#pragma warning(push, 0)
#include <d3d12.h>
#pragma warning(pop)

#include "nova_renderer/rhi/command_list.hpp"

#pragma warning(push, 0)
#include <wrl/client.h>
#pragma warning(pop)

#include "dx12_structs.hpp"

namespace nova::renderer::rhi {
    class D3D12RenderEngine;

    class Dx12CommandList final : public CommandList {
    public:
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmds;

        explicit Dx12CommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmds);

        void resource_barriers(PipelineStageFlags stages_before_barrier,
                               PipelineStageFlags stages_after_barrier,
                               const std::pmr::vector<ResourceBarrier>& barriers) override;

        void copy_buffer(Buffer* destination_buffer,
                         mem::Bytes destination_offset,
                         Buffer* source_buffer,
                         mem::Bytes source_offset,
                         mem::Bytes num_bytes) override;

        void execute_command_lists(const std::pmr::vector<CommandList*>& lists) override;

        void begin_renderpass(Renderpass* renderpass, Framebuffer* framebuffer) override;

        void end_renderpass() override;

        void bind_pipeline(const Pipeline* pipeline) override;

        void bind_descriptor_sets(const std::pmr::vector<DescriptorSet*>& descriptor_sets,
                                  const PipelineInterface* pipeline_interface) override;

        void bind_vertex_buffers(const std::pmr::vector<Buffer*>& buffers) override;
        
        void bind_index_buffer(const Buffer* buffer) override;

        void draw_indexed_mesh(uint32_t num_indices, uint32_t num_instances) override;

        void upload_data_to_image(
            Image* image, size_t width, size_t height, size_t bytes_per_pixel, Buffer* staging_buffer, void* data) override;

        void upload_data_to_image(
            Image* image, size_t width, size_t height, size_t bytes_per_pixel, Buffer* staging_buffer, void* data) override;
    private:
        Microsoft::WRL::ComPtr<ID3D12Heap> staging_buffer_heap;
    };
} // namespace nova::renderer::rhi
