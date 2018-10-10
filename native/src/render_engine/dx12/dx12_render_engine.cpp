/*!
 * \author ddubois 
 * \date 30-Aug-18.
 */

#include <windows.h>

#include "dx12_render_engine.hpp"
#include "dx12_opaque_types.hpp"
#include <d3d12sdklayers.h>
#include <algorithm>
#include "../../util/logger.hpp"
#include "../../loading/shaderpack/render_graph_builder.hpp"

namespace nova {
    DXGI_FORMAT get_dx12_format_from_pixel_format(const pixel_format_enum pixel_format);


    dx12_render_engine::dx12_render_engine(const nova_settings &settings) : render_engine(settings),
            num_in_flight_frames(settings.get_options().max_in_flight_frames) {
        create_device();
        create_rtv_command_queue();
        create_full_frame_fences();

        std::vector<command_list_base*> direct_buffers;
        direct_buffers.reserve(32);    // Not sure how many we need, this should be enough
        buffer_pool.emplace(D3D12_COMMAND_LIST_TYPE_DIRECT, direct_buffers);

        std::vector<command_list_base*> copy_buffers;
        copy_buffers.reserve(32);
        buffer_pool.emplace(D3D12_COMMAND_LIST_TYPE_COPY, copy_buffers);

        std::vector<command_list_base*> compute_buffers;
        compute_buffers.reserve(32);
        buffer_pool.emplace(D3D12_COMMAND_LIST_TYPE_COMPUTE, compute_buffers);
    }

    void dx12_render_engine::open_window(uint32_t width, uint32_t height) {
        window = std::make_unique<win32_window>(width, height);
        create_swapchain();
        create_render_target_descriptor_heap();
    }

    const std::string dx12_render_engine::get_engine_name() {
        return "DirectX 12";
    }

