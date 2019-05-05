/*!
 * \author ddubois
 * \date 03-Apr-19.
 */

#include <D3DCompiler.h>
#include <d3d12sdklayers.h>

#include <spirv_cross/spirv_hlsl.hpp>

#include "../../loading/shaderpack/shaderpack_loading.hpp"
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

    std::shared_ptr<Window> DX12RenderEngine::get_window() const { return window; }

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

    Result<Renderpass*> DX12RenderEngine::create_renderpass(const shaderpack::RenderPassCreateInfo& data) {
        DX12Renderpass* renderpass = new DX12Renderpass;

        return Result<Renderpass*>(renderpass);
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

    Result<PipelineInterface*> DX12RenderEngine::create_pipeline_interface(
        const std::unordered_map<std::string, ResourceBindingDescription>& bindings,
        const std::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
        const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture) {

        DX12PipelineInterface* pipeline_interface = new DX12PipelineInterface;
        pipeline_interface->table_layouts.reserve(16);

        for(const auto& [binding_name, binding] : bindings) {
            pipeline_interface->table_layouts[binding.set].reserve(16);
            pipeline_interface->table_layouts[binding.set].push_back(binding);
        }

        for(const auto& [set, bindings] : pipeline_interface->table_layouts) {
            if(set > pipeline_interface->table_layouts.size()) {
                return Result<PipelineInterface*>(
                    NovaError("Pipeline interface doesn't use descriptor sets sequentially, but it needs to"));
            }
        }

        const std::size_t num_sets = pipeline_interface->table_layouts.size();

        D3D12_ROOT_SIGNATURE_DESC root_sig_desc = {};
        root_sig_desc.NumParameters = num_sets;
        root_sig_desc.pParameters = new D3D12_ROOT_PARAMETER[num_sets];

        // Make a descriptor table for each descriptor set
        for(uint32_t set = 0; set < num_sets; set++) {
            std::vector<ResourceBindingDescription>& descriptor_layouts = pipeline_interface->table_layouts.at(set);
            D3D12_ROOT_PARAMETER& param = const_cast<D3D12_ROOT_PARAMETER&>(root_sig_desc.pParameters[set]);
            param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

            param.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(descriptor_layouts.size());
            param.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[descriptor_layouts.size()];

            for(uint32_t i = 0; i < descriptor_layouts.size(); i++) {
                const ResourceBindingDescription& desc = descriptor_layouts.at(i);

                // Microsoft's sample DX12 renderer uses const_cast don't yell at me
                D3D12_DESCRIPTOR_RANGE& descriptor_range = const_cast<D3D12_DESCRIPTOR_RANGE&>(param.DescriptorTable.pDescriptorRanges[i]);
                descriptor_range.RangeType = to_dx12_range_type(desc.type);
                descriptor_range.NumDescriptors = desc.count;
                descriptor_range.BaseShaderRegister = desc.binding;
                descriptor_range.RegisterSpace = 0;
                descriptor_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            }
        }

        ComPtr<ID3DBlob> root_sig_blob;
        ComPtr<ID3DBlob> error_blob;

        const HRESULT res = D3D12SerializeRootSignature(&root_sig_desc,
                                                        D3D_ROOT_SIGNATURE_VERSION_1_1,
                                                        root_sig_blob.GetAddressOf(),
                                                        error_blob.GetAddressOf());
        if(!SUCCEEDED(res)) {
            const std::string err_str = static_cast<const char*>(error_blob->GetBufferPointer());
            return Result<PipelineInterface*>(NovaError(err_str));
        }

        device->CreateRootSignature(1,
                                    root_sig_blob->GetBufferPointer(),
                                    root_sig_blob->GetBufferSize(),
                                    IID_PPV_ARGS(&pipeline_interface->root_sig));

        pipeline_interface->color_attachments = color_attachments;
        pipeline_interface->depth_texture = depth_texture;

        return Result(static_cast<PipelineInterface*>(pipeline_interface));
    }

    DescriptorPool* DX12RenderEngine::create_descriptor_pool([[maybe_unused]] uint32_t num_sampled_images,
                                                             [[maybe_unused]] uint32_t num_samplers,
                                                             [[maybe_unused]] uint32_t num_uniform_buffers) {

        // TODO: A way to make CPU-only heaps that get copies to shader-visible heaps at the start of each frame
        // And do that for every backend
        DX12DescriptorPool* pool = new DX12DescriptorPool;
        return pool;
    }

    std::vector<DescriptorSet*> DX12RenderEngine::create_descriptor_sets(const PipelineInterface* pipeline_interface,
                                                                         [[maybe_unused]] const DescriptorPool* pool) {
        // Create a descriptor heap for each descriptor set
        // This is kinda gross and maybe I'll move to something else eventually but I gotta get past this code

        // send help

        const DX12PipelineInterface* dx12_pipeline_interface = static_cast<const DX12PipelineInterface*>(pipeline_interface);

        const uint32_t num_sets = dx12_pipeline_interface->table_layouts.size();

        std::vector<DescriptorSet*> descriptor_sets;
        descriptor_sets.reserve(num_sets);

        for(uint32_t i = 0; i < num_sets; i++) {
            const std::vector<ResourceBindingDescription> bindings_for_set = dx12_pipeline_interface->table_layouts.at(i);

            DX12DescriptorSet* set = new DX12DescriptorSet;

            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = bindings_for_set.size();
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = 0;

            device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&set->heap));

            descriptor_sets.push_back(set);
        }

        return descriptor_sets;
    }

    Result<Pipeline*> DX12RenderEngine::create_pipeline(const PipelineInterface* pipeline_interface,
                                                        const shaderpack::PipelineCreateInfo& data) {
        const DX12PipelineInterface* dx12_pipeline_interface = static_cast<const DX12PipelineInterface*>(pipeline_interface);

        DX12Pipeline* pipeline = new DX12Pipeline;

        const auto states_begin = data.states.begin();
        const auto states_end = data.states.end();

        D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc = {};

        /*
         * Compile all shader stages
         */

        std::unordered_map<uint32_t, std::vector<D3D12_DESCRIPTOR_RANGE1>> shader_inputs;
        spirv_cross::CompilerHLSL::Options options = {};
        options.shader_model = 51;

        ComPtr<ID3DBlob> vertex_blob = compile_shader(data.vertex_shader, "vs_5_1", options, shader_inputs);
        pipeline_state_desc.VS.BytecodeLength = vertex_blob->GetBufferSize();
        pipeline_state_desc.VS.pShaderBytecode = vertex_blob->GetBufferPointer();

        if(data.geometry_shader) {
            ComPtr<ID3DBlob> geometry_blob = compile_shader(*data.geometry_shader, "gs_5_1", options, shader_inputs);
            pipeline_state_desc.GS.BytecodeLength = geometry_blob->GetBufferSize();
            pipeline_state_desc.GS.pShaderBytecode = geometry_blob->GetBufferPointer();
        }

        if(data.tessellation_control_shader) {
            ComPtr<ID3DBlob> tessellation_control_blob = compile_shader(*data.tessellation_control_shader,
                                                                        "hs_5_1",
                                                                        options,
                                                                        shader_inputs);
            pipeline_state_desc.HS.BytecodeLength = tessellation_control_blob->GetBufferSize();
            pipeline_state_desc.HS.pShaderBytecode = tessellation_control_blob->GetBufferPointer();
        }
        if(data.tessellation_evaluation_shader) {
            ComPtr<ID3DBlob> tessellation_evaluation_blob = compile_shader(*data.tessellation_evaluation_shader,
                                                                           "ds_5_1",
                                                                           options,
                                                                           shader_inputs);
            pipeline_state_desc.DS.BytecodeLength = tessellation_evaluation_blob->GetBufferSize();
            pipeline_state_desc.DS.pShaderBytecode = tessellation_evaluation_blob->GetBufferPointer();
        }

        if(data.fragment_shader) {
            ComPtr<ID3DBlob> fragment_blob = compile_shader(*data.fragment_shader, "ps_5_1", options, shader_inputs);
            pipeline_state_desc.PS.BytecodeLength = fragment_blob->GetBufferSize();
            pipeline_state_desc.PS.pShaderBytecode = fragment_blob->GetBufferPointer();
        }

        pipeline->root_signature = dx12_pipeline_interface->root_sig.Get();
        pipeline_state_desc.pRootSignature = pipeline->root_signature;

        /*
         * Blend state
         */

        pipeline_state_desc.BlendState.AlphaToCoverageEnable = std::find(states_begin,
                                                                         states_end,
                                                                         shaderpack::StateEnum::EnableAlphaToCoverage) != states_end;
        pipeline_state_desc.BlendState.IndependentBlendEnable = false;
        D3D12_RENDER_TARGET_BLEND_DESC& blend_state = pipeline_state_desc.BlendState.RenderTarget[0];
        blend_state.BlendEnable = std::find(states_begin, states_end, shaderpack::StateEnum::Blending) != states_end;
        blend_state.SrcBlend = to_dx12_blend(data.source_blend_factor);
        blend_state.DestBlend = to_dx12_blend(data.destination_blend_factor);
        blend_state.BlendOp = D3D12_BLEND_OP_ADD;
        blend_state.SrcBlendAlpha = to_dx12_blend(data.source_blend_factor);
        blend_state.DestBlendAlpha = to_dx12_blend(data.destination_blend_factor);
        blend_state.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blend_state.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        pipeline_state_desc.SampleMask = 0xFFFFFFFF;

        /*
         * Rasterizer state
         */

        D3D12_RASTERIZER_DESC& raster_desc = pipeline_state_desc.RasterizerState;
        raster_desc.FillMode = D3D12_FILL_MODE_SOLID;
        if(std::find(states_begin, states_end, shaderpack::StateEnum::InvertCulling) != states_end) {
            raster_desc.CullMode = D3D12_CULL_MODE_FRONT;

        } else if(std::find(states_begin, states_end, shaderpack::StateEnum::DisableCulling) != states_end) {
            raster_desc.CullMode = D3D12_CULL_MODE_NONE;

        } else {
            raster_desc.CullMode = D3D12_CULL_MODE_BACK;
        }
        raster_desc.FrontCounterClockwise = true;
        raster_desc.DepthBias = static_cast<UINT>(std::round(data.depth_bias));
        raster_desc.SlopeScaledDepthBias = data.slope_scaled_depth_bias;
        raster_desc.DepthClipEnable = true;
        if(data.msaa_support != shaderpack::MsaaSupportEnum::None) {
            raster_desc.MultisampleEnable = true;
        }
        raster_desc.ForcedSampleCount = 0;
        raster_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;

        /*
         * Depth and Stencil
         */

        D3D12_DEPTH_STENCIL_DESC& ds_desc = pipeline_state_desc.DepthStencilState;
        ds_desc.DepthEnable = std::find(states_begin, states_end, shaderpack::StateEnum::DisableDepthTest) == states_end;
        if(std::find(states_begin, states_end, shaderpack::StateEnum::DisableDepthWrite) != states_end) {
            ds_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        } else {
            ds_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        }

        ds_desc.DepthFunc = to_dx12_compare_func(data.depth_func);
        ds_desc.StencilEnable = std::find(states_begin, states_end, shaderpack::StateEnum::EnableStencilTest) != states_end;
        ds_desc.StencilReadMask = data.stencil_read_mask;
        ds_desc.StencilWriteMask = data.stencil_write_mask;
        if(data.front_face) {
            const shaderpack::StencilOpState& front_face = *data.front_face;
            ds_desc.FrontFace.StencilFailOp = to_dx12_stencil_op(front_face.fail_op);
            ds_desc.FrontFace.StencilDepthFailOp = to_dx12_stencil_op(front_face.depth_fail_op);
            ds_desc.FrontFace.StencilPassOp = to_dx12_stencil_op(front_face.pass_op);
            ds_desc.FrontFace.StencilFunc = to_dx12_compare_func(front_face.compare_op);
        }
        if(data.back_face) {
            const shaderpack::StencilOpState& back_face = *data.back_face;
            ds_desc.BackFace.StencilFailOp = to_dx12_stencil_op(back_face.fail_op);
            ds_desc.BackFace.StencilDepthFailOp = to_dx12_stencil_op(back_face.depth_fail_op);
            ds_desc.BackFace.StencilPassOp = to_dx12_stencil_op(back_face.pass_op);
            ds_desc.BackFace.StencilFunc = to_dx12_compare_func(back_face.compare_op);
        }

        /*
         * Input description
         */

        const std::unordered_map<shaderpack::VertexFieldEnum, VertexAttribute> all_formats = get_all_vertex_attributes();

        std::vector<D3D12_INPUT_ELEMENT_DESC> input_descs;
        input_descs.reserve(data.vertex_fields.size());
        for(const shaderpack::VertexFieldData& vertex_field : data.vertex_fields) {
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

        pipeline_state_desc.InputLayout.NumElements = static_cast<UINT>(input_descs.size());
        pipeline_state_desc.InputLayout.pInputElementDescs = input_descs.data();

        /*
         * Index buffer strip cut and primitive topology
         */

        pipeline_state_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        pipeline_state_desc.PrimitiveTopologyType = to_dx12_topology(data.primitive_mode);

        /*
         * Render targets
         */

        const DX12Renderpass* dx12_renderpass = static_cast<const DX12Renderpass*>(renderpass);
        uint32_t i = 0;
        for(i = 0; i < dx12_renderpass->color_attachments.size(); i++) {
            const shaderpack::TextureAttachmentInfo& attachment_info = dx12_renderpass->color_attachments.at(i);
            if(attachment_info.pixel_format == shaderpack::PixelFormatEnum::Depth ||
               attachment_info.pixel_format == shaderpack::PixelFormatEnum::DepthStencil) {
                pipeline_state_desc.DSVFormat = to_dxgi_format(attachment_info.pixel_format);

            } else {
                pipeline_state_desc.RTVFormats[i] = to_dxgi_format(attachment_info.pixel_format);
            }
        }
        pipeline_state_desc.NumRenderTargets = i;

        /*
         * Multisampling
         */

        if(data.msaa_support != shaderpack::MsaaSupportEnum::None) {
            pipeline_state_desc.SampleDesc.Count = 4;
            pipeline_state_desc.SampleDesc.Quality = 1;
        }

        /*
         * Debugging
         */

        if(settings.debug.enabled) {
            pipeline_state_desc.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
        }

        /*
         * PSO creation!
         */

        const HRESULT hr = device->CreateGraphicsPipelineState(&pipeline_state_desc, IID_PPV_ARGS(&pipeline->pso));
        if(FAILED(hr)) {
            return Result<Pipeline*>(NovaError("Could not create PSO"));
        }

        return Result(static_cast<Pipeline*>(pipeline));
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

    ID3D12RootSignature* DX12RenderEngine::create_root_signature(
        const std::unordered_map<std::string, ResourceBindingDescription>& bindings) {}

    ComPtr<ID3DBlob> compile_shader(const shaderpack::ShaderSource& shader,
                                    const std::string& target,
                                    const spirv_cross::CompilerHLSL::Options& options,
                                    std::unordered_map<uint32_t, std::vector<D3D12_DESCRIPTOR_RANGE1>>& tables) {

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
        HRESULT hr = D3DCompile2(shader_hlsl.data(),
                                 shader_hlsl.size(),
                                 filename.string().c_str(),
                                 nullptr,
                                 D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                 "main",
                                 target.c_str(),
                                 D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_IEEE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 &shader_blob,
                                 &shader_compile_errors);
        if(FAILED(hr)) {
            std::stringstream ss;
            ss << "Could not compile vertex shader for pipeline " << filename.string() << ": "
               << static_cast<char*>(shader_compile_errors->GetBufferPointer());
            throw shaderpack::shader_compilation_failed(ss.str());
        }

        ComPtr<ID3D12ShaderReflection> shader_reflector;
        hr = D3DReflect(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), IID_PPV_ARGS(&shader_reflector));
        if(FAILED(hr)) {
            throw shaderpack::shader_reflection_failed("Could not create reflector, error code " + std::to_string(hr));
        }

        D3D12_SHADER_DESC shader_desc;
        hr = shader_reflector->GetDesc(&shader_desc);
        if(FAILED(hr)) {
            throw shaderpack::shader_reflection_failed("Could not get shader description");
        }

        std::unordered_map<std::string, D3D12_SHADER_INPUT_BIND_DESC> shader_inputs(shader_desc.BoundResources);
        // For each resource in the DX12 shader, find its set and binding in SPIR-V. Translate the sets and bindings into places in
        // descriptor tables
        for(uint32_t i = 0; i < shader_desc.BoundResources; i++) {
            D3D12_SHADER_INPUT_BIND_DESC bind_desc;
            hr = shader_reflector->GetResourceBindingDesc(i, &bind_desc);
            if(FAILED(hr)) {
                throw shaderpack::shader_reflection_failed("Could not get description for bind point " + std::to_string(i));
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

} // namespace nova::renderer::rhi
