#pragma once

// Fix WinAPI cause Rex broke it
#define interface struct

#include <D3D12MemAlloc.h>
#include <d3d12.h>
#include <d3d12shader.h>
#include <dxc/dxcapi.h>
#include <dxgi.h>
#include <spirv_hlsl.hpp>
#include <wrl/client.h>

#include "d3d12_resource_binder.hpp"
#include "nova_renderer/rhi/render_device.hpp"

#include "d3d12_structs.hpp"
#include "descriptor_allocator.hpp"

// Fix WinAPI cause Rex broke it
#define interface struct

namespace nova::renderer::rhi {
    class D3D12RenderDevice final : public RenderDevice {
    public:
        D3D12MA::Allocator* dma_allocator = nullptr;

        D3D12RenderDevice(NovaSettingsAccessManager& settings, NovaWindow& window, rx::memory::allocator& allocator);

        ~D3D12RenderDevice() override;

#pragma region RenderDevice
        void set_num_renderpasses(uint32_t num_renderpasses) override;

        [[nodiscard]] rx::ptr<RhiRenderpass> create_renderpass(const renderpack::RenderPassCreateInfo& data,
                                                               const glm::uvec2& framebuffer_size,
                                                               rx::memory::allocator& allocator) override;

        [[nodiscard]] rx::ptr<RhiFramebuffer> create_framebuffer(const RhiRenderpass& renderpass,
                                                                 const rx::vector<RhiImage*>& color_attachments,
                                                                 const rx::optional<RhiImage*> depth_attachment,
                                                                 const glm::uvec2& framebuffer_size,
                                                                 rx::memory::allocator& allocator) override;

        [[nodiscard]] rx::ptr<RhiPipeline> create_surface_pipeline(const RhiGraphicsPipelineState& pipeline_state,
                                                                   rx::memory::allocator& allocator) override;

        [[nodiscard]] rx::ptr<RhiPipeline> create_global_pipeline(const RhiGraphicsPipelineState& pipeline_state,
                                                                  rx::memory::allocator& allocator) override;

        [[nodiscard]] rx::ptr<RhiResourceBinder> create_resource_binder_for_pipeline(const RhiPipeline& pipeline,
                                                                                     rx::memory::allocator& allocator) override;

        RhiResourceBinder* get_material_resource_binder() override;

        [[nodiscard]] rx::ptr<RhiBuffer> create_buffer(const RhiBufferCreateInfo& info, rx::memory::allocator& allocator) override;

        void write_data_to_buffer(const void* data, mem::Bytes num_bytes, const RhiBuffer& buffer) override;

        [[nodiscard]] rx::ptr<RhiSampler> create_sampler(const RhiSamplerCreateInfo& create_info,
                                                         rx::memory::allocator& allocator) override;

        [[nodiscard]] rx::ptr<RhiImage> create_image(const renderpack::TextureCreateInfo& info, rx::memory::allocator& allocator) override;

        [[nodiscard]] rx::ptr<RhiSemaphore> create_semaphore(rx::memory::allocator& allocator) override;

        [[nodiscard]] rx::vector<rx::ptr<RhiSemaphore>> create_semaphores(uint32_t num_semaphores,
                                                                          rx::memory::allocator& allocator) override;

        [[nodiscard]] rx::ptr<RhiFence> create_fence(bool signaled, rx::memory::allocator& allocator) override;

        [[nodiscard]] rx::vector<rx::ptr<RhiFence>> create_fences(uint32_t num_fences,
                                                                  bool signaled,
                                                                  rx::memory::allocator& allocator) override;

        void wait_for_fences(const rx::vector<RhiFence*>& fences) override;

        void reset_fences(const rx::vector<RhiFence*>& fences) override;

        void destroy_renderpass(rx::ptr<RhiRenderpass> pass) override;

        void destroy_framebuffer(rx::ptr<RhiFramebuffer> framebuffer) override;

        void destroy_texture(rx::ptr<RhiImage> resource) override;

        void destroy_semaphores(rx::vector<rx::ptr<RhiSemaphore>>& semaphores) override;

        void destroy_fences(const rx::vector<rx::ptr<RhiFence>>& fences) override;

        rx::ptr<RhiRenderCommandList> create_command_list(uint32_t thread_idx,
                                                          QueueType needed_queue_type,
                                                          RhiRenderCommandList::Level level,
                                                          rx::memory::allocator& allocator) override;

