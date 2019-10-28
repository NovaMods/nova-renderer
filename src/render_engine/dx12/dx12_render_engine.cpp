#pragma warning(push, 0)
#include <D3DCompiler.h>
#include <d3d12sdklayers.h>
#include <spirv_hlsl.hpp>
#pragma warning pop

#include "nova_renderer/util/platform.hpp"

#include "../../loading/shaderpack/shaderpack_loading.hpp"
#include "../../util/logger.hpp"
#include "../../util/windows_utils.hpp"
#include "../../windowing/win32_window.hpp"
#include "d3dx12.h"
#include "dx12_command_list.hpp"
#include "dx12_render_engine.hpp"
#include "dx12_structs.hpp"
#include "dx12_swapchain.hpp"
#include "dx12_utils.hpp"

using Microsoft::WRL::ComPtr;

#define CPU_FENCE_SIGNALED 16
#define GPU_FENCE_SIGNALED 32

namespace nova::renderer::rhi {
    D3D12RenderEngine::D3D12RenderEngine(NovaSettingsAccessManager& settings) : RenderEngine(&mallocator, settings) {
        create_device();

        create_queues();

        open_window_and_create_swapchain(settings.settings.window, settings.settings.max_in_flight_frames);

        rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        cbv_srv_uav_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        if(settings.settings.debug.enabled) {
            setup_debug_output();
        }
    }

    void D3D12RenderEngine::set_num_renderpasses(const uint32_t num_renderpasses) {
        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_descriptor = {};
        rtv_heap_descriptor.NumDescriptors = num_renderpasses * 8;
        rtv_heap_descriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

        const HRESULT hr = device->CreateDescriptorHeap(&rtv_heap_descriptor, IID_PPV_ARGS(&rtv_descriptor_heap));
        if(FAILED(hr)) {
            NOVA_LOG(FATAL) << "Could not create descriptor heap for the RTV";
        }
    }

    ntl::Result<DeviceMemory*> D3D12RenderEngine::allocate_device_memory(const uint64_t size,
                                                                         const MemoryUsage type,
                                                                         const ObjectType allowed_objects) {
        auto* memory = new DX12DeviceMemory;

        D3D12_HEAP_DESC desc = {};
        desc.SizeInBytes = size;
        desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

        switch(type) {
            case MemoryUsage::DeviceOnly:
                desc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
                break;

            case MemoryUsage::LowFrequencyUpload:
                [[fallthrough]];
            case MemoryUsage::StagingBuffer:
                desc.Properties.Type = D3D12_HEAP_TYPE_UPLOAD;
        }

        desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

        switch(allowed_objects) {
            case ObjectType::Buffer:
                desc.Flags = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
                break;

            case ObjectType::Texture:
                desc.Flags = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_BUFFERS;
                break;

            case ObjectType::RenderTexture:
                desc.Flags = D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
                break;

            case ObjectType::SwapchainSurface:
                desc.Flags = D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES | D3D12_HEAP_FLAG_ALLOW_DISPLAY;
                break;

            case ObjectType::Any:
                break;
        }

        if(type == MemoryUsage::DeviceOnly) {
            adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &local_info);

            if(size > local_info.AvailableForReservation) {
                return ntl::Result<DeviceMemory*>(ntl::NovaError("Not enough space for memory allocation"));
            }

        } else {
            adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &non_local_info);

