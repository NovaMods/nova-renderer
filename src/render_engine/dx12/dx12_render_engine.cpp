/*!
 * \author ddubois
 * \date 03-Apr-19.
 */

#include <d3d12sdklayers.h>

#include "../../util/logger.hpp"
#include "../../util/windows_utils.hpp"
#include "d3dx12.h"
#include "dx12_command_list.hpp"
#include "dx12_render_engine.hpp"
#include "dx12_structs.hpp"
#include "dx12_utils.hpp"

using Microsoft::WRL::ComPtr;

namespace nova::renderer::rhi {
    DX12RenderEngine::DX12RenderEngine(NovaSettings& settings) : RenderEngine(settings) {
        create_device();

        open_window_and_create_surface(settings.window);

        create_queues();

        rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    std::shared_ptr<window_t> DX12RenderEngine::get_window() const { return window; }

    void DX12RenderEngine::set_num_renderpasses(const uint32_t num_renderpasses) {
        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_descriptor = {};
        rtv_heap_descriptor.NumDescriptors = num_renderpasses * 8;
        rtv_heap_descriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtv_heap_descriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        const HRESULT hr = device->CreateDescriptorHeap(&rtv_heap_descriptor, IID_PPV_ARGS(&rtv_descriptor_heap));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create descriptor heap for the RTV";
            throw render_engine_initialization_exception("Could not create descriptor head for the RTV");
        }
    }

    result<Renderpass*> DX12RenderEngine::create_renderpass(const shaderpack::RenderPassCreateInfo& data) {
        return result<Renderpass*>(new DX12Renderpass);
    }

    Framebuffer* DX12RenderEngine::create_framebuffer(const Renderpass* renderpass,
                                                      const std::vector<Image*>& attachments,
                                                      const glm::uvec2& framebuffer_size) {
        const size_t attachment_count = attachments.size();
        DX12Framebuffer* framebuffer = new DX12Framebuffer;
        framebuffer->render_targets.reserve(attachment_count);

        std::vector<ID3D12Resource*> rendertargets;
        rendertargets.reserve(attachment_count);

        for(uint32_t i = 0; i < attachments.size(); i++) {
            const Image* attachment = attachments.at(i);
            const DX12Image* d3d12_image = static_cast<const DX12Image*>(attachment);

            rendertargets.emplace_back(d3d12_image->resource);

            framebuffer->render_targets.emplace_back(framebuffer->descriptor_heap->GetCPUDescriptorHandleForHeapStart());

            // Create the Render Target View, which binds the swapchain buffer to the RTV handle
            device->CreateRenderTargetView(d3d12_image->resource, nullptr, framebuffer->render_targets.at(i));

            // Increment the RTV handle
            framebuffer->render_targets.at(i).Offset(1, rtv_descriptor_size);
        }

        framebuffer->size = framebuffer_size;

        return framebuffer;
    }

    Pipeline* DX12RenderEngine::create_pipeline(const Renderpass* renderpass, const shaderpack::PipelineCreateInfo& data) {
        return nullptr;
    }

    Buffer* DX12RenderEngine::create_buffer(const BufferCreateInfo& info) { return nullptr; }

    Image* DX12RenderEngine::create_texture(const shaderpack::TextureCreateInfo& info) {
        DX12Image* image = new DX12Image;

        const shaderpack::TextureFormat& format = info.format;

        glm::uvec2 dimensions;
        if(format.dimension_type == shaderpack::TextureDimensionTypeEnum::Absolute) {
            dimensions.x = static_cast<uint32_t>(format.width);
            dimensions.y = static_cast<uint32_t>(format.height);
        } else {
            swapchain->GetSourceSize(&dimensions.x, &dimensions.y);
            dimensions.x *= static_cast<uint32_t>(format.width);
            dimensions.y *= static_cast<uint32_t>(format.height);
        }

        const DXGI_FORMAT dx12_format = to_dxgi_format(format.pixel_format);

        DXGI_SAMPLE_DESC sample_desc = {};
        sample_desc.Count = 1;
        sample_desc.Quality = 1;

        D3D12_RESOURCE_DESC texture_desc = {};
        texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texture_desc.Alignment = 0;
        texture_desc.Width = dimensions.x;
        texture_desc.Height = dimensions.y;
        texture_desc.DepthOrArraySize = 1;
        texture_desc.MipLevels = 1;
        texture_desc.Format = dx12_format;
        texture_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texture_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        texture_desc.SampleDesc = sample_desc;

        if(format.pixel_format == shaderpack::PixelFormatEnum::Depth || format.pixel_format == shaderpack::PixelFormatEnum::DepthStencil) {
            texture_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        }

        // TODO: Info in shaderpack::texture_create_info_t about what heap to put the texture in
        ComPtr<ID3D12Resource> texture;
        auto heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        const HRESULT hr = device->CreateCommittedResource(&heap_props,
                                                           D3D12_HEAP_FLAG_NONE,
                                                           &texture_desc,
                                                           D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                           nullptr,
                                                           IID_PPV_ARGS(&texture));

        if(FAILED(hr)) {
            std::string error_description;
            switch(hr) {
                case E_OUTOFMEMORY:
                    error_description = "Out of memory";
                    break;

                case E_INVALIDARG:
                    error_description = "One or more arguments are invalid";
                    break;
            }

            NOVA_LOG(ERROR) << "Could not create texture " << info.name << ": Error code " << hr
                            << ", Error description: " << error_description << ", Windows error: '" << get_last_windows_error() << "'";

            return nullptr;
        }
        texture->SetName(s2ws(info.name).c_str());

        return image;
    }