        void submit_command_list(rx::ptr<RhiRenderCommandList> cmds,
                                 QueueType queue,
                                 rx::optional<RhiFence> fence_to_signal,
                                 const rx::vector<RhiSemaphore*>& wait_semaphores,
                                 const rx::vector<RhiSemaphore*>& signal_semaphores) override;

        void end_frame(FrameContext& ctx) override;
#pragma endregion

        void wait_for_single_fence(const D3D12Fence& fence);

        [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12PipelineState> compile_pso(const D3D12Pipeline& pipeline_info, D3D12Renderpass& current_renderpass);

    private:
        Microsoft::WRL::ComPtr<IDXGIFactory> factory;

        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;

        Microsoft::WRL::ComPtr<ID3D12Device> device;
        Microsoft::WRL::ComPtr<ID3D12Device1> device1;

        Microsoft::WRL::ComPtr<ID3D12Debug> debug_controller;

        Microsoft::WRL::ComPtr<ID3D12InfoQueue> info_queue;

        Microsoft::WRL::ComPtr<ID3D12CommandQueue> graphics_queue;

        Microsoft::WRL::ComPtr<ID3D12CommandQueue> dma_queue;

        rx::vector<spirv_cross::HLSLResourceBinding> standard_hlsl_bindings;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> standard_root_signature;

        rx::ptr<DescriptorAllocator> shader_resource_descriptors;
        rx::ptr<DescriptorAllocator> render_target_descriptors;
        rx::ptr<DescriptorAllocator> depth_stencil_descriptors;

        Microsoft::WRL::ComPtr<IDxcLibrary> dxc_library;

        Microsoft::WRL::ComPtr<IDxcCompiler> dxc_compiler;

        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> direct_command_allocator;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> copy_command_allocator;

        /*!
         * \brief Indicates whether this device has a Unified Memory Architecture
         *
         * UMA devices don't need to use a transfer queue to upload data, they can map a pointer directly to all resources
         */
        bool is_uma = false;

        /*!
         * \brief Indicates the level of hardware and driver support for render passes
         *
         * Tier 0 - No support, don't use renderpasses
         * Tier 1 - render targets and depth/stencil writes should use renderpasses, but UAV writes are not supported
         * Tire 2 - render targets, depth/stencil, and UAV writes should use renderpasses
         */
        D3D12_RENDER_PASS_TIER render_pass_tier = D3D12_RENDER_PASS_TIER_0;

        /*!
         * \brief Indicates support the the DXR API
         *
         * If this is `false`, the user will be unable to use any DXR shaderpacks
         */
        bool has_raytracing = false;

        mem::Bytes max_cbv_size{65536};

        rx::vector<HANDLE> fence_wait_events;

        rx::ptr<D3D12ResourceBinder> material_resource_binder;

#pragma region Initialization
        void enable_validation_layer();

        void initialize_dxgi();

        void select_adapter();

        void create_queues();

        void create_swapchain(HWND hwnd);

        void create_command_allocators();

        void create_standard_root_signature();

        [[nodiscard]] rx::ptr<DescriptorAllocator> create_descriptor_allocator(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT num_descriptors) const;

        void create_descriptor_heaps();

        void initialize_dma();

        void initialize_standard_resource_binding_mappings();

        void create_shader_compiler();

        void create_material_resource_binder();
#pragma endregion

#pragma region Helpers
        [[nodiscard]] Microsoft::WRL::ComPtr<IDxcBlob> compile_spirv_to_dxil(const rx::vector<uint32_t>& spirv,
                                                                             LPCWSTR target_profile,
                                                                             const rx::string& pipeline_name);

        using RootSignatureWithDescriptors = rx::pair<rx::map<rx::string, D3D12_CPU_DESCRIPTOR_HANDLE>,
                                                      Microsoft::WRL::ComPtr<ID3D12RootSignature>>;

        [[nodiscard]] RootSignatureWithDescriptors create_root_signature(const rx::map<rx::string, D3D12_SHADER_INPUT_BIND_DESC>& bindings,
                                                                         rx::memory::allocator& allocator);

        [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12RootSignature> compile_root_signature(
            const D3D12_ROOT_SIGNATURE_DESC& root_signature_desc) const;

        [[nodiscard]] HANDLE get_next_event();
#pragma endregion
    };
} // namespace nova::renderer::rhi
