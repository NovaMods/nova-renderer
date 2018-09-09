/*!
 * \author ddubois 
 * \date 30-Aug-18.
 */

#ifndef NOVA_RENDERER_DX_12_RENDER_ENGINE_HPP
#define NOVA_RENDERER_DX_12_RENDER_ENGINE_HPP

#include "../render_engine.hpp"

#include <d3d12.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dxgi1_4.h>
#include "d3dx12.h"
#include "win32_window.hpp"

#include <wrl.h>

#include "dx12_opaque_types.hpp"

using Microsoft::WRL::ComPtr;

namespace nova {
    struct command_list {
        ComPtr<ID3D12CommandList> list;
        ComPtr<ID3D12CommandAllocator> allocator;
    };

    struct gfx_command_list {
        ComPtr<ID3D12GraphicsCommandList> list;
        ComPtr<ID3D12CommandAllocator> allocator;
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
         */
        explicit dx12_render_engine(const settings& settings);

        static const std::string get_engine_name();

        /**
         * render_engine overrides
         */

        void open_window(uint32_t width, uint32_t height) override;

        std::shared_ptr<iwindow> get_window() const override;

        void set_frame_graph() override;

        void render_frame() override;

    private:
        // direct3d stuff
        ComPtr<IDXGIFactory2> dxgi_factory;

        ComPtr<ID3D12Device> device; // direct3d device

        ComPtr<IDXGISwapChain3> swapchain; // swapchain used to switch between render targets

        ComPtr<ID3D12CommandQueue> direct_command_queue; // container for command lists

        ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap; // a descriptor heap to hold resources like the render targets

        ComPtr<ID3D12Resource> rendertargets[FRAME_BUFFER_COUNT]; // number of render targets equal to buffer count

        uint32_t rtv_descriptor_size; // size of the rtv descriptor on the device (all front and back buffers will be the same size)

        // Maps from command buffer type to command buffer list
        std::unordered_map<int, std::vector<command_list>> buffer_pool;

        uint32_t frame_index = 0;

        ComPtr<ID3D12Fence1> render_to_backbuffer_fence;
        HANDLE render_to_backbuffer_fence_event;


        void create_device();

        void create_rtv_command_queue();

        /*!
         * \brief Creates the swapchain from the size of the window
         *
         * This method has a precondition that the window must be initialized
         */
        void create_swapchain();

        std::shared_ptr<win32_window> window;

        /*!
         * \brief Creates the descriptor heap for the swapchain
         */
        void create_render_target_descriptor_heap();

        command_list allocate_command_list(D3D12_COMMAND_LIST_TYPE command_list_type);

        gfx_command_list get_graphics_command_list();
    };
}

#endif //NOVA_RENDERER_DX_12_RENDER_ENGINE_HPP
