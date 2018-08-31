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
#include "../../util/logger.hpp"
#include "win32_window.hpp"

namespace nova {
    /*!
     * \brief Implements a render engine for DirectX 12
     */
    class dx12_render_engine : public render_engine {
    public:
        /*!
         * \brief Initializes DX12
         * \param settings The settings that may or may not influence initializaion
         */
        explicit dx12_render_engine(const settings& settings);

        static const std::string get_engine_name();

        /**
         * render_engine overrides
         */

        void open_window(uint32_t width, uint32_t height) override;

    private:
        const logger& LOG;

        // direct3d stuff
        const uint32_t frameBufferCount = 3; // number of buffers we want, 2 for double buffering, 3 for triple buffering

        IDXGIFactory2* dxgi_factory;

        ID3D12Device* device; // direct3d device

        IDXGISwapChain3* swapchain; // swapchain used to switch between render targets

        ID3D12CommandQueue* direct_command_queue; // container for command lists

        ID3D12DescriptorHeap* rtvDescriptorHeap; // a descriptor heap to hold resources like the render targets

        ID3D12Resource* renderTargets[frameBufferCount]; // number of render targets equal to buffer count

        ID3D12CommandAllocator* commandAllocator[frameBufferCount]; // we want enough allocators for each buffer * number of threads (we only have one thread)

        ID3D12GraphicsCommandList* commandList; // a command list we can record commands into, then execute them to render the frame

        ID3D12Fence* fence[frameBufferCount];    // an object that is locked while our command list is being executed by the gpu. We need as many
        //as we have allocators (more if we want to know when the gpu is finished with an asset)

        HANDLE fenceEvent; // a handle to an event when our fence is unlocked by the gpu

        UINT64 fenceValue[frameBufferCount]; // this value is incremented each frame. each fence will have its own value

        int frame_index; // current rtv we are on

        int rtvDescriptorSize; // size of the rtv descriptor on the device (all front and back buffers will be the same size)


        void create_device();

        void create_rtv_command_queue();

        /*!
         * \brief Creates the swapchain from the size of the window
         *
         * This method has a precondition that the window must be initialized
         */
        void create_swapchain();

        std::unique_ptr<win32_window> window;
    };
}

#endif //NOVA_RENDERER_DX_12_RENDER_ENGINE_HPP