            if(size > non_local_info.AvailableForReservation) {
                return ntl::Result<DeviceMemory*>(ntl::NovaError("Not enough space for memory allocation"));
            }
        }

        const auto result = device->CreateHeap(&desc, IID_PPV_ARGS(&memory->heap));
        if(SUCCEEDED(result)) {
            return ntl::Result<DeviceMemory*>(memory);

        } else {
            return ntl::Result<DeviceMemory*>(ntl::NovaError(to_string(result)));
        }
    }

    ntl::Result<Renderpass*> D3D12RenderEngine::create_renderpass(const shaderpack::RenderPassCreateInfo& data,
                                                                  const glm::uvec2& /* framebuffer_size */) {
        auto* renderpass = new DX12Renderpass;

        return ntl::Result<Renderpass*>(renderpass);
    }

    Framebuffer* D3D12RenderEngine::create_framebuffer(const Renderpass* /* renderpass */,
                                                       const std::vector<Image*>& attachments,
                                                       const glm::uvec2& framebuffer_size) {
        const size_t attachment_count = attachments.size();
        auto* framebuffer = new DX12Framebuffer;
        framebuffer->render_targets.reserve(attachment_count);

        std::vector<ID3D12Resource*> rendertargets;
        rendertargets.reserve(attachment_count);

        CD3DX12_CPU_DESCRIPTOR_HANDLE base_rtv_descriptor(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(),
                                                          next_rtv_descriptor_index,
                                                          rtv_descriptor_size);

        for(uint32_t i = 0; i < attachments.size(); i++) {
            const Image* attachment = attachments.at(i);
            const auto* d3d12_image = static_cast<const DX12Image*>(attachment);

            rendertargets.emplace_back(d3d12_image->resource.Get());

            framebuffer->render_targets.emplace_back(base_rtv_descriptor);

            // Increment the RTV handle
            framebuffer->render_targets.at(i).Offset(i, rtv_descriptor_size);

            // Create the Render Target View, which binds the swapchain buffer to the RTV handle
            device->CreateRenderTargetView(d3d12_image->resource.Get(), nullptr, framebuffer->render_targets.at(i));
        }

        next_rtv_descriptor_index += attachment_count;

        framebuffer->size = framebuffer_size;

        return framebuffer;
    }

    ntl::Result<PipelineInterface*> D3D12RenderEngine::create_pipeline_interface(
        const std::unordered_map<std::string, ResourceBindingDescription>& bindings,
        const std::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
        const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture) {
        auto* pipeline_interface = new DX12PipelineInterface;
        pipeline_interface->bindings = bindings;

        pipeline_interface->table_layouts.reserve(16);

        for(const auto& [binding_name, binding] : bindings) {
            pipeline_interface->table_layouts[binding.set].reserve(16);
            pipeline_interface->table_layouts[binding.set].push_back(binding);
        }

        for(const auto& [set, bindings] : pipeline_interface->table_layouts) {
            if(set > pipeline_interface->table_layouts.size()) {
                return ntl::Result<PipelineInterface*>(
                    ntl::NovaError("Pipeline interface doesn't use descriptor sets sequentially, but it needs to"));
            }
        }

        const UINT num_sets = static_cast<UINT>(pipeline_interface->table_layouts.size());

        D3D12_ROOT_SIGNATURE_DESC root_sig_desc = {};
        root_sig_desc.NumParameters = num_sets;
        root_sig_desc.pParameters = new D3D12_ROOT_PARAMETER[num_sets];

        // Make a descriptor table for each descriptor set
        for(uint32_t set = 0; set < num_sets; set++) {
            std::vector<ResourceBindingDescription>& descriptor_layouts = pipeline_interface->table_layouts.at(set);
            auto& param = const_cast<D3D12_ROOT_PARAMETER&>(root_sig_desc.pParameters[set]);
            param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

            param.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(descriptor_layouts.size());
            param.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[descriptor_layouts.size()];

            for(uint32_t i = 0; i < descriptor_layouts.size(); i++) {
                const ResourceBindingDescription& desc = descriptor_layouts.at(i);

                // Microsoft's sample DX12 renderer uses const_cast don't yell at me
                auto& descriptor_range = const_cast<D3D12_DESCRIPTOR_RANGE&>(param.DescriptorTable.pDescriptorRanges[i]);
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
            return ntl::Result<PipelineInterface*>(ntl::NovaError(err_str));
        }

        device->CreateRootSignature(1,
                                    root_sig_blob->GetBufferPointer(),
                                    root_sig_blob->GetBufferSize(),
                                    IID_PPV_ARGS(&pipeline_interface->root_sig));

        pipeline_interface->color_attachments = color_attachments;
        pipeline_interface->depth_texture = depth_texture;

        return ntl::Result(static_cast<PipelineInterface*>(pipeline_interface));
    }

    DescriptorPool* D3D12RenderEngine::create_descriptor_pool(uint32_t /* num_sampled_images */,
                                                              uint32_t /* num_samplers */,
                                                              uint32_t /* num_uniform_buffers */) {
        auto* pool = new DX12DescriptorPool; // This seems wrong
        return pool;
    }

    std::vector<DescriptorSet*> D3D12RenderEngine::create_descriptor_sets(const PipelineInterface* pipeline_interface,
                                                                          DescriptorPool* /* pool */) {
        // Create a descriptor heap for each descriptor set
        // This is kinda gross and maybe I'll move to something else eventually but I gotta get past this code

        // send help

        const auto* dx12_pipeline_interface = static_cast<const DX12PipelineInterface*>(pipeline_interface);

        const auto num_sets = static_cast<const uint32_t>(dx12_pipeline_interface->table_layouts.size());

        std::vector<DescriptorSet*> descriptor_sets;
        descriptor_sets.reserve(num_sets);

        for(uint32_t i = 0; i < num_sets; i++) {
            const std::vector<ResourceBindingDescription> bindings_for_set = dx12_pipeline_interface->table_layouts.at(i);

            auto* set = new DX12DescriptorSet;

            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = static_cast<uint32_t>(bindings_for_set.size());
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = 0;

            device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&set->heap));

            descriptor_sets.push_back(set);
        }

        return descriptor_sets;
    }

    void D3D12RenderEngine::update_descriptor_sets(std::vector<DescriptorSetWrite>& writes) {
        // We want to create descriptors in the heaps in the order of their bindings
        for(const DescriptorSetWrite& write : writes) {
            const auto* set = static_cast<const DX12DescriptorSet*>(write.set);
            CD3DX12_CPU_DESCRIPTOR_HANDLE write_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(set->heap->GetCPUDescriptorHandleForHeapStart(),
                                                                                       cbv_srv_uav_descriptor_size * write.binding);

            switch(write.type) {
                case DescriptorType::CombinedImageSampler: {
                    const DescriptorImageUpdate* image_update = write.image_info;
                    const auto* image = static_cast<const DX12Image*>(image_update->image);
                    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
                    srv_desc.Format = to_dxgi_format(image_update->format.pixel_format);
                    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                    srv_desc.Texture2D.MostDetailedMip = 0;
                    srv_desc.Texture2D.MipLevels = 1;
                    srv_desc.Texture2D.PlaneSlice = 0;
                    srv_desc.Texture2D.ResourceMinLODClamp = 0;

                    device->CreateShaderResourceView(image->resource.Get(), &srv_desc, write_handle);

                } break;

                case DescriptorType::UniformBuffer: {
                } break;

                case DescriptorType::StorageBuffer: {
                } break;

                default:;
            }
        }
    }

    ntl::Result<Pipeline*> D3D12RenderEngine::create_pipeline(PipelineInterface* pipeline_interface,
                                                              const shaderpack::PipelineCreateInfo& data) {
        const auto* dx12_pipeline_interface = static_cast<const DX12PipelineInterface*>(pipeline_interface);

        auto* pipeline = new_object<DX12Pipeline>();

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
        pipeline_state_desc.pRootSignature = pipeline->root_signature.Get();

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

        const std::vector<D3D12_INPUT_ELEMENT_DESC> input_descs = get_input_descriptions();
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

        uint32_t i = 0;
        for(i = 0; i < dx12_pipeline_interface->color_attachments.size(); i++) {
            const shaderpack::TextureAttachmentInfo& attachment_info = dx12_pipeline_interface->color_attachments.at(i);
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

        if(settings.settings.debug.enabled) {
            pipeline_state_desc.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
        }

        /*
         * PSO creation!
         */

        const HRESULT hr = device->CreateGraphicsPipelineState(&pipeline_state_desc, IID_PPV_ARGS(&pipeline->pso));
        if(FAILED(hr)) {
            return ntl::Result<Pipeline*>(MAKE_ERROR("Could not create PSO: {:s}", to_string(hr)));
        }

        return ntl::Result(static_cast<Pipeline*>(pipeline));
    }

    Buffer* D3D12RenderEngine::create_buffer(const BufferCreateInfo& info, DeviceMemoryResource& memory) {
        auto* buffer = new_object<DX12Buffer>();

        D3D12_RESOURCE_STATES states = {};
        switch(info.buffer_usage) {
            case BufferUsage::UniformBuffer: {
                states = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            } break;

            case BufferUsage::IndexBuffer: {
                states = D3D12_RESOURCE_STATE_INDEX_BUFFER;
            } break;

            case BufferUsage::VertexBuffer: {
                states = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            } break;

            default:;
        }

        D3D12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Buffer(info.size);

        // TODO: Handle buffer alignment
        const auto allocation = memory.allocate(info.size);
        const auto* dx12_memory = static_cast<DX12DeviceMemory*>(allocation.memory);

        device->CreatePlacedResource(dx12_memory->heap.Get(),
                                     allocation.allocation_info.offset.b_count(),
                                     &resource_desc,
                                     states,
                                     nullptr,
                                     IID_PPV_ARGS(&buffer->resource));

        buffer->size = bvestl::polyalloc::Bytes(info.size);

        return buffer;
    }

    void D3D12RenderEngine::write_data_to_buffer(const void* data, const uint64_t num_bytes, const uint64_t offset, const Buffer* buffer) {
        const auto dx_buffer = static_cast<const DX12Buffer*>(buffer);

        D3D12_RANGE mapped_range;
        mapped_range.Begin = offset;
        mapped_range.End = offset + num_bytes;

        void* mapped_buffer;
        dx_buffer->resource->Map(0, &mapped_range, &mapped_buffer);
        std::memcpy(mapped_buffer, data, num_bytes);
        dx_buffer->resource->Unmap(0, &mapped_range);
    }

    Image* D3D12RenderEngine::create_image(const shaderpack::TextureCreateInfo& info) {
        const auto image = new_object<DX12Image>();

        const shaderpack::TextureFormat& format = info.format;

        glm::uvec2 dimensions;
        if(format.dimension_type == shaderpack::TextureDimensionTypeEnum::Absolute) {
            dimensions.x = static_cast<uint32_t>(format.width);
            dimensions.y = static_cast<uint32_t>(format.height);
        } else {
            dimensions = swapchain->get_size();
        }

        const DXGI_FORMAT dx12_format = to_dxgi_format(format.pixel_format);

        DXGI_SAMPLE_DESC sample_desc;
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

        D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_RENDER_TARGET;

        if(format.pixel_format == shaderpack::PixelFormatEnum::Depth || format.pixel_format == shaderpack::PixelFormatEnum::DepthStencil) {
            texture_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        }

        ComPtr<ID3D12Resource> texture;
        auto heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        const HRESULT hr = device->CreateCommittedResource(&heap_props,
                                                           D3D12_HEAP_FLAG_NONE,
                                                           &texture_desc,
                                                           state,
                                                           nullptr,
                                                           IID_PPV_ARGS(&texture));

        if(FAILED(hr)) {
            NOVA_LOG(ERROR) << "Could not create texture " << info.name << ": Error code " << hr << ", Error description: " << to_string(hr)
                            << ", Windows error: '" << get_last_windows_error() << "'";

            return nullptr;
        }
        texture->SetName(s2ws(info.name).c_str());

        return image;
    }

    Semaphore* D3D12RenderEngine::create_semaphore() { return nullptr; }

    std::vector<Semaphore*> D3D12RenderEngine::create_semaphores(uint32_t num_semaphores) { return std::vector<Semaphore*>(); }

    Fence* D3D12RenderEngine::create_fence(const bool signaled) {
        auto* fence = new_object<DX12Fence>();

        const uint32_t initial_value = signaled ? CPU_FENCE_SIGNALED : 0;

        device->CreateFence(initial_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence->fence.GetAddressOf()));
        fence->event = CreateEvent(nullptr, false, signaled, nullptr);
        fence->fence->SetEventOnCompletion(CPU_FENCE_SIGNALED, fence->event);

        return fence;
    }

    std::vector<Fence*> D3D12RenderEngine::create_fences(const uint32_t num_fences, const bool signaled) {
        std::vector<Fence*> fences;
        fences.reserve(num_fences);

        const uint32_t initial_value = signaled ? CPU_FENCE_SIGNALED : 0;
        for(uint32_t i = 0; i < num_fences; i++) {
            auto* fence = new_object<DX12Fence>();

            device->CreateFence(initial_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence->fence.GetAddressOf()));
            fence->event = CreateEvent(nullptr, false, signaled, nullptr);
            fence->fence->SetEventOnCompletion(CPU_FENCE_SIGNALED, fence->event);

            fences.emplace_back(fence);
        }

        return fences;
    }

    void D3D12RenderEngine::wait_for_fences(const std::vector<Fence*> fences) {
        for(const Fence* fence : fences) {
            const auto* dx_fence = static_cast<const DX12Fence*>(fence);
            WaitForSingleObject(dx_fence->event, INFINITE);
        }
    }

    void D3D12RenderEngine::reset_fences(const std::vector<Fence*>& fences) {
        for(Fence* fence : fences) {
            auto* dx12_fence = static_cast<DX12Fence*>(fence);
            ResetEvent(dx12_fence->event);
        }
    }

    void D3D12RenderEngine::destroy_renderpass(Renderpass* /* pass */) { // No work needed, DX12Renderpasses don't own any GPU objects
    }

    void D3D12RenderEngine::destroy_framebuffer(Framebuffer* framebuffer) {
        auto* d3d12_framebuffer = static_cast<DX12Framebuffer*>(framebuffer);

        // TODO: Some way to free the framebuffer's RTV descriptors? Probably we'll just destroy all the rendergraph framebuffers together,
        // completely clearing out the RTV descriptor heap
    }

    void D3D12RenderEngine::destroy_pipeline_interface(PipelineInterface* pipeline_interface) {
        auto* dx12_interface = static_cast<DX12PipelineInterface*>(pipeline_interface);

        dx12_interface->root_sig = nullptr;
    }

    void D3D12RenderEngine::destroy_pipeline(Pipeline* pipeline) {
        auto* dx_pipeline = static_cast<DX12Pipeline*>(pipeline);
        dx_pipeline->pso = nullptr;
        dx_pipeline->root_signature = nullptr;
    }

    void D3D12RenderEngine::destroy_texture(Image* resource) {
        auto* d3d12_framebuffer = static_cast<DX12Image*>(resource);
        d3d12_framebuffer->resource = nullptr;
    }

    void D3D12RenderEngine::destroy_semaphores(std::vector<Semaphore*>& semaphores) {
        for(Semaphore* semaphore : semaphores) {
            auto* dx_semaphore = static_cast<DX12Semaphore*>(semaphore);
            dx_semaphore->fence = nullptr;
        }
    }

    void D3D12RenderEngine::destroy_fences(std::vector<Fence*>& fences) {
        for(Fence* fence : fences) {
            auto* dx_fence = static_cast<DX12Fence*>(fence);
            dx_fence->fence = nullptr;
            CloseHandle(dx_fence->event);
        }
    }

    CommandList* D3D12RenderEngine::get_command_list(const uint32_t thread_idx,
                                                     const QueueType needed_queue_type,
                                                     const CommandList::Level level) {
        D3D12_COMMAND_LIST_TYPE command_list_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        if(level == CommandList::Level::Secondary) {
            command_list_type = D3D12_COMMAND_LIST_TYPE_BUNDLE;

        } else {
            switch(needed_queue_type) {
                case QueueType::Graphics:
                    break;

                case QueueType::Transfer:
                    command_list_type = D3D12_COMMAND_LIST_TYPE_COPY;
                    break;

                case QueueType::AsyncCompute:
                    command_list_type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
                    break;
            }
        }

        ID3D12CommandAllocator* command_allocator = command_allocators.at(thread_idx).at(command_list_type);

        ComPtr<ID3D12CommandList> list;
        device->CreateCommandList(0, command_list_type, command_allocator, nullptr, IID_PPV_ARGS(&list));

        ComPtr<ID3D12GraphicsCommandList> graphics_list;
        list->QueryInterface(IID_PPV_ARGS(&graphics_list));

        return new_object<Dx12CommandList>(graphics_list);
    }

    void D3D12RenderEngine::submit_command_list(CommandList* cmds,
                                                const QueueType queue,
                                                Fence* fence_to_signal,
                                                const std::vector<Semaphore*>& wait_semaphores,
                                                const std::vector<Semaphore*>& signal_semaphores) {
        auto* dx_cmds = static_cast<Dx12CommandList*>(cmds);
        dx_cmds->cmds->Close();

        ComPtr<ID3D12CommandQueue> dx_queue;
        switch(queue) {
            case QueueType::Graphics:
                dx_queue = direct_command_queue;
                break;

            case QueueType::Transfer:
                dx_queue = copy_command_queue;
                break;

            case QueueType::AsyncCompute:
                dx_queue = compute_command_queue;
                break;
        }

        ID3D12GraphicsCommandList* gfx_cmds = dx_cmds->cmds.Get();
        ID3D12CommandList* submittable_cmds;
        gfx_cmds->QueryInterface(IID_PPV_ARGS(&submittable_cmds));

        for(const Semaphore* semaphore : wait_semaphores) {
            const auto* dx_semaphore = static_cast<const DX12Semaphore*>(semaphore);
            dx_queue->Wait(dx_semaphore->fence.Get(), GPU_FENCE_SIGNALED);
        }
        dx_queue->ExecuteCommandLists(1, &submittable_cmds);

        for(const Semaphore* semaphore : signal_semaphores) {
            const auto* dx_semaphore = static_cast<const DX12Semaphore*>(semaphore);
            dx_queue->Signal(dx_semaphore->fence.Get(), GPU_FENCE_SIGNALED);
        }

        const auto* dx_signal_fence = static_cast<const DX12Fence*>(fence_to_signal);
        dx_queue->Signal(dx_signal_fence->fence.Get(), CPU_FENCE_SIGNALED);
    }

    void D3D12RenderEngine::open_window_and_create_swapchain(const NovaSettings::WindowOptions& options, const uint32_t num_frames) {
        window = std::make_unique<Win32Window>(options);

        auto* win32_window = static_cast<Win32Window*>(window.get());

        const auto window_handle = win32_window->get_window_handle();

        swapchain = new DX12Swapchain(this,
                                      dxgi_factory.Get(),
                                      device.Get(),
                                      window_handle,
                                      glm::uvec2{options.height, options.width},
                                      num_frames,
                                      direct_command_queue.Get());
    }

    void D3D12RenderEngine::create_device() {
        if(settings.settings.debug.enabled && settings.settings.debug.enable_validation_layers) {
            ComPtr<ID3D12Debug> debug_controller;
            D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
            debug_controller->EnableDebugLayer();

            if(settings.settings.debug.enable_gpu_based_validation) {
                ComPtr<ID3D12Debug1> debug1;
                const auto hr = debug_controller->QueryInterface(IID_PPV_ARGS(&debug1));
                if(SUCCEEDED(hr)) {
                    debug1->SetEnableGPUBasedValidation(true);
                }
            }
        }

        NOVA_LOG(TRACE) << "Creating DX12 device";

        CHECK_ERROR(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgi_factory)), "Could not create DXGI Factory");

        NOVA_LOG(TRACE) << "Device created";

        ComPtr<IDXGIAdapter1> adapter1;

        uint32_t adapter_index = 0;
        bool adapter_found = false;

        while(dxgi_factory->EnumAdapters1(adapter_index, &adapter1) != DXGI_ERROR_NOT_FOUND) {
            DXGI_ADAPTER_DESC1 desc;
            adapter1->GetDesc1(&desc);

            if(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                // Ignore software devices
                adapter_index++;
                continue;
            }

            // Direct3D 12 is feature level 11.
            //
            // cool
            const HRESULT hr = D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
            if(SUCCEEDED(hr)) {
                adapter_found = true;
                adapter1->QueryInterface(IID_PPV_ARGS(&adapter));

                if(adapter) {
                    adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &local_info);
                    adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &non_local_info);

                } else {
                    NOVA_LOG(ERROR) << "Needed adapter interface not supported";
                }

                break;
            }

            adapter_index++;
        }

        if(!adapter_found) {
            NOVA_LOG(ERROR) << "Could not find a GPU that supports DX12";
        }
    }

    void D3D12RenderEngine::create_queues() {
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

    void D3D12RenderEngine::setup_debug_output() {
        const auto hr = device->QueryInterface(IID_PPV_ARGS(&info_queue));
        if(SUCCEEDED(hr)) {
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);

        } else {
            NOVA_LOG(ERROR) << "Could not set up debugging: " << to_string(hr);
        }
    }

    ComPtr<ID3DBlob> compile_shader(const shaderpack::ShaderSource& shader,
                                    const std::string& target,
                                    const spirv_cross::CompilerHLSL::Options& options,
                                    std::unordered_map<uint32_t, std::vector<D3D12_DESCRIPTOR_RANGE1>>& tables) {

        auto* shader_compiler = new spirv_cross::CompilerHLSL(shader.source);
        shader_compiler->set_hlsl_options(options);

        const spirv_cross::ShaderResources resources = shader_compiler->get_shader_resources();

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

        const auto& shader_hlsl = shader_compiler->compile();

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
            return {};
        }

        ComPtr<ID3D12ShaderReflection> shader_reflector;
        hr = D3DReflect(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), IID_PPV_ARGS(&shader_reflector));
        if(FAILED(hr)) {
            NOVA_LOG(ERROR) << "Could not create reflector, error code " << std::to_string(hr);
            return {};
        }

        D3D12_SHADER_DESC shader_desc;
        hr = shader_reflector->GetDesc(&shader_desc);
        if(FAILED(hr)) {
            NOVA_LOG(ERROR) << "Could not get shader description";
            return {};
        }

        std::unordered_map<std::string, D3D12_SHADER_INPUT_BIND_DESC> shader_inputs(shader_desc.BoundResources);
        // For each resource in the DX12 shader, find its set and binding in SPIR-V. Translate the sets and bindings into places in
        // descriptor tables
        for(uint32_t i = 0; i < shader_desc.BoundResources; i++) {
            D3D12_SHADER_INPUT_BIND_DESC bind_desc;
            hr = shader_reflector->GetResourceBindingDesc(i, &bind_desc);
            if(FAILED(hr)) {
                NOVA_LOG(ERROR) << "Could not get description for bind point " << std::to_string(i);
                return {};
            }

            D3D12_DESCRIPTOR_RANGE_TYPE descriptor_type = {};
            spirv_cross::Resource spirv_resource = {};
            uint32_t set = 0;

            switch(bind_desc.Type) {
                case D3D_SIT_CBUFFER:
                    descriptor_type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                    spirv_resource = spirv_uniform_buffers.at(bind_desc.Name);
                    set = shader_compiler->get_decoration(spirv_resource.id, spv::DecorationDescriptorSet);
                    add_resource_to_descriptor_table(descriptor_type, bind_desc, set, tables);
                    break;

                case D3D_SIT_TEXTURE:
                case D3D_SIT_TBUFFER:
                    descriptor_type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                    spirv_resource = spirv_sampled_images.at(bind_desc.Name);
                    set = shader_compiler->get_decoration(spirv_resource.id, spv::DecorationDescriptorSet);
                    add_resource_to_descriptor_table(descriptor_type, bind_desc, set, tables);

                    // Also add a descriptor table entry for the sampler
                    add_resource_to_descriptor_table(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, bind_desc, set, tables);
                    break;

                case D3D_SIT_SAMPLER:
                    descriptor_type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                    break;

                default:
                    descriptor_type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                    break;
            }
        }

        return shader_blob;
    }

    void add_resource_to_descriptor_table(const D3D12_DESCRIPTOR_RANGE_TYPE descriptor_type,
                                          const D3D12_SHADER_INPUT_BIND_DESC& bind_desc,
                                          const uint32_t set,
                                          std::unordered_map<uint32_t, std::vector<D3D12_DESCRIPTOR_RANGE1>>& tables) {
        D3D12_DESCRIPTOR_RANGE1 range = {};
        range.BaseShaderRegister = bind_desc.BindPoint;
        range.RegisterSpace = bind_desc.Space;
        range.NumDescriptors = 1;
        range.RangeType = descriptor_type;
        range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        tables[set].push_back(range);
    }
} // namespace nova::renderer::rhi
