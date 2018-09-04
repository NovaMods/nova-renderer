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

using Microsoft::WRL::ComPtr;

namespace nova {
#define FRAME_BUFFER_COUNT 3
    struct iresource {
        ComPtr<ID3D12Resource> descriptor;
    };

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

        std::shared_ptr<iframebuffer> get_current_swapchain_framebuffer() const override;

        std::shared_ptr<iresource> get_current_swapchain_image() const override;

        std::shared_ptr<command_buffer_base> allocate_command_buffer(command_buffer_type type) override;

        void execute_command_buffers(const std::vector<std::shared_ptr<command_buffer_base>>& buffers) override;

        void free_command_buffer(std::shared_ptr<command_buffer_base> buf) override;

        void present_swapchain_image() override;

    private:
        // direct3d stuff
        ComPtr<IDXGIFactory2> dxgi_factory;

        ComPtr<ID3D12Device> device; // direct3d device

        ComPtr<IDXGISwapChain3> swapchain; // swapchain used to switch between render targets

        ComPtr<ID3D12CommandQueue> direct_command_queue; // container for command lists

        ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap; // a descriptor heap to hold resources like the render targets

        ComPtr<ID3D12Resource> rendertargets[FRAME_BUFFER_COUNT]; // number of render targets equal to buffer count

        ComPtr<ID3D12Fence> fence[FRAME_BUFFER_COUNT];    // an object that is locked while our command list is being executed by the gpu. We need as many
        //as we have allocators (more if we want to know when the gpu is finished with an asset)

        HANDLE fenceEvent; // a handle to an event when our fence is unlocked by the gpu

        UINT64 fenceValue[FRAME_BUFFER_COUNT]; // this value is incremented each frame. each fence will have its own value

        int frame_index; // current rtv we are on

        uint32_t rtv_descriptor_size; // size of the rtv descriptor on the device (all front and back buffers will be the same size)

        std::unordered_map<int, std::vector<std::shared_ptr<command_buffer_base>>> buffer_pool;


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
    };
}

#endif //NOVA_RENDERER_DX_12_RENDER_ENGINE_HPP
