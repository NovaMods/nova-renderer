#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "d3d12_render_device.hpp"
#include "d3d12_structs.hpp"
#include "nova_renderer/rhi/command_list.hpp"

namespace nova::renderer::rhi {
    class D3D12RenderCommandList : RhiRenderCommandList {
    public:
        explicit D3D12RenderCommandList(rx::memory::allocator& allocator, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmds, D3D12RenderDevice& device_in);

        D3D12RenderCommandList(const D3D12RenderCommandList& other) = delete;
        D3D12RenderCommandList& operator=(const D3D12RenderCommandList& other) = delete;

        D3D12RenderCommandList(D3D12RenderCommandList&& old) noexcept = default;
        D3D12RenderCommandList& operator=(D3D12RenderCommandList&& old) noexcept = default;

        ~D3D12RenderCommandList() override = default;

#pragma region RhiRenderCommandList
        void set_debug_name(const rx::string& name) override;

        void set_checkpoint(const rx::string& checkpoint_name) override;

        void bind_material_resources(RhiBuffer& camera_buffer,
                                     RhiBuffer& material_buffer,
                                     RhiSampler& point_sampler,
                                     RhiSampler& bilinear_sampler,
                                     RhiSampler& trilinear_sampler,
                                     const rx::vector<RhiImage*>& images,
                                     rx::memory::allocator& allocator) override;

        void bind_resources(RhiResourceBinder& binder) override;

        void resource_barriers(PipelineStage stages_before_barrier,
                               PipelineStage stages_after_barrier,
                               const rx::vector<RhiResourceBarrier>& barriers) override;

        void copy_buffer(RhiBuffer& destination_buffer,
                         mem::Bytes destination_offset,
                         RhiBuffer& source_buffer,
                         mem::Bytes source_offset,
                         mem::Bytes num_bytes) override;

        void upload_data_to_image(
            RhiImage& image, size_t width, size_t height, size_t bytes_per_pixel, RhiBuffer& staging_buffer, const void* data) override;

        void execute_command_lists(const rx::vector<RhiRenderCommandList*>& lists) override;

        void set_camera(const Camera& camera) override;

        void begin_renderpass(RhiRenderpass& renderpass, RhiFramebuffer& framebuffer) override;

        void end_renderpass() override;

        void set_material_index(uint32_t index) override;

        void set_pipeline(const RhiPipeline& pipeline) override;

        void bind_vertex_buffers(const rx::vector<RhiBuffer*>& buffers) override;

        void bind_index_buffer(const RhiBuffer& buffer, IndexType index_size) override;

        void draw_indexed_mesh(uint32_t num_indices, uint32_t offset, uint32_t num_instances) override;

        void set_scissor_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
#pragma endregion

    ID3D12GraphicsCommandList* get_d3d12_list() const;

    private:
        rx::memory::allocator* internal_allocator;

        D3D12RenderDevice* device;

        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> command_list_4;

        D3D12Renderpass* current_renderpass = nullptr; 
    };
} // namespace nova::renderer::rhi
