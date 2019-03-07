/*!
 * \author ddubois
 * \date 30-Aug-18.
 */

#ifndef NOVA_RENDERER_DX_12_RENDER_ENGINE_HPP
#define NOVA_RENDERER_DX_12_RENDER_ENGINE_HPP

#include "../render_engine.hpp"

#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_2.h>
#include <dxgi1_4.h>
#include "win32_window.hpp"

#include <memory>
#include <mutex>
#include <spirv_hlsl.hpp>
#include <unordered_map>
#include <wrl.h>
#include "../../loading/shaderpack/render_graph_builder.hpp"
#include "dx12_texture.hpp"

using Microsoft::WRL::ComPtr;

namespace nova::ttl {
    class task_scheduler;
}

namespace nova::renderer {

    struct command_list_base {
        ComPtr<ID3D12CommandAllocator> allocator;
        D3D12_COMMAND_LIST_TYPE type;
        ComPtr<ID3D12Fence> submission_fence;
        uint32_t fence_value = 0;
        bool is_done = false;
    };

    struct command_list : public command_list_base {
        ComPtr<ID3D12CommandList> list;
    };

    struct gfx_command_list : public command_list_base {
        ComPtr<ID3D12GraphicsCommandList> list;
    };

    struct pipeline {
        ComPtr<ID3D12PipelineState> pso;
        ComPtr<ID3D12RootSignature> root_signature;
    };

#define FRAME_BUFFER_COUNT 3
    /*!
     * \brief Implements a render engine for DirectX 12
     */
    class dx12_render_engine : public render_engine {
    public:
        /*!
         * \brief Initializes DX12
         * \param settings The settings that may or may not influence initialization
         * \param scheduler The task scheduler that this render engine should use
         */
        explicit dx12_render_engine(const nova_settings& settings, nova::ttl::task_scheduler* scheduler);

        static const std::string get_engine_name();

        /**
         * render_engine overrides
         */

        std::shared_ptr<iwindow> get_window() const override;

        void set_shaderpack(const shaderpack_data& data) override;

        mesh_id_t add_mesh(const mesh_data&) override;

        void delete_mesh(uint32_t) override;

        void render_frame() override;

    private:
        // direct3d stuff
        ComPtr<IDXGIFactory2> dxgi_factory;

        ComPtr<ID3D12Device> device; // direct3d device

        ComPtr<IDXGISwapChain3> swapchain; // swapchain used to switch between render targets

        ComPtr<ID3D12CommandQueue> direct_command_queue; // container for command lists

        ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap; // a descriptor heap to hold resources like the render targets

        std::vector<ComPtr<ID3D12Resource>> rendertargets; // number of render targets equal to buffer count

        ComPtr<ID3D12QueryHeap> renderpass_timestamp_query_heap;

        uint32_t rtv_descriptor_size; // size of the rtv descriptor on the device (all front and back buffers will be the same size)

        // Maps from command buffer type to command buffer list
        std::unordered_map<D3D12_COMMAND_LIST_TYPE, std::vector<command_list_base*>> buffer_pool;
        std::mutex buffer_pool_mutex;

        /*
         * Synchronization is hard
         *
         * Nova renders the first frame. It gets all the command lists it needs, records them, and they get added to
         * this map.
         *
         * Nova renders the second frame. It looks at all these command lists to see if any are done executing. If so,
         * they can be reused. If not, it makes new ones
         *
         * Nova renders the third frame. Once again it reuses any command lists that have finished executing, and
         * allocates whatever it needs.
         *
         * Now we get to the fourth frame. Nova has a maximum number of in-flight frames, which defaults to three. The
         * fourth frame has to wait for the first frame to finish
         */
        std::vector<command_list_base*> lists_to_free;
        std::mutex lists_to_free_mutex;

        uint32_t frame_index = 0;

        uint32_t num_in_flight_frames = 3;

        std::vector<ComPtr<ID3D12Fence>> frame_fences;
        std::vector<uint32_t> frame_fence_values;
        HANDLE full_frame_fence_event;

