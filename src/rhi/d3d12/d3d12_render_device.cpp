#include "d3d12_render_device.hpp"

#include <rx/core/log.h>

#include "nova_renderer/constants.hpp"
#include "nova_renderer/exception.hpp"

#include "d3d12_structs.hpp"
#include "d3dx12.h"

using namespace Microsoft::WRL;

namespace nova::renderer ::rhi {
    RX_LOG("D3D12RenderDevice", logger);

    D3D12RenderDevice::D3D12RenderDevice(NovaSettingsAccessManager& settings, NovaWindow& window, rx::memory::allocator& allocator)
        : RenderDevice(settings, window, allocator) {

        if(settings->debug.enabled && settings->debug.enable_validation_layers) {
            enable_validation_layer();
        }

        initialize_dxgi();

        select_adapter();

        create_queues();

        create_standard_root_signature();

        create_descriptor_heaps();

        initialize_dma();
    }

    D3D12RenderDevice::~D3D12RenderDevice() { dma_allocator->Release(); }

    void D3D12RenderDevice::set_num_renderpasses(uint32_t /* num_renderpasses */) {
        // Don't think we actually need to do anything
    }

    ntl::Result<RhiRenderpass*> D3D12RenderDevice::create_renderpass(const renderpack::RenderPassCreateInfo& data,
                                                                     const glm::uvec2& /* framebuffer_size */,
                                                                     rx::memory::allocator& allocator) {
        auto* renderpass = allocator.create<D3D12RenderPass>();
        renderpass->render_target_descriptions = rx::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC>{&allocator};

        if(!data.output_buffers.is_empty()) {
            renderpass->flags |= D3D12_RENDER_PASS_FLAG_ALLOW_UAV_WRITES;
        }

        return ntl::Result<RhiRenderpass*>{renderpass};
    }

    RhiFramebuffer* D3D12RenderDevice::create_framebuffer(const RhiRenderpass* /* renderpass */,
                                                          const rx::vector<RhiImage*>& color_attachments,
                                                          const rx::optional<RhiImage*> depth_attachment,
                                                          const glm::uvec2& framebuffer_size,
                                                          rx::memory::allocator& allocator) {
        auto* framebuffer = allocator.create<D3D12Framebuffer>();
        framebuffer->size = framebuffer_size;

        rx::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtv_descriptors{&allocator};
        color_attachments.each_fwd([&](const RhiImage* image) {
            auto* d3d12_image = static_cast<const D3D12Image*>(image);
            const auto descriptor_handle = render_target_descriptors->get_next_free_descriptor();

            D3D12_RENDER_TARGET_VIEW_DESC desc{};
            desc.Format = d3d12_image->format;
            desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

            device->CreateRenderTargetView(d3d12_image->resource.Get(), &desc, descriptor_handle);

            rtv_descriptors.push_back(descriptor_handle);
        });
        framebuffer->num_attachments = static_cast<uint32_t>(framebuffer->render_target_descriptors.size());

        if(depth_attachment) {
            auto* d3d12_depth = static_cast<const D3D12Image*>(*depth_attachment);
            const auto descriptor_handle = depth_stencil_descriptors->get_next_free_descriptor();

            D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
            dsv_desc.Format = d3d12_depth->format;
            dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

            device->CreateDepthStencilView(d3d12_depth->resource.Get(), &dsv_desc, descriptor_handle);

            framebuffer->depth_stencil_descriptor = descriptor_handle;
        }

        return framebuffer;
    }

    void D3D12RenderDevice::enable_validation_layer() {
        const auto res = D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
        if(SUCCEEDED(res)) {
            debug_controller->EnableDebugLayer();

        } else {
            logger->error("Could not enable the D3D12 validation layer");
        }
    }

    void D3D12RenderDevice::initialize_dxgi() { CreateDXGIFactory(IID_PPV_ARGS(&factory)); }

    void D3D12RenderDevice::select_adapter() {
        // We want an adapter:
        // - Not integrated, if possible

        // TODO: Figure out how to get the number of adapters in advance
        rx::vector<ComPtr<IDXGIAdapter>> adapters{&internal_allocator};

        UINT adapter_idx = 0;
        ComPtr<IDXGIAdapter> cur_adapter;
        while(factory->EnumAdapters(adapter_idx, &cur_adapter) != DXGI_ERROR_NOT_FOUND) {
            adapters.push_back(cur_adapter);
            adapter_idx++;
        }

        // TODO: Score adapters based on things like supported feature level and available vram

        adapters.each_fwd([&](const ComPtr<IDXGIAdapter>& adapter) {
            DXGI_ADAPTER_DESC desc;
            adapter->GetDesc(&desc);

            if(desc.VendorId == INTEL_PCI_VENDOR_ID && adapters.size() > 1) {
                // Prefer something other then the Intel GPU
                return RX_ITERATION_CONTINUE;
            }

            ComPtr<ID3D12Device> try_device;
            auto res = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&try_device));
            if(SUCCEEDED(res)) {
                // check the features we care about
                D3D12_FEATURE_DATA_D3D12_OPTIONS d3d12_options;
                try_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &d3d12_options, sizeof(d3d12_options));
                if(d3d12_options.ResourceBindingTier != D3D12_RESOURCE_BINDING_TIER_3) {
                    // Resource binding tier three means we can have partially bound descriptor array. Nova relies on partially bound
                    // descriptor arrays, so we need it
                    // Thus - if we find an adapter without full descriptor indexing support, we ignore it

                    return RX_ITERATION_CONTINUE;
                }

                device = try_device;

                // Save information about the device
                D3D12_FEATURE_DATA_ARCHITECTURE arch;
                res = device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &arch, sizeof(D3D12_FEATURE_DATA_ARCHITECTURE));
                if(SUCCEEDED(res)) {
                    is_uma = arch.CacheCoherentUMA;
                }

