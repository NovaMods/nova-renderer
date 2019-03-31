/*!
 * \author ddubois 
 * \date 30-Mar-19.
 */

#ifndef NOVA_RENDERER_D_3_D_12_COMMAND_LIST_HPP
#define NOVA_RENDERER_D3D_12_COMMAND_LIST_HPP
#include "nova_renderer/command_list.hpp"
#include <wrl/client.h>
#include <d3d12.h>

namespace nova::renderer {
    using namespace Microsoft::WRL;

    struct resource_t {
        ComPtr<ID3D12Resource> resource;
    };
        
    class d3d12_graphics_command_list : public command_list {
    public:
        explicit d3d12_graphics_command_list(ComPtr<ID3D12GraphicsCommandList> cmds);

        void resource_barrier(const std::vector<resource_barrier_t>& barriers) override;

        void copy_buffer() override;

        void execute_command_lists() override;

        void begin_renderpass() override;
        void end_renderpass() override;
        void bind_pipeline() override;
        void bind_material() override;

        void bind_vertex_buffers() override;
        void bind_index_buffers() override;
        void draw_indexed() override;

    private:
        ComPtr<ID3D12GraphicsCommandList> cmds;
    };
}

#endif //NOVA_RENDERER_D_3_D_12_COMMAND_LIST_HPP