        /*!
         * \brief All the textures that the loaded shaderpack has created
         */
        std::unordered_map<std::string, dx12_texture> dynamic_textures;
        std::unordered_map<std::string, uint32_t> dynamic_tex_name_to_idx;

        /*!
         * \brief The passes in the current frame graph, in submission order
         */
        std::unordered_map<std::string, render_pass_data> render_passes;
        std::vector<std::string> ordered_passes;

        std::shared_ptr<win32_window> window;

        void create_device();

        void create_rtv_command_queue();

    protected:
        void open_window(uint32_t width, uint32_t height) override;

    private:
        /*!
         * \brief Creates the swapchain from the size of the window
         *
         * This method has a precondition that the window must be initialized
         */
        void create_swapchain();

        /*!
         * \brief Creates the descriptor heap for the swapchain
         */
        void create_render_target_descriptor_heap();

        command_list* allocate_command_list(D3D12_COMMAND_LIST_TYPE command_list_type) const;

        gfx_command_list* get_graphics_command_list();

        void release_command_list(command_list_base* list);

        void create_full_frame_fences();

        void wait_for_previous_frame();

        void try_to_free_command_lists();

        void create_dynamic_textures(const std::vector<texture_resource_data>& texture_datas, std::vector<render_pass_data> passes);

        void make_pipeline_state_objects(const std::vector<pipeline_data>& pipelines, ttl::task_scheduler* scheduler);

        pipeline make_single_pso(const pipeline_data& input);

        ComPtr<ID3D12RootSignature> create_root_signature(
            const std::unordered_map<uint32_t, std::vector<D3D12_DESCRIPTOR_RANGE1>>& tables) const;

        /*!
         * \brief Creates a timestamp query heap with enough space to time every render pass
         *
         * \param num_queries The number of queries the heap needs to support
         */
        void create_gpu_query_heap(size_t num_queries);
    };

    /*!
     * \brief Compiles the provided shader_source into a DirectX 12 shader
     *
     * \param shader The shader_source to compile. `shader.source` should be SPIR-V code
     * \param target The shader target to compile to. Shader model 5.1 is recommended
     * \param options Any options to use when compiling this shader
     * \param tables All the descriptor tables that this shader declares
     *
     * \return The compiled shader
     */
    ComPtr<ID3DBlob> compile_shader(const shader_source& shader,
                                    const std::string& target,
                                    const spirv_cross::CompilerHLSL::Options& options,
                                    std::unordered_map<uint32_t, std::vector<D3D12_DESCRIPTOR_RANGE1>>& tables);

    bool operator==(const D3D12_ROOT_PARAMETER1& param1, const D3D12_ROOT_PARAMETER1& param2);
    bool operator!=(const D3D12_ROOT_PARAMETER1& param1, const D3D12_ROOT_PARAMETER1& param2);

    bool operator==(const D3D12_ROOT_DESCRIPTOR_TABLE1& table1, const D3D12_ROOT_DESCRIPTOR_TABLE1& table2);
    bool operator!=(const D3D12_ROOT_DESCRIPTOR_TABLE1& table1, const D3D12_ROOT_DESCRIPTOR_TABLE1& table2);

    bool operator==(const D3D12_DESCRIPTOR_RANGE1& range1, const D3D12_DESCRIPTOR_RANGE1& range2);
    bool operator!=(const D3D12_DESCRIPTOR_RANGE1& range1, const D3D12_DESCRIPTOR_RANGE1& range2);

    bool operator==(const D3D12_ROOT_CONSTANTS& lhs, const D3D12_ROOT_CONSTANTS& rhs);

    bool operator==(const D3D12_ROOT_DESCRIPTOR1& lhs, const D3D12_ROOT_DESCRIPTOR1& rhs);
} // namespace nova::renderer

#endif // NOVA_RENDERER_DX_12_RENDER_ENGINE_HPP