                D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5;
                res = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5));
                if(SUCCEEDED(res)) {
                    render_pass_tier = options5.RenderPassesTier;
                    has_raytracing = options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
                }

                if(settings->debug.enabled && settings->debug.enable_validation_layers) {
                    if(settings->debug.break_on_validation_errors) {
                        res = device->QueryInterface(IID_PPV_ARGS(&info_queue));
                        if(SUCCEEDED(res)) {
                            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
                            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
                        }
                    }
                }

                return RX_ITERATION_STOP;
            }

            return RX_ITERATION_CONTINUE;
        });

        if(!device) {
            throw Exception("Could not find a suitable D3D12 adapter");
        }
    }

    void D3D12RenderDevice::create_queues() {
        // One graphics queue and one optional DMA queue
        D3D12_COMMAND_QUEUE_DESC graphics_queue_desc{};
        graphics_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        auto result = device->CreateCommandQueue(&graphics_queue_desc, IID_PPV_ARGS(&graphics_queue));
        if(FAILED(result)) {
            throw Exception("Could not create graphics command queue");
        }

        if(!is_uma) {
            // No need to care about DMA on UMA cause we can just map everything
            D3D12_COMMAND_QUEUE_DESC dma_queue_desc{};
            dma_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
            result = device->CreateCommandQueue(&dma_queue_desc, IID_PPV_ARGS(&dma_queue));
            if(FAILED(result)) {
                logger->warning("Could not create a DMA queue on a non-UMA adapter, data transfers will have to use the graphics queue");
            }
        }
    }

    void D3D12RenderDevice::create_standard_root_signature() {
        rx::vector<CD3DX12_ROOT_PARAMETER> root_parameters{&internal_allocator, 4};

        // Root constants for material index and camera index
        root_parameters[0].InitAsConstants(2, 0);

        // Camera data buffer
        root_parameters[1].InitAsShaderResourceView(0);

        // Material data buffer
        root_parameters[2].InitAsShaderResourceView(1);

        // Textures array
        root_parameters[3].InitAsShaderResourceView(3, MAX_NUM_TEXTURES);

        rx::vector<D3D12_STATIC_SAMPLER_DESC> static_samplers{&internal_allocator, 3};

        // Point sampler
        auto& point_sampler_desc = static_samplers[0];
        point_sampler_desc.Filter = D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;

        auto& linear_sampler = static_samplers[1];
        linear_sampler.Filter = D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
        linear_sampler.RegisterSpace = 1;

        auto& trilinear_sampler = static_samplers[2];
        trilinear_sampler.Filter = D3D12_FILTER_ANISOTROPIC;
        trilinear_sampler.MaxAnisotropy = 8;
        trilinear_sampler.RegisterSpace = 2;

        D3D12_ROOT_SIGNATURE_DESC root_signature_desc{};
        root_signature_desc.NumParameters = static_cast<UINT>(root_parameters.size());
        root_signature_desc.pParameters = root_parameters.data();
        root_signature_desc.NumStaticSamplers = static_cast<UINT>(static_samplers.size());
        root_signature_desc.pStaticSamplers = static_samplers.data();

        ComPtr<ID3DBlob> root_signature_blob;
        ComPtr<ID3DBlob> error_blob;
        auto result = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &root_signature_blob, &error_blob);
        if(FAILED(result)) {
            const rx::string msg{&internal_allocator, reinterpret_cast<char*>(error_blob->GetBufferPointer()), error_blob->GetBufferSize()};
            throw Exception(rx::string::format("Could not create standard root signature: %s", msg));
        }

        result = device->CreateRootSignature(0,
                                             root_signature_blob->GetBufferPointer(),
                                             root_signature_blob->GetBufferSize(),
                                             IID_PPV_ARGS(&standard_root_signature));
        if(FAILED(result)) {
            throw Exception("Could not create root signature");
        }
    }

    rx::ptr<DescriptorAllocator> D3D12RenderDevice::create_descriptor_allocator(const D3D12_DESCRIPTOR_HEAP_TYPE type,
                                                                                const UINT num_descriptors) const {
        ComPtr<ID3D12DescriptorHeap> heap;

        D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
        heap_desc.Type = type;
        heap_desc.NumDescriptors = num_descriptors;
        heap_desc.Flags = (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE :
                                                                            D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
        device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&heap));
        const auto descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        return rx::make_ptr<DescriptorAllocator>(&internal_allocator, heap, descriptor_size, internal_allocator);
    }

    void D3D12RenderDevice::create_descriptor_heaps() {
        shader_resource_descriptors = create_descriptor_allocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 65536);
        render_target_descriptors = create_descriptor_allocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1024);
        depth_stencil_descriptors = create_descriptor_allocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 512);
    }

    void D3D12RenderDevice::initialize_dma() {
        D3D12MA::ALLOCATOR_DESC allocator_desc{};
        allocator_desc.pAdapter = adapter.Get();
        allocator_desc.pDevice = device.Get();
        const auto result = D3D12MA::CreateAllocator(&allocator_desc, &dma_allocator);
        if(FAILED(result)) {
            throw Exception("Could not initialize D3D12 Memory Allocator");
        }
    }
} // namespace nova::renderer::rhi
