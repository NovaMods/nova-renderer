/*!
 * \author ddubois
 * \date 03-Apr-19.
 */

#include <d3d12sdklayers.h>

#include "../../util/logger.hpp"
#include "../../util/windows_utils.hpp"
#include "d3d12_render_engine.hpp"
#include "d3d12_structs.hpp"
#include "d3dx12.h"
#include "dx12_utils.hpp"

using Microsoft::WRL::ComPtr;

namespace nova::renderer::rhi {
    d3d12_render_engine::d3d12_render_engine(nova_settings& settings) : render_engine_t(settings) {
        create_device();

        open_window_and_create_surface(settings.window);

        create_queues();

        create_rtv_descriptor_heap();
    }

    std::shared_ptr<window_t> d3d12_render_engine::get_window() const { return window; }

    result<renderpass_t*> d3d12_render_engine::create_renderpass(const shaderpack::render_pass_create_info_t& data) {
        return result<renderpass_t*>(new d3d12_renderpass_t);
    }

    framebuffer_t* d3d12_render_engine::create_framebuffer(const renderpass_t* renderpass,
                                                           const std::vector<image_t*>& attachments,
                                                           const glm::uvec2& framebuffer_size) {
        const size_t attachment_count = attachments.size();
        d3d12_framebuffer_t* framebuffer = new d3d12_framebuffer_t;
        framebuffer->render_targets.reserve(attachment_count);
        
        const uint32_t rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        
        std::vector<ID3D12Resource*> rendertargets;
        rendertargets.reserve(attachment_count);

        for(uint32_t i = 0; i < attachments.size(); i++) {
            const image_t* attachment = attachments.at(i);
            const d3d12_image_t* d3d12_image = static_cast<const d3d12_image_t*>(attachment);

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

    pipeline_t* d3d12_render_engine::create_pipeline(const shaderpack::pipeline_create_info_t& data) { return nullptr; }

    buffer_t* d3d12_render_engine::create_buffer(const buffer_create_info_t& info) { return nullptr; }

    image_t* d3d12_render_engine::create_texture(const shaderpack::texture_create_info_t& info) {
        d3d12_image_t* image = new d3d12_image_t;

        const shaderpack::texture_format& format = info.format;

        glm::uvec2 dimensions;
        if(format.dimension_type == shaderpack::texture_dimension_type_enum::Absolute) {
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

        if(format.pixel_format == shaderpack::pixel_format_enum::Depth ||
           format.pixel_format == shaderpack::pixel_format_enum::DepthStencil) {
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

            NOVA_LOG(ERROR) << "Could not create texture " << info.name << ": Error code " << hr << ", Error description '"
                            << error_description << ", Windows error: '" << get_last_windows_error() << "'";

            return nullptr;
        }
        texture->SetName(s2ws(info.name).c_str());

        return image;
    }

    semaphore_t* d3d12_render_engine::create_semaphore() { return nullptr; }

    std::vector<semaphore_t*> d3d12_render_engine::create_semaphores(uint32_t num_semaphores) { return std::vector<semaphore_t*>(); }

    fence_t* d3d12_render_engine::create_fence(bool signaled) { return nullptr; }

    std::vector<fence_t*> d3d12_render_engine::create_fences(uint32_t num_fences, bool signaled) { return std::vector<fence_t*>(); }

    void d3d12_render_engine::destroy_renderpass(renderpass_t* pass) { delete pass; }

    void d3d12_render_engine::destroy_framebuffer(const framebuffer_t* framebuffer) {
        const d3d12_framebuffer_t* d3d12_framebuffer = static_cast<const d3d12_framebuffer_t*>(framebuffer);
        d3d12_framebuffer->descriptor_heap->Release();

        delete d3d12_framebuffer;
    }

    void d3d12_render_engine::destroy_pipeline(pipeline_t* pipeline) {}

    void d3d12_render_engine::destroy_texture(image_t* resource) {
        const d3d12_image_t* d3d12_framebuffer = static_cast<const d3d12_image_t*>(resource);
        d3d12_framebuffer->resource->Release();

        delete d3d12_framebuffer;
    }

    void d3d12_render_engine::destroy_semaphores(const std::vector<semaphore_t*>& semaphores) {}

    void d3d12_render_engine::destroy_fences(const std::vector<fence_t*>& fences) {}

    command_list_t* d3d12_render_engine::allocate_command_list(uint32_t thread_idx,
                                                               queue_type needed_queue_type,
                                                               command_list_t::level level) {
        return nullptr;
    }

    void d3d12_render_engine::submit_command_list(command_list_t* cmds,
                                                  queue_type queue,
                                                  fence_t* fence_to_signal,
                                                  const std::vector<semaphore_t*>& wait_semaphores,
                                                  const std::vector<semaphore_t*>& signal_semaphores) {}

    void d3d12_render_engine::open_window_and_create_surface(const nova_settings::window_options& options) {}

    void d3d12_render_engine::create_device() {
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

    void d3d12_render_engine::create_queues() {
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
