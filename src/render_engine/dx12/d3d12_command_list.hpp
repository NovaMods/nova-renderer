/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#ifndef NOVA_RENDERER_D3D12_COMMAND_LIST_HPP
#define NOVA_RENDERER_D3D12_COMMAND_LIST_HPP
#include <d3d12.h>
#include <wrl/client.h>
#include <nova_renderer/command_list.hpp>

#include "d3d12_structs.hpp"

namespace nova::renderer::rhi {
    using namespace Microsoft::WRL;

    class d3d12_command_list : public command_list_t {
    public:
        explicit d3d12_command_list(ComPtr<ID3D12GraphicsCommandList> cmds);

        void resource_barriers([[maybe_unused]] pipeline_stage_flags stages_before_barrier,
                              [[maybe_unused]] pipeline_stage_flags stages_after_barrier,
                              const std::vector<resource_barrier_t>& barriers) override final;

        void copy_buffer(resource_t* destination_buffer,
                         uint64_t destination_offset,
                         resource_t* source_buffer,
                         uint64_t source_offset,
                         uint64_t num_bytes) override final;

        void execute_command_lists(const std::vector<command_list_t*>& lists) override final;

        void begin_renderpass([[maybe_unused]] renderpass_t* renderpass, framebuffer_t* framebuffer) override final;
        void end_renderpass() override final;
        void bind_pipeline() override final;
        void bind_material() override final;

        void bind_vertex_buffers() override final;
        void bind_index_buffer() override final;
        void draw_indexed_mesh() override final;

    private:
        ComPtr<ID3D12GraphicsCommandList> cmds;
    };
} // namespace nova::renderer

#endif // NOVA_RENDERER_D3D12_COMMAND_LIST_HPP
