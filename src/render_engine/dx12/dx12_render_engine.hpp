#pragma once

#include <memory>

// TODO: Don't always use mallocator
#include "nova_renderer/memory/mallocator.hpp"
#include "nova_renderer/rhi/render_engine.hpp"

#pragma warning(push, 0)
#include <d3d12.h>
#include <d3d12shader.h>
#include <dxgi1_4.h>
#include <spirv_hlsl.hpp>
#include <wrl.h>
#pragma warning(pop)

#include "dx12_swapchain.hpp"

// Make sure we don't pollute includees with windows.h problems
// TODO: Convert to modules and watch this problem _finally_ start to go away
#ifdef ERROR
#undef ERROR
#endif

namespace nova::renderer::rhi {
    /*!
     * \brief D3D12-specific capabilities
     */
    struct D3D12DeviceCapabilities {
        D3D_FEATURE_LEVEL feature_level;
        bool supports_renderpasses;
    };

    /*!
     * \brief D3D12 implementation of a render engine
     */
    class D3D12RenderEngine final : public RenderEngine {
    public:
        D3D12RenderEngine(NovaSettingsAccessManager& settings, const std::shared_ptr<NovaWindow>& window);

        D3D12RenderEngine(D3D12RenderEngine&& old) noexcept = delete;
        D3D12RenderEngine& operator=(D3D12RenderEngine&& old) noexcept = delete;

        D3D12RenderEngine(const D3D12RenderEngine& other) = delete;
        D3D12RenderEngine& operator=(const D3D12RenderEngine& other) = delete;

        virtual ~D3D12RenderEngine() = default;

        // Inherited via render_engine
        void set_num_renderpasses(uint32_t num_renderpasses) override;

        ntl::Result<DeviceMemory*> allocate_device_memory(uint64_t size, MemoryUsage type, ObjectType allowed_objects) override;

        ntl::Result<Renderpass*> create_renderpass(const shaderpack::RenderPassCreateInfo& data,
                                                   const glm::uvec2& framebuffer_size) override;

        Framebuffer* create_framebuffer(const Renderpass* renderpass,
                                        const std::vector<Image*>& color_attachments,
                                        const std::optional<Image*> depth_attachment,
                                        const glm::uvec2& framebuffer_size) override;

        ntl::Result<PipelineInterface*> create_pipeline_interface(
            const std::unordered_map<std::string, ResourceBindingDescription>& bindings,
            const std::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
            const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture) override;

        DescriptorPool* create_descriptor_pool(uint32_t num_sampled_images, uint32_t num_samplers, uint32_t num_uniform_buffers) override;

        /*!
         * \brief Creates all the descriptor sets that are needed for this pipeline interface
         *
         * Currently I create a separate descriptor heap for each descriptor set. This is fairly easy and closely
         * mimics what Vulkan does. However, I have no idea at all about how performant this is. I've heard that using
         * a single descriptor heap is noticeably better, but I've also heard that this only applies to XBox. Further
         * research and testing is needed to resolve this
         */
        std::vector<DescriptorSet*> create_descriptor_sets(const PipelineInterface* pipeline_interface, DescriptorPool* pool) override;

        void update_descriptor_sets(std::vector<DescriptorSetWrite>& writes) override;

        ntl::Result<Pipeline*> create_pipeline(PipelineInterface* pipeline_interface, const shaderpack::PipelineCreateInfo& data) override;

        Buffer* create_buffer(const BufferCreateInfo& info, DeviceMemoryResource& memory) override;

        void write_data_to_buffer(const void* data, uint64_t num_bytes, uint64_t offset, const Buffer* buffer) override;

        Image* create_image(const shaderpack::TextureCreateInfo& info) override;

        Semaphore* create_semaphore() override;

        std::vector<Semaphore*> create_semaphores(uint32_t num_semaphores) override;

        Fence* create_fence(bool signaled = false) override;