    Semaphore* DX12RenderEngine::create_semaphore() { return nullptr; }

    std::vector<Semaphore*> DX12RenderEngine::create_semaphores(uint32_t num_semaphores) { return std::vector<Semaphore*>(); }

    Fence* DX12RenderEngine::create_fence(bool signaled) { return nullptr; }

    std::vector<Fence*> DX12RenderEngine::create_fences(uint32_t num_fences, bool signaled) { return std::vector<Fence*>(); }

    void DX12RenderEngine::destroy_renderpass(Renderpass* pass) { delete pass; }

    void DX12RenderEngine::destroy_framebuffer(const Framebuffer* framebuffer) {
        const DX12Framebuffer* d3d12_framebuffer = static_cast<const DX12Framebuffer*>(framebuffer);
        d3d12_framebuffer->descriptor_heap->Release();

        delete d3d12_framebuffer;
    }

    void DX12RenderEngine::destroy_pipeline(Pipeline* pipeline) {}

    void DX12RenderEngine::destroy_texture(Image* resource) {
        const DX12Image* d3d12_framebuffer = static_cast<const DX12Image*>(resource);
        d3d12_framebuffer->resource->Release();

        delete d3d12_framebuffer;
    }

    void DX12RenderEngine::destroy_semaphores(const std::vector<Semaphore*>& semaphores) {}

    void DX12RenderEngine::destroy_fences(const std::vector<Fence*>& fences) {}

    CommandList* DX12RenderEngine::allocate_command_list(uint32_t thread_idx, QueueType needed_queue_type, CommandList::Level level) {
        return nullptr;
    }

    void DX12RenderEngine::submit_command_list(CommandList* cmds,
                                               QueueType queue,
                                               Fence* fence_to_signal,
                                               const std::vector<Semaphore*>& wait_semaphores,
                                               const std::vector<Semaphore*>& signal_semaphores) {}

    void DX12RenderEngine::open_window_and_create_surface(const NovaSettings::WindowOptions& options) {}

    void DX12RenderEngine::create_device() {
        if(settings.debug.enabled && settings.debug.enable_validation_layers) {
            ComPtr<ID3D12Debug> debug_controller;
            D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
            debug_controller->EnableDebugLayer();
        }

        NOVA_LOG(TRACE) << "Creating DX12 device";

        CHECK_ERROR(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgi_factory)), "Could not create DXGI Factory");

        NOVA_LOG(TRACE) << "Device created";

        ComPtr<IDXGIAdapter1> adapter;

        uint32_t adapter_index = 0;
        bool adapter_found = false;

        while(dxgi_factory->EnumAdapters1(adapter_index, &adapter) != DXGI_ERROR_NOT_FOUND) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                // Ignore software devices
                adapter_index++;
                continue;
            }

            // Direct3D 12 is feature level 11.
            //
            // cool
            const HRESULT hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
            if(SUCCEEDED(hr)) {
                adapter_found = true;
                break;
            }

            adapter_index++;
        }

        if(!adapter_found) {
            NOVA_LOG(ERROR) << "Could not find a GPU that supports DX12";
        }
    }

    void DX12RenderEngine::create_queues() {
        D3D12_COMMAND_QUEUE_DESC rtv_queue_desc = {};
        rtv_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        CHECK_ERROR(device->CreateCommandQueue(&rtv_queue_desc, IID_PPV_ARGS(&direct_command_queue)),
                    "Could not create main command queue");

        D3D12_COMMAND_QUEUE_DESC compute_queue_desc = {};
        compute_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        CHECK_ERROR(device->CreateCommandQueue(&compute_queue_desc, IID_PPV_ARGS(&compute_command_queue)),
                    "Could not create asymc compute command queue");

        D3D12_COMMAND_QUEUE_DESC copy_queue_desc = {};
        copy_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        CHECK_ERROR(device->CreateCommandQueue(&copy_queue_desc, IID_PPV_ARGS(&copy_command_queue)), "Could not create copy command queue");
    }
} // namespace nova::renderer::rhi
