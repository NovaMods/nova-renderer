/*!
 * \author ddubois
 * \date 30-Aug-18.
 */

#include <windows.h>
#include <D3DCompiler.h>

#include "dx12_render_engine.hpp"
#include <d3d12sdklayers.h>
#include <algorithm>
#include <unordered_set>

#include "../../util/logger.hpp"
#include "../../loading/shaderpack/render_graph_builder.hpp"
#include "../../util/windows_utils.hpp"
#include "../../../3rdparty/SPIRV-Cross/spirv_cross.hpp"
#include "../../loading/shaderpack/shaderpack_loading.hpp"
#include "vertex_attributes.hpp"
#include "d3dx12.h"
#include "../../nova_renderer.hpp"
#include "dx12_utils.hpp"

#include "../../tasks/task_scheduler.hpp"

namespace nova {
    dx12_render_engine::dx12_render_engine(const nova_settings& settings, ttl::task_scheduler* scheduler) : render_engine(settings, scheduler), num_in_flight_frames(settings.get_options().max_in_flight_frames) {
        NOVA_LOG(INFO) << "Initializing Direct3D 12 rendering";

        create_device();
        create_rtv_command_queue();
        create_full_frame_fences();

        std::vector<command_list_base *> direct_buffers;
        direct_buffers.reserve(32);  // Not sure how many we need, this should be enough
        buffer_pool.emplace(D3D12_COMMAND_LIST_TYPE_DIRECT, direct_buffers);

        std::vector<command_list_base *> copy_buffers;
        copy_buffers.reserve(32);
        buffer_pool.emplace(D3D12_COMMAND_LIST_TYPE_COPY, copy_buffers);

        std::vector<command_list_base *> compute_buffers;
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

        HRESULT hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgi_factory));
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

        const auto &window_size = window->get_window_size();

        DXGI_SAMPLE_DESC sample_desc = {};
        sample_desc.Count = 1;

        DXGI_SWAP_CHAIN_DESC1 swapchain_description{};
        swapchain_description.Width = window_size.x;
        swapchain_description.Height = window_size.y;
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

        gfx_command_list *present_commands = get_graphics_command_list();
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

        ID3D12CommandList *commands[] = {present_commands->list.Get()};
        direct_command_queue->ExecuteCommandLists(1, commands);
        direct_command_queue->Signal(present_commands->submission_fence.Get(), present_commands->fence_value);

        direct_command_queue->Signal(frame_fences.at(frame_index).Get(), frame_fence_values.at(frame_index));

        swapchain->Present(0, 0);

        release_command_list(present_commands);
    }

    void dx12_render_engine::wait_for_previous_frame() {  // Wait for the previous frame at our index to finish
        frame_index = swapchain->GetCurrentBackBufferIndex();
        if(frame_fences.at(frame_index)->GetCompletedValue() < frame_fence_values.at(frame_index)) {
            frame_fences.at(frame_index)->SetEventOnCompletion(frame_fence_values.at(frame_index), full_frame_fence_event);
            WaitForSingleObject(full_frame_fence_event, INFINITE);
        }
        // There is enough space in a 32-bit integer to run Nova at 60 fps for almost three years. Running Nova
        // continuously for more than one year is not supported
        frame_fence_values[frame_index]++;
    }

    command_list *dx12_render_engine::allocate_command_list(D3D12_COMMAND_LIST_TYPE command_list_type) const {
        ComPtr<ID3D12CommandAllocator> allocator;
        HRESULT hr = device->CreateCommandAllocator(command_list_type, IID_PPV_ARGS(&allocator));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create command buffer";
            throw render_engine_initialization_exception("Could not create command buffer");
        }

        ComPtr<ID3D12CommandList> list;
        hr = device->CreateCommandList(0, command_list_type, allocator.Get(), nullptr, IID_PPV_ARGS(&list));
        if(FAILED(hr)) {
            NOVA_LOG(ERROR) << "Could not create a command list of type " << (uint32_t) command_list_type;
            throw render_engine_initialization_exception("Could not create command list");
        }

        ComPtr<ID3D12Fence> fence;
        device->CreateFence(1, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

        auto *cmd_list = new command_list;
        cmd_list->list = list;
        cmd_list->allocator = allocator;
        cmd_list->submission_fence = fence;
        cmd_list->type = command_list_type;

        return cmd_list;
    }

    gfx_command_list *dx12_render_engine::get_graphics_command_list() {
        std::lock_guard<std::mutex> lock(buffer_pool_mutex);
        gfx_command_list *new_list;
        auto &buffers = buffer_pool.at(D3D12_COMMAND_LIST_TYPE_DIRECT);
        if(buffers.empty()) {
            command_list *alloc_list = allocate_command_list(D3D12_COMMAND_LIST_TYPE_DIRECT);

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
            command_list_base *pool_list = buffers.back();
            pool_list->is_done = false;
            buffers.pop_back();

            // CLion complains about the static cast but dynasmic_cast doesn't work since gfx_command_list doesn't have
            // a vtable
            new_list = static_cast<gfx_command_list *>(pool_list);
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

    void dx12_render_engine::set_shaderpack(const shaderpack_data& data) {
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
        render_passes.clear();
        render_passes.reserve(data.passes.size());
        for(const render_pass_data &pass_data : data.passes) {
            render_passes[pass_data.name] = pass_data;
        }
        ordered_passes = order_passes(render_passes);

        NOVA_LOG(DEBUG) << "Flattened frame graph";

        create_gpu_query_heap(ordered_passes.size());

        std::vector<render_pass_data> passes_in_submission_order;
        passes_in_submission_order.reserve(ordered_passes.size());
        for(const std::string& pass_name : ordered_passes) {
            passes_in_submission_order.push_back(render_passes.at(pass_name));
        }
        create_dynamic_textures(data.resources.textures, passes_in_submission_order);

        make_pipeline_state_objects(data.pipelines, scheduler);
    }

    std::future<uint32_t> dx12_render_engine::add_mesh(const mesh_data&) {
        // TODO

        return {};
    }

    void dx12_render_engine::delete_mesh(uint32_t) {
        // TODO
    }

    void dx12_render_engine::try_to_free_command_lists() {
        std::lock_guard<std::mutex> lock(lists_to_free_mutex);

        for(command_list_base *list : lists_to_free) {
            if(list->submission_fence->GetCompletedValue() == list->fence_value) {
                // Command list is done!
                std::lock_guard<std::mutex> pool_lock(buffer_pool_mutex);
                list->is_done = true;
                buffer_pool.at(list->type).push_back(list);
            }
        }

        const auto erase_itr = std::remove_if(lists_to_free.begin(), lists_to_free.end(), [](const command_list_base *list) { return list->is_done; });
        lists_to_free.erase(erase_itr, lists_to_free.end());
    }

    void dx12_render_engine::create_dynamic_textures(const std::vector<texture_resource_data> &texture_datas, std::vector<render_pass_data> passes) {
        std::unordered_map<std::string, texture_resource_data> textures;
        for(const texture_resource_data &data : texture_datas) {
            textures[data.name] = data;
        }

        std::unordered_map<std::string, range> resource_used_range;
        std::vector<std::string> resources_in_order;
        determine_usage_order_of_textures(passes, resource_used_range, resources_in_order);

        NOVA_LOG(TRACE) << "Ordered resources";

        const std::unordered_map<std::string, std::string> aliases = determine_aliasing_of_textures(textures, resource_used_range, resources_in_order);
        NOVA_LOG(TRACE) << "Figured out which resources can be aliased";

        glm::uvec2 swapchain_dimensions;
        swapchain->GetSourceSize(&swapchain_dimensions.x, &swapchain_dimensions.y);

        // For each texture:
        //  - If it isn't in the aliases map, create a new texture with its format and add it to the textures map
        //  - If it is in the aliases map, follow its chain of aliases

        for(const auto &named_texture : textures) {
            std::string texture_name = named_texture.first;
            while(aliases.find(texture_name) != aliases.end()) {
                NOVA_LOG(TRACE) << "Resource " << texture_name << " is aliased with " << aliases.at(texture_name);
                texture_name = aliases.at(texture_name);
            }

            // We've found the first texture in this alias chain - let's create an actual texture for it if needed
            if(dynamic_tex_name_to_idx.find(texture_name) == dynamic_tex_name_to_idx.end()) {
                NOVA_LOG(TRACE) << "Need to create it";
                // The texture we're all aliasing doesn't have a real texture yet. Let's fix that
                const texture_format &format = textures.at(texture_name).format;

                glm::uvec2 dimensions;
                if(format.dimension_type == texture_dimension_type_enum::Absolute) {
                    dimensions.x = static_cast<uint32_t>(format.width);
                    dimensions.y = static_cast<uint32_t>(format.height);

                } else {
                    dimensions = swapchain_dimensions;
                    dimensions.x *= format.width;
                    dimensions.y *= format.height;
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

                if(format.pixel_format == pixel_format_enum::Depth || format.pixel_format == pixel_format_enum::DepthStencil) {
                    texture_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                }

                ComPtr<ID3D12Resource> texture;
                auto heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
                const HRESULT hr = device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &texture_desc, 
                    D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(&texture));

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

                    NOVA_LOG(ERROR) << "Could not create texture " << texture_name << ": Error code " << hr << ", Error description '" << error_description << ", Windows error: '" << get_last_windows_error() << "'";

                    continue;
                }
                texture->SetName(s2ws(texture_name).c_str());

                auto new_tex_index = dynamic_textures.size();
                dynamic_textures[texture_name] = dx12_texture(textures.at(texture_name), texture);
                dynamic_tex_name_to_idx.emplace(texture_name, new_tex_index);

                NOVA_LOG(TRACE) << "Added texture " << texture_name << " to the dynamic textures";
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

    void dx12_render_engine::make_pipeline_state_objects(const std::vector<pipeline_data>& pipelines, ttl::task_scheduler* scheduler) {
        std::vector<std::future<pipeline>> future_pipelines(pipelines.size());
        std::size_t write_pipeline = 0;

        for(const pipeline_data& data : pipelines) {
            if(!data.name.empty()) {
                future_pipelines[write_pipeline] = scheduler->add_task([&](ttl::task_scheduler* task_scheduler, const pipeline_data data) {
                    try {
                        return make_single_pso(data);
                    } catch(shader_compilation_failed& err) {
                        NOVA_LOG(ERROR) << "Could not compile shaders for PSO " << data.name << ": " << err.what();
                        return pipeline{};
                    }
                }, data);
                write_pipeline++;
            }
        }

        for(auto& future_pipeline : future_pipelines) {
            future_pipeline.wait();
            // TODO
        }
    }
    
    pipeline dx12_render_engine::make_single_pso(const pipeline_data& input) {
        const render_pass_data& render_pass = render_passes.at(input.pass);
        const auto states_begin = input.states.begin();
        const auto states_end = input.states.end();
        pipeline output;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc = {};

        /*
         * Compile all shader stages
         */

        std::unordered_map<uint32_t, std::vector<D3D12_DESCRIPTOR_RANGE1>> shader_inputs;
        spirv_cross::CompilerHLSL::Options options = {};
        options.shader_model = 51;

        ComPtr<ID3DBlob> vertex_blob = compile_shader(input.vertex_shader, "vs_5_1", options, shader_inputs);
        pipeline_state_desc.VS.BytecodeLength = vertex_blob->GetBufferSize();
        pipeline_state_desc.VS.pShaderBytecode = vertex_blob->GetBufferPointer();

        if(input.geometry_shader) {
            ComPtr<ID3DBlob> geometry_blob = compile_shader(*input.geometry_shader, "gs_5_1", options, shader_inputs);
            pipeline_state_desc.GS.BytecodeLength = geometry_blob->GetBufferSize();
            pipeline_state_desc.GS.pShaderBytecode = geometry_blob->GetBufferPointer();
        }

        if(input.tessellation_control_shader) {
            ComPtr<ID3DBlob> tessellation_control_blob = compile_shader(*input.tessellation_control_shader, "hs_5_1", options, shader_inputs);
            pipeline_state_desc.HS.BytecodeLength = tessellation_control_blob->GetBufferSize();
            pipeline_state_desc.HS.pShaderBytecode = tessellation_control_blob->GetBufferPointer();
        }
        if(input.tessellation_evaluation_shader) {
            ComPtr<ID3DBlob> tessellation_evaluation_blob = compile_shader(*input.tessellation_evaluation_shader, "ds_5_1", options, shader_inputs);
            pipeline_state_desc.DS.BytecodeLength = tessellation_evaluation_blob->GetBufferSize();
            pipeline_state_desc.DS.pShaderBytecode = tessellation_evaluation_blob->GetBufferPointer();
        }

        if(input.fragment_shader) {
            ComPtr<ID3DBlob> fragment_blob = compile_shader(*input.fragment_shader, "ps_5_1", options, shader_inputs);
            pipeline_state_desc.PS.BytecodeLength = fragment_blob->GetBufferSize();
            pipeline_state_desc.PS.pShaderBytecode = fragment_blob->GetBufferPointer();
        }
        
        output.root_signature = create_root_signature(shader_inputs);
        pipeline_state_desc.pRootSignature = output.root_signature.Get();

        /*
        * Blend state
        */

        pipeline_state_desc.BlendState.AlphaToCoverageEnable = std::find(states_begin, states_end, state_enum::EnableAlphaToCoverage) != states_end;
        pipeline_state_desc.BlendState.IndependentBlendEnable = false;
        D3D12_RENDER_TARGET_BLEND_DESC& blend_state = pipeline_state_desc.BlendState.RenderTarget[0];
        blend_state.BlendEnable = std::find(states_begin, states_end, state_enum::Blending) != states_end;
        blend_state.SrcBlend = to_dx12_blend(input.source_blend_factor);
        blend_state.DestBlend = to_dx12_blend(input.destination_blend_factor);
        blend_state.BlendOp = D3D12_BLEND_OP_ADD;
        blend_state.SrcBlendAlpha = to_dx12_blend(input.source_blend_factor);
        blend_state.DestBlendAlpha = to_dx12_blend(input.destination_blend_factor);
        blend_state.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blend_state.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        pipeline_state_desc.SampleMask = 0xFFFFFFFF;

        /*
         * Rasterizer state
         */

        D3D12_RASTERIZER_DESC& raster_desc = pipeline_state_desc.RasterizerState;
        raster_desc.FillMode = D3D12_FILL_MODE_SOLID;
        if(std::find(states_begin, states_end, state_enum::InvertCulling) != states_end) {
            raster_desc.CullMode = D3D12_CULL_MODE_FRONT;

        } else if(std::find(states_begin, states_end, state_enum::DisableCulling) != states_end) {
            raster_desc.CullMode = D3D12_CULL_MODE_NONE;

        } else {
            raster_desc.CullMode = D3D12_CULL_MODE_BACK;
        }
        raster_desc.FrontCounterClockwise = true;
        raster_desc.DepthBias = input.depth_bias;
        raster_desc.SlopeScaledDepthBias = input.slope_scaled_depth_bias;
        raster_desc.DepthClipEnable = true;
        if(input.msaa_support != msaa_support_enum::None) {
            raster_desc.MultisampleEnable = true;
        }
        raster_desc.ForcedSampleCount = 0;
        raster_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;

        /*
         * Depth and Stencil
         */

        D3D12_DEPTH_STENCIL_DESC& ds_desc = pipeline_state_desc.DepthStencilState;
        ds_desc.DepthEnable = std::find(states_begin, states_end, state_enum::DisableDepthTest) == states_end;
        if(std::find(states_begin, states_end, state_enum::DisableDepthWrite) != states_end) {
            ds_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

        } else {
            ds_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        }

        ds_desc.DepthFunc = to_dx12_compare_func(input.depth_func);
        ds_desc.StencilEnable = std::find(states_begin, states_end, state_enum::EnableStencilTest) != states_end;
        ds_desc.StencilReadMask = input.stencil_read_mask;
        ds_desc.StencilWriteMask = input.stencil_write_mask;
        if(input.front_face) {
            const stencil_op_state& front_face = *input.front_face;
            ds_desc.FrontFace.StencilFailOp = to_dx12_stencil_op(front_face.fail_op);
            ds_desc.FrontFace.StencilDepthFailOp = to_dx12_stencil_op(front_face.depth_fail_op);
            ds_desc.FrontFace.StencilPassOp = to_dx12_stencil_op(front_face.pass_op);
            ds_desc.FrontFace.StencilFunc = to_dx12_compare_func(front_face.compare_op);
        }
        if(input.back_face) {
            const stencil_op_state& back_face = *input.back_face;
            ds_desc.BackFace.StencilFailOp = to_dx12_stencil_op(back_face.fail_op);
            ds_desc.BackFace.StencilDepthFailOp = to_dx12_stencil_op(back_face.depth_fail_op);
            ds_desc.BackFace.StencilPassOp = to_dx12_stencil_op(back_face.pass_op);
            ds_desc.BackFace.StencilFunc = to_dx12_compare_func(back_face.compare_op);
        }

        /*
         * Input description
         */

        const std::unordered_map<vertex_field_enum, vertex_attribute> all_formats = get_all_vertex_attributes();

        std::vector<D3D12_INPUT_ELEMENT_DESC> input_descs;
        input_descs.reserve(input.vertex_fields.size());
        for(const vertex_field_data& vertex_field : input.vertex_fields) {
            const vertex_attribute& attr = all_formats.at(vertex_field.field);

            D3D12_INPUT_ELEMENT_DESC desc = {};
            desc.SemanticName = vertex_field.semantic_name.data();
            desc.Format = attr.format;
            desc.InputSlot = 0;
            desc.AlignedByteOffset = attr.offset;
            desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            desc.InstanceDataStepRate = 0;

            input_descs.push_back(desc);
        }

        pipeline_state_desc.InputLayout.NumElements = input_descs.size();
        pipeline_state_desc.InputLayout.pInputElementDescs = input_descs.data();

        /*
         * Index buffer strip cut and primitive topology
         */

        pipeline_state_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        pipeline_state_desc.PrimitiveTopologyType = to_dx12_topology(input.primitive_mode);

        /*
         * Render targets
         */

        uint32_t i = 0;
        for(i = 0; i < render_pass.texture_outputs.size(); i++) {
            const texture_attachment& attachment = render_pass.texture_outputs.at(i);
            const dx12_texture& tex = dynamic_textures.at(attachment.name);
            if(tex.is_depth_texture()) {
                pipeline_state_desc.DSVFormat = tex.get_dxgi_format();

            } else {
                pipeline_state_desc.RTVFormats[i] = to_dxgi_format(tex.get_data().format.pixel_format);
            }
        }
        pipeline_state_desc.NumRenderTargets = i;

        /*
         * Multisampling
         */

        if(input.msaa_support != msaa_support_enum::None) {
            pipeline_state_desc.SampleDesc.Count = 4;
            pipeline_state_desc.SampleDesc.Quality = 1;
        }

        /*
         * Debugging
         */

        if(nova_renderer::get_instance()->get_settings().get_options().debug.enabled) {
            pipeline_state_desc.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
        }

        /*
         * PSO creation!
         */

        const HRESULT hr = device->CreateGraphicsPipelineState(&pipeline_state_desc, IID_PPV_ARGS(&output.pso));
        if(FAILED(hr)) {
            throw shader_compilation_failed("Could not create PSO");
        }

        return output;
    }

    void add_resource_to_descriptor_table(D3D12_DESCRIPTOR_RANGE_TYPE descriptor_type, const D3D12_SHADER_INPUT_BIND_DESC & bind_desc, const uint32_t set, std::unordered_map<uint32_t, std::vector<D3D12_DESCRIPTOR_RANGE1>>& tables) {
        D3D12_DESCRIPTOR_RANGE1 range = {};
        range.BaseShaderRegister = bind_desc.BindPoint;
        range.RegisterSpace = bind_desc.Space;
        range.NumDescriptors = 1;
        range.RangeType = descriptor_type;
        range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        tables[set].push_back(range);
    }
    
    ComPtr<ID3DBlob> compile_shader(const shader_source& shader, const std::string& target, 
        const spirv_cross::CompilerHLSL::Options& options, std::unordered_map<uint32_t, std::vector<D3D12_DESCRIPTOR_RANGE1>>& tables) {

        spirv_cross::CompilerHLSL shader_compiler(shader.source);
        shader_compiler.set_hlsl_options(options);

        const spirv_cross::ShaderResources resources = shader_compiler.get_shader_resources();

        // Make maps of all the types of things we care about
        std::unordered_map<std::string, spirv_cross::Resource> spirv_sampled_images;
        spirv_sampled_images.reserve(resources.sampled_images.size());
        for(const spirv_cross::Resource& sampled_image : resources.sampled_images) {
            spirv_sampled_images[sampled_image.name] = sampled_image;
        }
        
        std::unordered_map<std::string, spirv_cross::Resource> spirv_uniform_buffers;
        spirv_uniform_buffers.reserve(resources.uniform_buffers.size());
        for(const spirv_cross::Resource& uniform_buffer : resources.uniform_buffers) {
            spirv_uniform_buffers[uniform_buffer.name] = uniform_buffer;
        }
        
        std::string shader_hlsl = shader_compiler.compile();

        const fs::path& filename = shader.filename;
        fs::path debug_path = filename.filename();
        debug_path.replace_extension(target + ".generated.hlsl");
        write_to_file(shader_hlsl, debug_path);

        ComPtr<ID3DBlob> shader_blob;
        ComPtr<ID3DBlob> shader_compile_errors;
        HRESULT hr = D3DCompile2(shader_hlsl.data(), shader_hlsl.size(), filename.string().c_str(), nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", target.c_str(),
            D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_IEEE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, 0, nullptr,
            0, &shader_blob, &shader_compile_errors);
        if(FAILED(hr)) {
            std::stringstream ss;
            ss << "Could not compile vertex shader for pipeline " << filename.string() << ": " << static_cast<char*>(shader_compile_errors->GetBufferPointer());
            throw shader_compilation_failed(ss.str());
        }


        ComPtr<ID3D12ShaderReflection> shader_reflector;
        hr = D3DReflect(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), IID_PPV_ARGS(&shader_reflector));
        if(FAILED(hr)) {
            throw shader_reflection_failed("Could not create reflector, error code " + std::to_string(hr));
        }

        D3D12_SHADER_DESC shader_desc;
        hr = shader_reflector->GetDesc(&shader_desc);
        if(FAILED(hr)) {
            throw shader_reflection_failed("Could not get shader description");
        }

        std::unordered_map<std::string, D3D12_SHADER_INPUT_BIND_DESC> shader_inputs(shader_desc.BoundResources);
        // For each resource in the DX12 shader, find its set and binding in SPIR-V. Translate the sets and bindings into places in descriptor tables
        for(uint32_t i = 0; i < shader_desc.BoundResources; i++) {
            D3D12_SHADER_INPUT_BIND_DESC bind_desc;
            hr = shader_reflector->GetResourceBindingDesc(i, &bind_desc);
            if(FAILED(hr)) {
                throw shader_reflection_failed("Could not get description for bind point " + std::to_string(i));
            }

            D3D12_DESCRIPTOR_RANGE_TYPE descriptor_type = {};
            spirv_cross::Resource spirv_resource = {};
            uint32_t set = 0;

            switch(bind_desc.Type) {
                case D3D_SIT_CBUFFER:
                    descriptor_type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                    spirv_resource = spirv_uniform_buffers.at(bind_desc.Name);
                    set = shader_compiler.get_decoration(spirv_resource.id, spv::DecorationDescriptorSet);
                    add_resource_to_descriptor_table(descriptor_type, bind_desc, set, tables);
                    break;

                case D3D_SIT_TEXTURE:
                case D3D_SIT_TBUFFER:
                    descriptor_type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                    spirv_resource = spirv_sampled_images.at(bind_desc.Name);
                    set = shader_compiler.get_decoration(spirv_resource.id, spv::DecorationDescriptorSet);
                    add_resource_to_descriptor_table(descriptor_type, bind_desc, set, tables);

                    // Also add a descriptor table entry for the sampler
                    add_resource_to_descriptor_table(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, bind_desc, set, tables);
                    break;

                case D3D_SIT_SAMPLER:
                    descriptor_type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                    break;

                case D3D_SIT_STRUCTURED:
                case D3D_SIT_UAV_RWTYPED:
                case D3D_SIT_UAV_RWSTRUCTURED:
                case D3D_SIT_UAV_RWBYTEADDRESS: 
                case D3D_SIT_UAV_APPEND_STRUCTURED: 
                case D3D_SIT_UAV_CONSUME_STRUCTURED: 
                case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                    descriptor_type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                    break;
            }
        }

        return shader_blob;
    }
    
    ComPtr<ID3D12RootSignature> dx12_render_engine::create_root_signature(const std::unordered_map<uint32_t, std::vector<D3D12_DESCRIPTOR_RANGE1>>& tables) const {
        std::vector<D3D12_ROOT_PARAMETER1> params(tables.size());
        std::size_t cur_param = 0;
        for(const auto& table : tables) {
            D3D12_ROOT_PARAMETER1& parameter = params.at(cur_param);
            parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            parameter.DescriptorTable.NumDescriptorRanges = table.second.size();
            parameter.DescriptorTable.pDescriptorRanges = table.second.data();

            cur_param++;
        }

        D3D12_VERSIONED_ROOT_SIGNATURE_DESC versioned_sig = {};
        versioned_sig.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
        versioned_sig.Desc_1_1.NumParameters = params.size();
        versioned_sig.Desc_1_1.pParameters = params.data();

        ComPtr<ID3DBlob> compiled_root_sig;
        ComPtr<ID3DBlob> root_sig_compile_error;
        HRESULT hr = D3D12SerializeVersionedRootSignature(&versioned_sig, &compiled_root_sig, &root_sig_compile_error);
        if(FAILED(hr)) {
            const std::string root_sig_compile_error_str = static_cast<char*>(root_sig_compile_error->GetBufferPointer());
            throw shader_compilation_failed("Could not compile root signature: " + root_sig_compile_error_str);
        }

        ComPtr<ID3D12RootSignature> root_sig;
        hr = device->CreateRootSignature(0, compiled_root_sig->GetBufferPointer(), compiled_root_sig->GetBufferSize(), IID_PPV_ARGS(&root_sig));
        if(FAILED(hr)) {
            throw shader_compilation_failed("Could not create root signature");
        }
        
        return root_sig;
    }

    bool operator==(const D3D12_ROOT_PARAMETER1& param1, const D3D12_ROOT_PARAMETER1& param2) {
        if(param1.ParameterType != param2.ParameterType) {
            return false;
        }

        switch(param1.ParameterType) {
        case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            return param1.DescriptorTable == param2.DescriptorTable;

        case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            return param1.Constants == param2.Constants;

        case D3D12_ROOT_PARAMETER_TYPE_CBV:
        case D3D12_ROOT_PARAMETER_TYPE_SRV:
        case D3D12_ROOT_PARAMETER_TYPE_UAV:
            return param1.Descriptor == param2.Descriptor;

        default:
            return false;
        }
    }

    bool operator!=(const D3D12_ROOT_PARAMETER1& param1, const D3D12_ROOT_PARAMETER1& param2) {
        return !(param1 == param2);
    }

    bool operator==(const D3D12_ROOT_DESCRIPTOR_TABLE1& table1, const D3D12_ROOT_DESCRIPTOR_TABLE1& table2) {
        if(table1.NumDescriptorRanges != table2.NumDescriptorRanges) {
            return false;
        }

        for(uint32_t i = 0; i < table1.NumDescriptorRanges; i++) {
            if(table1.pDescriptorRanges[i] != table2.pDescriptorRanges[i]) {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const D3D12_ROOT_DESCRIPTOR_TABLE1& table1, const D3D12_ROOT_DESCRIPTOR_TABLE1& table2) {
        return !(table1 == table2);
    }

    bool operator==(const D3D12_DESCRIPTOR_RANGE1& range1, const D3D12_DESCRIPTOR_RANGE1& range2) {
        return range1.RangeType == range2.RangeType &&
            range1.NumDescriptors == range2.NumDescriptors &&
            range1.BaseShaderRegister == range2.BaseShaderRegister &&
            range1.RegisterSpace == range2.RegisterSpace &&
            range1.Flags == range2.Flags &&
            range1.OffsetInDescriptorsFromTableStart == range2.OffsetInDescriptorsFromTableStart;
    }

    bool operator!=(const D3D12_DESCRIPTOR_RANGE1& range1, const D3D12_DESCRIPTOR_RANGE1& range2) {
        return !(range1 == range2);
    }
    
    bool operator==(const D3D12_ROOT_CONSTANTS& lhs, const D3D12_ROOT_CONSTANTS& rhs) {
        return lhs.ShaderRegister == rhs.ShaderRegister &&
            lhs.RegisterSpace == rhs.RegisterSpace &&
            lhs.Num32BitValues == rhs.Num32BitValues;
    }

    bool operator==(const D3D12_ROOT_DESCRIPTOR1& lhs, const D3D12_ROOT_DESCRIPTOR1& rhs) {
        return lhs.ShaderRegister == rhs.ShaderRegister &&
            lhs.RegisterSpace == rhs.RegisterSpace &&
            lhs.Flags == rhs.Flags;
    }
}  // namespace nova