        std::vector<Fence*> create_fences(uint32_t num_fences, bool signaled = false) override;

        void wait_for_fences(std::vector<Fence*> fences) override;

        void reset_fences(const std::vector<Fence*>& fences) override;

        void destroy_renderpass(Renderpass* pass) override;

        void destroy_framebuffer(Framebuffer* framebuffer) override;

        void destroy_pipeline_interface(PipelineInterface* pipeline_interface) override;

        void destroy_pipeline(Pipeline* pipeline) override;

        void destroy_texture(Image* resource) override;

        void destroy_semaphores(std::vector<Semaphore*>& semaphores) override;

        void destroy_fences(std::vector<Fence*>& fences) override;

        CommandList* get_command_list(uint32_t thread_idx, QueueType needed_queue_type, CommandList::Level level) override;

        void submit_command_list(CommandList* cmds,
                                 QueueType queue,
                                 Fence* fence_to_signal = nullptr,
                                 const std::vector<Semaphore*>& wait_semaphores = {},
                                 const std::vector<Semaphore*>& signal_semaphores = {}) override;

        void create_swapchain(uint32_t num_frames);

    private:
        D3D12DeviceCapabilities d3d12_capabilities;

        // TODO: don't always use mallocator
        bvestl::polyalloc::Mallocator mallocator;

        Microsoft::WRL::ComPtr<IDXGIFactory4> dxgi_factory;

        Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter;
        Microsoft::WRL::ComPtr<ID3D12Device> device;
        Microsoft::WRL::ComPtr<ID3D12Device4> device4;

        Microsoft::WRL::ComPtr<ID3D12CommandQueue> direct_command_queue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> compute_command_queue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> copy_command_queue;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap;
        uint32_t next_rtv_descriptor_index = 0;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsv_descriptor_heap;
        uint32_t next_dsv_descriptor_index = 0;

        uint32_t rtv_descriptor_size = 0;
        uint32_t dsv_descriptor_size = 0;
        uint32_t cbv_srv_uav_descriptor_size = 0;

        DXGI_QUERY_VIDEO_MEMORY_INFO local_info;
        DXGI_QUERY_VIDEO_MEMORY_INFO non_local_info;

        /*!
         * \brief The index in the vector is the thread index
         */
        std::vector<std::unordered_map<D3D12_COMMAND_LIST_TYPE, Microsoft::WRL::ComPtr<ID3D12CommandAllocator>>> command_allocators;

#pragma region Initialization
        void create_device();

        void create_queues();

        /*!
         * \brief Creates the pre-thread command allocators that we use at runtime
         */
        void create_command_allocators();

        /*!
         * \brief Sets up a few things to make the debugging experience much nicer
         *
         * - Enables breakpoints on errors and corruption messages
         * - Will tell the debug layer to output to stdout whenever I figure out how to do that
         */
        void setup_debug_output();

        /*!
         * \brief Queries the hardware for its capabilities and limits
         */
        void determine_device_capabilities();
#pragma endregion

#pragma region Helpers
        Microsoft::WRL::ComPtr<ID3D12InfoQueue> info_queue = nullptr;
#pragma endregion
    };

    Microsoft::WRL::ComPtr<ID3DBlob> compile_shader(const shaderpack::ShaderSource& shader,
                                                    const std::string& target,
                                                    const spirv_cross::CompilerHLSL::Options& options,
                                                    std::unordered_map<uint32_t, std::vector<D3D12_DESCRIPTOR_RANGE1>>& tables);

    void add_resource_to_descriptor_table(D3D12_DESCRIPTOR_RANGE_TYPE descriptor_type,
                                          const D3D12_SHADER_INPUT_BIND_DESC& bind_desc,
                                          uint32_t set,
                                          std::unordered_map<uint32_t, std::vector<D3D12_DESCRIPTOR_RANGE1>>& tables);
} // namespace nova::renderer::rhi