    void dx12_render_engine::create_device() {
//#ifndef NDEBUG
        ComPtr<ID3D12Debug> debug_controller;
        D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
        debug_controller->EnableDebugLayer();
//#endif

        NOVA_LOG(TRACE) << "Creating DX12 device";

        HRESULT hr;

        hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgi_factory));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create DXGI Factory";
            throw render_engine_initialization_exception("Could not create DXGI Factory");
        }
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
            hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
            if(SUCCEEDED(hr)) {
                adapter_found = true;
                break;
            }

            adapter_index++;
        }

        if(!adapter_found) {
            NOVA_LOG(FATAL) << "Could not find a GPU that supports DX12";
            throw render_engine_initialization_exception("Could not find a GPU that supports DX12");
        }

        NOVA_LOG(TRACE) << "Adapter found";

        hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create Dx12 device";
            throw render_engine_initialization_exception("Could not create Dx12 device");
        }
    }

    void dx12_render_engine::create_rtv_command_queue() {
        D3D12_COMMAND_QUEUE_DESC rtv_queue_desc = {};

        HRESULT hr = device->CreateCommandQueue(&rtv_queue_desc, IID_PPV_ARGS(&direct_command_queue));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create main command queue";
            throw render_engine_initialization_exception("Could not create main command queue");
        }
    }

    void dx12_render_engine::create_swapchain() {
        if(!window) {
            NOVA_LOG(FATAL) << "Cannot initialize the swapchain before the window!";
            throw render_engine_initialization_exception("Cannot initialize the swapchain before the window");
        }

        const auto& window_size = window->get_window_size();

        DXGI_SAMPLE_DESC sample_desc = {};
        sample_desc.Count = 1;

        DXGI_SWAP_CHAIN_DESC1 swapchain_description {};
        swapchain_description.Width = window_size.width;
        swapchain_description.Height = window_size.height;
        swapchain_description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        swapchain_description.SampleDesc = sample_desc;
        swapchain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchain_description.BufferCount = FRAME_BUFFER_COUNT;

        swapchain_description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        ComPtr<IDXGISwapChain1> swapchain_uncast;
        // No target window specified in DXGI_SWAP_CHAIN_DESC, and no window associated with owning factory. [ MISCELLANEOUS ERROR #6: ]
        HRESULT hr = dxgi_factory->CreateSwapChainForHwnd(direct_command_queue.Get(), window->get_window_handle(), &swapchain_description, nullptr, nullptr, &swapchain_uncast);
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create swapchain";
            if(hr == DXGI_ERROR_INVALID_CALL) {
                NOVA_LOG(INFO) << "Invalid call - one or more of the parameters was wrong";
            } else if(hr == DXGI_STATUS_OCCLUDED) {
                NOVA_LOG(INFO) << "Fullscreen is unavaible";
            } else if(hr == E_OUTOFMEMORY) {
                NOVA_LOG(INFO) << "Out of memory. Soz bro :/";
            }

            throw render_engine_initialization_exception("Could not create swapchain");
        }

        swapchain_uncast->QueryInterface(IID_PPV_ARGS(&swapchain));
    }

    void dx12_render_engine::create_render_target_descriptor_heap() {
        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_descriptor = {};
        rtv_heap_descriptor.NumDescriptors = FRAME_BUFFER_COUNT;
        rtv_heap_descriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

        // The heap isn't visible to shaders because shaders don't need to read the swapchain... but we still need a
        // descriptor heap
        rtv_heap_descriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        HRESULT hr = device->CreateDescriptorHeap(&rtv_heap_descriptor, IID_PPV_ARGS(&rtv_descriptor_heap));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create descriptor heap for the RTV";
            throw render_engine_initialization_exception("Could not create descriptor head for the RTV");
        }

        rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

        for(uint32_t i = 0; i < FRAME_BUFFER_COUNT; i++) {
            ComPtr<ID3D12Resource> rendertarget;
            hr = swapchain->GetBuffer(i, IID_PPV_ARGS(&rendertarget));
            if(FAILED(hr)) {
                NOVA_LOG(FATAL) << "Could not create RTV for swapchain image " << i;
                throw render_engine_initialization_exception("Could not create RTV for swapchain");
            }

            rendertargets.push_back(rendertarget);

            // Create the Render Target View, which binds the swapchain buffer to the RTV handle
            device->CreateRenderTargetView(rendertargets[i].Get(), nullptr, rtv_handle);

            // Increment the RTV handle
            rtv_handle.Offset(1, rtv_descriptor_size);
        }
    }

    std::shared_ptr<iwindow> dx12_render_engine::get_window() const {
        return window;
    }

    void dx12_render_engine::render_frame() {
        wait_for_previous_frame();

        try_to_free_command_lists();

        gfx_command_list* present_commands = get_graphics_command_list();
        HRESULT hr = present_commands->allocator->Reset();
        if(FAILED(hr)) {
            NOVA_LOG(WARN) << "Could not reset command list allocator, memory usage will likely increase dramatically";
        }

        present_commands->list->Reset(present_commands->allocator.Get(), nullptr);

        CD3DX12_RESOURCE_BARRIER to_render_target = CD3DX12_RESOURCE_BARRIER::Transition(rendertargets[frame_index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        present_commands->list->ResourceBarrier(1, &to_render_target);

        CD3DX12_CPU_DESCRIPTOR_HANDLE current_framebuffer_rtv(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), frame_index, rtv_descriptor_size);
        present_commands->list->OMSetRenderTargets(1, &current_framebuffer_rtv, false, nullptr);

        glm::vec4 clear_color(0, 0.2f, 0.4f, 1.0f);
        present_commands->list->ClearRenderTargetView(current_framebuffer_rtv, &clear_color.x, 0, nullptr);

        CD3DX12_RESOURCE_BARRIER to_presentable = CD3DX12_RESOURCE_BARRIER::Transition(rendertargets[frame_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        present_commands->list->ResourceBarrier(1, &to_presentable);

        present_commands->list->Close();

        ID3D12CommandList* commands[] = {present_commands->list.Get()};
        direct_command_queue->ExecuteCommandLists(1, commands);
        direct_command_queue->Signal(present_commands->submission_fence.Get(), present_commands->fence_value);

        direct_command_queue->Signal(frame_fences.at(frame_index).Get(), frame_fence_values.at(frame_index));

        swapchain->Present(0, 0);

        release_command_list(present_commands);

    }

    void dx12_render_engine::wait_for_previous_frame() {// Wait for the previous frame at our index to finish
        frame_index = swapchain->GetCurrentBackBufferIndex();
        if(frame_fences.at(frame_index)->GetCompletedValue() < frame_fence_values.at(frame_index)) {
            frame_fences.at(frame_index)->SetEventOnCompletion(frame_fence_values.at(frame_index),
                                                               full_frame_fence_event);
            WaitForSingleObject(full_frame_fence_event, INFINITE);
        }
        // There is enough space in a 32-bit integer to run Nova at 60 fps for almost three years. Running Nova
        // continuously for more than one year is not supported
        frame_fence_values[frame_index]++;
    }

    command_list* dx12_render_engine::allocate_command_list(D3D12_COMMAND_LIST_TYPE command_list_type) const {
        ComPtr<ID3D12CommandAllocator> allocator;
        HRESULT hr = device->CreateCommandAllocator(command_list_type, IID_PPV_ARGS(&allocator));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create command buffer";
            throw std::runtime_error("Could not create command buffer");
        }

        ComPtr<ID3D12CommandList> list;
        hr = device->CreateCommandList(0, command_list_type, allocator.Get(), nullptr, IID_PPV_ARGS(&list));
        if(FAILED(hr)) {
            NOVA_LOG(ERROR) << "Could not create a command list of type " << (uint32_t)command_list_type;
            throw std::runtime_error("Could not create command list");
        }

        ComPtr<ID3D12Fence> fence;
        device->CreateFence(1, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

        auto* cmd_list = new command_list;
        cmd_list->list = list;
        cmd_list->allocator = allocator;
        cmd_list->submission_fence = fence;
        cmd_list->type = command_list_type;

        return cmd_list;
    }

    gfx_command_list* dx12_render_engine::get_graphics_command_list() {
        std::lock_guard<std::mutex> lock(buffer_pool_mutex);
        gfx_command_list* new_list;
        auto& buffers = buffer_pool.at(D3D12_COMMAND_LIST_TYPE_DIRECT);
        if(buffers.empty()) {
            command_list* alloc_list = allocate_command_list(D3D12_COMMAND_LIST_TYPE_DIRECT);

            ComPtr<ID3D12GraphicsCommandList> graphics_list;
            alloc_list->list->QueryInterface(IID_PPV_ARGS(&graphics_list));
            graphics_list->Close();

            new_list = new gfx_command_list;
            new_list->list = graphics_list;
            new_list->type = alloc_list->type;
            new_list->allocator = alloc_list->allocator;
            new_list->submission_fence = alloc_list->submission_fence;
            new_list->fence_value = alloc_list->fence_value;

        } else {
            command_list_base* pool_list = buffers.back();
            pool_list->is_done = false;
            buffers.pop_back();

            // CLion complains about the static cast but dynasmic_cast doesn't work since gfx_command_list doesn't have
            // a vtable
            new_list = static_cast<gfx_command_list*>(pool_list);
        }

        return new_list;
    }

    void dx12_render_engine::release_command_list(command_list_base *list) {
        std::lock_guard<std::mutex> lock(lists_to_free_mutex);
        lists_to_free.push_back(list);
    }

    void dx12_render_engine::create_full_frame_fences() {
        frame_fence_values.resize(num_in_flight_frames);

        frame_fences.resize(num_in_flight_frames);

        HRESULT hr;

        for(uint32_t i = 0; i < num_in_flight_frames; i++) {
            ComPtr<ID3D12Fence> fence;
            hr = device->CreateFence(1, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
            if(SUCCEEDED(hr)) {
                frame_fences[i] = fence;
            } else {
                NOVA_LOG(FATAL) << "Could not create fence for from index " << frame_index;
                throw render_engine_initialization_exception("Could not create fence for from index " + std::to_string(frame_index));
            }
        }
        NOVA_LOG(TRACE) << "Created full-frame fences";

        full_frame_fence_event = CreateEvent(nullptr, false, false, nullptr);
    }

    void dx12_render_engine::set_shaderpack(shaderpack_data data) {
        // Let's build our data from the ground up!
        // To load a new shaderpack, we need to first clear out all the data from the old shaderpack. Then, we can 
        // make the new dynamic textures and samplers, then the PSOs, then the material definitions, then the 
        // renderpasses
        // Except DX12 doesn't have a renderpass - but that's fine. We can put the necessary data together and pretend

        NOVA_LOG(DEBUG) << "DX12 render engine loading a new shaderpack";

        // Clear out the old shaderpack's data, including samplers
        dynamic_textures.clear();

        NOVA_LOG(DEBUG) << "Cleared data from old shaderpack";

        // Build up E V E R Y T H I N G
        std::vector<render_pass_data> ordered_passes = flatten_frame_graph(data.passes);

        NOVA_LOG(DEBUG) << "Flattened frame graph";

        create_gpu_query_heap(ordered_passes.size());

        create_dynamic_textures(data.resources.textures, ordered_passes);
    }

    std::vector<render_pass_data> dx12_render_engine::flatten_frame_graph(const std::vector<render_pass_data>& passes) {
        std::unordered_map<std::string, render_pass_data> passes_by_name;
        passes_by_name.reserve(passes.size());
        for(const render_pass_data& pass_data : passes) {
            passes_by_name[pass_data.name] = pass_data;
        }

        std::vector<render_pass_data> ordered_passes;

        std::vector<std::string> ordered_pass_names = order_passes(passes_by_name);
        ordered_passes.reserve(ordered_pass_names.size());
        for(const std::string& pass_name : ordered_pass_names) {
            ordered_passes.push_back(passes_by_name.at(pass_name));
        }

        return ordered_passes;
    }

    void dx12_render_engine::try_to_free_command_lists() {
        std::lock_guard<std::mutex> lock(lists_to_free_mutex);

        for(command_list_base* list : lists_to_free) {
            if(list->submission_fence->GetCompletedValue() == list->fence_value) {
                // Command list is done!
                std::lock_guard<std::mutex> pool_lock(buffer_pool_mutex);
                list->is_done = true;
                buffer_pool.at(list->type).push_back(list);
            }
        }

        const auto erase_itr = std::remove_if(lists_to_free.begin(), lists_to_free.end(), [](const command_list_base* list) {return list->is_done;});
        lists_to_free.erase(erase_itr, lists_to_free.end());
    }

    void dx12_render_engine::create_dynamic_textures(const std::vector<texture_resource_data>& texture_datas, std::vector<render_pass_data> passes) {
        std::unordered_map<std::string, texture_resource_data> textures;
        for(const texture_resource_data& data : texture_datas) {
            textures[data.name] = data;
        }

        struct range {
            uint32_t first_write_pass = ~0u;
            uint32_t last_write_pass = 0;
            uint32_t first_read_pass = ~0u;
            uint32_t last_read_pass = 0;

            bool has_writer() const {
                return first_write_pass <= last_write_pass;
            }

            bool has_reader() const {
                return first_read_pass <= last_read_pass;
            }

            bool is_used() const {
                return has_writer() || has_reader();
            }

            bool can_alias() const {
                // If we read before we have completely written to a resource we need to preserve it, so no alias is possible.
                return !(has_reader() && has_writer() && first_read_pass <= first_write_pass);
            }

            unsigned last_used_pass() const {
                unsigned last_pass = 0;
                if(has_writer())
                    last_pass = std::max(last_pass, last_write_pass);
                if(has_reader())
                    last_pass = std::max(last_pass, last_read_pass);
                return last_pass;
            }

            unsigned first_used_pass() const {
                unsigned first_pass = ~0u;
                if(has_writer())
                    first_pass = std::min(first_pass, first_write_pass);
                if(has_reader())
                    first_pass = std::min(first_pass, first_read_pass);
                return first_pass;
            }

            bool is_disjoint_with(const range& other) const {
                if(!is_used() || !other.is_used())
                    return false;
                if(!can_alias() || !other.can_alias())
                    return false;

                bool left = last_used_pass() < other.first_used_pass();
                bool right = other.last_used_pass() < first_used_pass();
                return left || right;
            }
        };

        // Look at what range of render passes each resource is used in
        std::unordered_map<std::string, range> resource_used_range;
        std::vector<std::string> resources_in_order;

        uint32_t pass_idx = 0;
        for(const auto& pass : passes) {
            if(pass.texture_inputs) {
                const input_textures& all_inputs = pass.texture_inputs.value();
                // color attachments
                for(const auto &input : all_inputs.color_attachments) {
                    auto& tex_range = resource_used_range[input];

                    if(pass_idx < tex_range.first_write_pass) {
                        tex_range.first_write_pass = pass_idx;

                    } else if(pass_idx > tex_range.last_write_pass) {
                        tex_range.last_write_pass = pass_idx;
                    }

                    if(std::find(resources_in_order.begin(), resources_in_order.end(), input) == resources_in_order.end()) {
                        resources_in_order.push_back(input);
                    }
                }

                // shader-only textures
                for(const auto &input : all_inputs.bound_textures) {
                    auto& tex_range = resource_used_range[input];

                    if(pass_idx < tex_range.first_write_pass) {
                        tex_range.first_write_pass = pass_idx;

                    } else if(pass_idx > tex_range.last_write_pass) {
                        tex_range.last_write_pass = pass_idx;
                    }

                    if(std::find(resources_in_order.begin(), resources_in_order.end(), input) == resources_in_order.end()) {
                        resources_in_order.push_back(input);
                    }
                }
            }

            if(!pass.texture_outputs.empty()) {
                for(const auto &output : pass.texture_outputs) {
                    auto& tex_range = resource_used_range[output.name];

                    if(pass_idx < tex_range.first_write_pass) {
                        tex_range.first_write_pass = pass_idx;

                    } else if(pass_idx > tex_range.last_write_pass) {
                        tex_range.last_write_pass = pass_idx;
                    }

                    if(std::find(resources_in_order.begin(), resources_in_order.end(), output.name) == resources_in_order.end()) {
                        resources_in_order.push_back(output.name);
                    }
                }
            }

            pass_idx++;
        }

        NOVA_LOG(TRACE) << "Ordered resources";

        // Figure out which resources can be aliased
        std::unordered_map<std::string, std::string> aliases;

        for(size_t i = 0; i < resources_in_order.size(); i++) {
            const auto& to_alias_name = resources_in_order[i];
            NOVA_LOG(TRACE) << "Determining if we can alias `" << to_alias_name << "`. Does it exist? " << (textures.find(to_alias_name) != textures.end());
            if(to_alias_name == "Backbuffer" || to_alias_name == "backbuffer") {
                // Yay special cases!
                continue;
            }

            const auto& to_alias_format = textures.at(to_alias_name).format;

            // Only try to alias with lower-indexed resources
            for(size_t j = 0; j < i; j++) {
                NOVA_LOG(TRACE) << "Trying to alias it with resource at index " << j << " out of " << resources_in_order.size();
                const auto& try_alias_name = resources_in_order[j];
                if(resource_used_range[to_alias_name].is_disjoint_with(resource_used_range[try_alias_name])) {
                    // They can be aliased if they have the same format
                    const auto& try_alias_format = textures.at(try_alias_name).format;
                    if(to_alias_format == try_alias_format) {
                        aliases[to_alias_name] = try_alias_name;
                    }
                }
            }
        }

        NOVA_LOG(TRACE) << "Figured out which resources can be aliased";

        glm::uvec2 swapchain_dimensions;
        swapchain->GetSourceSize(&swapchain_dimensions.x, &swapchain_dimensions.y);

        // For each texture:
        //  - If it isn't in the aliases map, create a new texture with its format and add it to the textures map
        //  - If it is in the aliases map, follow its chain of aliases

        for(const auto& named_texture : textures) {
            std::string texture_name = named_texture.first;
            while(aliases.find(texture_name) != aliases.end()) {
                NOVA_LOG(TRACE) << "Resource " << texture_name << " is aliased with " << aliases[texture_name];
                texture_name = aliases[texture_name];
            }

            // We've found the first texture in this alias chain - let's create an actual texture for it if needed
            if(dynamic_tex_name_to_idx.find(texture_name) == dynamic_tex_name_to_idx.end()) {
                NOVA_LOG(TRACE) << "Need to create it";
                // The texture we're all aliasing doesn't have a real texture yet. Let's fix that
                const texture_format& format = textures.at(texture_name).format;

                glm::uvec2 dimensions;
                if(format.dimension_type == texture_dimension_type_enum::Absolute) {
                    dimensions.x = static_cast<uint32_t>(format.width);
                    dimensions.y = static_cast<uint32_t>(format.height);

                } else {
                    dimensions = swapchain_dimensions;
                    dimensions.x *= format.width;
                    dimensions.y *= format.height;
                }

                DXGI_FORMAT dx12_format = get_dx12_format_from_pixel_format(format.pixel_format);

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

                if(format.pixel_format == pixel_format_enum::Depth || format.pixel_format == pixel_format_enum::DepthStencil) {
                    texture_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                }

                auto pixel_format = get_vk_format_from_pixel_format(format.pixel_format);
                NOVA_LOG(DEBUG) << "Creating a texture with nova format " << pixel_format_enum::to_string(format.pixel_format) << " and Vulkan format " << vk::to_string(pixel_format) << ". It's being created for texture " << texture_name;
                auto tex = texture2D(std::to_string(dynamic_textures.size()), dimensions, pixel_format, usage, context);

                auto new_tex_index = dynamic_textures.size();
                dynamic_textures.emplace_back(std::move(tex));
                dynamic_tex_name_to_idx.emplace(texture_name, new_tex_index);

                NOVA_LOG(TRACE) << "Added texture " << tex.get_name() << " to the dynamic textures";
                NOVA_LOG(TRACE) << "set dynamic_texture_to_idx[" << texture_name << "] = " << new_tex_index;

            } else {
                NOVA_LOG(TRACE) << "The physical resource already exists, so we're just gonna use that";
                // The texture we're aliasing already has a real texture behind it - so let's use that
                dynamic_tex_name_to_idx[named_texture.first] = dynamic_tex_name_to_idx[texture_name];
            }
        }
    }

    void dx12_render_engine::create_gpu_query_heap(size_t num_queries) {
        D3D12_QUERY_HEAP_DESC heap_desc = {};
        heap_desc.Count = num_queries;
        heap_desc.NodeMask = 0;
        heap_desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;

        device->CreateQueryHeap(&heap_desc, IID_PPV_ARGS(&renderpass_timestamp_query_heap));
    }

    DXGI_FORMAT get_dx12_format_from_pixel_format(const pixel_format_enum pixel_format) {
        switch(pixel_format) {
            case pixel_format_enum::RGB8: 
            case pixel_format_enum::RGBA8:
                return DXGI_FORMAT_R8G8B8A8_SNORM;

            case pixel_format_enum::RGB16F:
            case pixel_format_enum::RGBA16F:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;

            case pixel_format_enum::RGB32F:
            case pixel_format_enum::RGBA32F: 
                return DXGI_FORMAT_R32G32B32A32_FLOAT;

            case pixel_format_enum::Depth: 
                return DXGI_FORMAT_D32_FLOAT;

            case pixel_format_enum::DepthStencil: 
                return DXGI_FORMAT_D24_UNORM_S8_UINT;
        }
    }
}
