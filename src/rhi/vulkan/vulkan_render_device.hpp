#pragma once

#include "nova_renderer/rhi/render_device.hpp"

#include "vk_structs.hpp"
#include "vulkan_swapchain.hpp"

namespace nova::renderer::rhi {
    struct VulkanMemoryHeap : VkMemoryHeap {
        VkDeviceSize amount_allocated = 0;
    };

    struct VulkanDeviceInfo {
        uint64_t max_uniform_buffer_size = 0;
    };

    /*!
     * \brief Vulkan implementation of a render engine
     */
    class VulkanRenderDevice final : public RenderDevice {
    public:
        // Global Vulkan objects
        VkInstance instance;

        VkDevice device;

        VkSurfaceKHR surface{};

        uint32_t graphics_family_index;
        uint32_t compute_family_index;
        uint32_t transfer_family_index;

        VkQueue graphics_queue;
        VkQueue compute_queue;
        VkQueue copy_queue;

        // Info about the hardware
        VulkanGpuInfo gpu;

        // Debugging things
        PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = nullptr;
        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;

        VulkanRenderDevice(NovaSettingsAccessManager& settings,
                           NovaWindow& window,
                           mem::AllocatorHandle<>& allocator);

        VulkanRenderDevice(VulkanRenderDevice&& old) noexcept = delete;
        VulkanRenderDevice& operator=(VulkanRenderDevice&& old) noexcept = delete;

        VulkanRenderDevice(const VulkanRenderDevice& other) = delete;
        VulkanRenderDevice& operator=(const VulkanRenderDevice& other) = delete;

        ~VulkanRenderDevice() = default;

#pragma region Render engine interface
        void set_num_renderpasses(uint32_t num_renderpasses) override;

        ntl::Result<DeviceMemory*> allocate_device_memory(mem::Bytes size,
                                                          MemoryUsage usage,
                                                          ObjectType allowed_objects,
                                                          mem::AllocatorHandle<>& allocator) override;

        ntl::Result<Renderpass*> create_renderpass(const shaderpack::RenderPassCreateInfo& data,
                                                   const glm::uvec2& framebuffer_size,
                                                   mem::AllocatorHandle<>& allocator) override;

        Framebuffer* create_framebuffer(const Renderpass* renderpass,
                                        const std::pmr::vector<Image*>& color_attachments,
                                        const std::optional<Image*> depth_attachment,
                                        const glm::uvec2& framebuffer_size,
                                        mem::AllocatorHandle<>& allocator) override;

        ntl::Result<PipelineInterface*> create_pipeline_interface(
            const std::unordered_map<std::string, ResourceBindingDescription>& bindings,
            const std::pmr::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
            const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture,
            mem::AllocatorHandle<>& allocator) override;

        DescriptorPool* create_descriptor_pool(uint32_t num_sampled_images,
                                               uint32_t num_samplers,
                                               uint32_t num_uniform_buffers,
                                               mem::AllocatorHandle<>& allocator) override;

        std::pmr::vector<DescriptorSet*> create_descriptor_sets(const PipelineInterface* pipeline_interface,
                                                                DescriptorPool* pool,
                                                                mem::AllocatorHandle<>& allocator) override;

        void update_descriptor_sets(std::pmr::vector<DescriptorSetWrite>& writes) override;

        ntl::Result<Pipeline*> create_pipeline(PipelineInterface* pipeline_interface,
                                               const shaderpack::PipelineCreateInfo& data,
                                               mem::AllocatorHandle<>& allocator) override;

        Buffer* create_buffer(const BufferCreateInfo& info, DeviceMemoryResource& memory, mem::AllocatorHandle<>& allocator) override;

        void write_data_to_buffer(const void* data, mem::Bytes num_bytes, mem::Bytes offset, const Buffer* buffer) override;

        Image* create_image(const shaderpack::TextureCreateInfo& info, mem::AllocatorHandle<>& allocator) override;

        Semaphore* create_semaphore(mem::AllocatorHandle<>& allocator) override;

        std::pmr::vector<Semaphore*> create_semaphores(uint32_t num_semaphores, mem::AllocatorHandle<>& allocator) override;

        Fence* create_fence(mem::AllocatorHandle<>& allocator, bool signaled = false) override;

        std::pmr::vector<Fence*> create_fences(mem::AllocatorHandle<>& allocator, uint32_t num_fences, bool signaled = false) override;

        void wait_for_fences(std::pmr::vector<Fence*> fences) override;

        void reset_fences(const std::pmr::vector<Fence*>& fences) override;

        void destroy_renderpass(Renderpass* pass, mem::AllocatorHandle<>& allocator) override;

        void destroy_framebuffer(Framebuffer* framebuffer, mem::AllocatorHandle<>& allocator) override;

        void destroy_pipeline_interface(PipelineInterface* pipeline_interface, mem::AllocatorHandle<>& allocator) override;

        void destroy_pipeline(Pipeline* pipeline, mem::AllocatorHandle<>& allocator) override;

        void destroy_texture(Image* resource, mem::AllocatorHandle<>& allocator) override;

        void destroy_semaphores(std::pmr::vector<Semaphore*>& semaphores, mem::AllocatorHandle<>& allocator) override;

        void destroy_fences(const std::pmr::vector<Fence*>& fences, mem::AllocatorHandle<>& allocator) override;

        CommandList* create_command_list(mem::AllocatorHandle<>& allocator,
                                         uint32_t thread_idx,
                                         QueueType needed_queue_type,
                                         CommandList::Level level) override;

        void submit_command_list(CommandList* cmds,
                                 QueueType queue,
                                 Fence* fence_to_signal = nullptr,
                                 const std::pmr::vector<Semaphore*>& wait_semaphores = {},
                                 const std::pmr::vector<Semaphore*>& signal_semaphores = {}) override;
#pragma endregion

        [[nodiscard]] uint32_t get_queue_family_index(QueueType type) const;

    protected:
        void create_surface();

    private:
        VulkanDeviceInfo vk_info;

        /*!
         * The index in the vector is the thread index, the key in the map is the queue family index
         */
        std::pmr::vector<std::pmr::unordered_map<uint32_t, VkCommandPool>> command_pools_by_thread_idx;

        /*!
         * \brief Keeps track of how much has been allocated from each heap
         *
         * In the same order as VulkanGpuInfo::memory_properties::memoryHeaps
         */
        std::pmr::vector<uint32_t> heap_usages;

        /*!
         * \brief Map from HOST_VISIBLE memory allocations to the memory address they're mapped to
         *
         * The Vulkan render engine maps memory when it's allocated, if the memory is for a uniform or a staging
         * buffer.
         */
        std::unordered_map<VkDeviceMemory, void*> heap_mappings;

#pragma region Initialization
        std::pmr::vector<const char*> enabled_layer_names;

        void create_instance();

        void enable_debug_output();

        /*!
         * \brief Copies device information, such as hardware limits and memory architecture, to the API-agnostic DeviceInfo struct
         *
         * This allows things outside of a render engine to make decisions based on GPU information
         */
        void save_device_info();

        void create_device_and_queues();

        bool does_device_support_extensions(VkPhysicalDevice device, const std::pmr::vector<char*>& required_device_extensions);

        void create_swapchain();

        void create_per_thread_command_pools();

        [[nodiscard]] std::pmr::unordered_map<uint32_t, VkCommandPool> make_new_command_pools() const;
#pragma endregion

#pragma region Helpers
        enum class MemorySearchMode { Exact, Fuzzy };

        /*!
         * \brief Finds the index of the memory type with the desired flags
         *
         * \param[in] search_flags Flags to search for
         * \param[in] search_mode What search mode to use. If search_mode is MemorySearchMode::Exact, this method will only return the index
         * of a memory type whose flags exactly match search_flags. If search_mode is MemorySearchMode::Fuzzy, this method will return the
         * index of the first memory type whose flags include search_flags
         *
         * \return The index of the memory type with the desired flags, or VK_MAX_MEMORY_TYPES if no memory types match the given flags
         */
        [[nodiscard]] uint32_t find_memory_type_with_flags(uint32_t search_flags, MemorySearchMode search_mode = MemorySearchMode::Fuzzy);

        [[nodiscard]] std::optional<VkShaderModule> create_shader_module(const std::pmr::vector<uint32_t>& spirv) const;

        [[nodiscard]] std::pmr::vector<VkDescriptorSetLayout> create_descriptor_set_layouts(
            const std::unordered_map<std::string, ResourceBindingDescription>& all_bindings, mem::AllocatorHandle<>& allocator) const;

        /*!
         * \brief Gets the image view associated with the given image
         *
         * Nova simplifies things a lot and only has one image view for each image. This is maintained within the
         * Vulkan backend, since neither DX12 nor OpenGL have a direct equivalent. I may or may not emulate image views
         * for those APIs if the demand is there, but idk
         *
         * The method checks an internal hash map. If there's already an image view for the given image then great,
         * otherwise one is created on-demand
         */
        [[nodiscard]] static VkImageView image_view_for_image(const Image* image);

        [[nodiscard]] static VkCommandBufferLevel to_vk_command_buffer_level(CommandList::Level level);

        [[nodiscard]] static std::tuple<std::pmr::vector<VkVertexInputAttributeDescription>,
                                        std::pmr::vector<VkVertexInputBindingDescription>>
        get_input_assembler_setup(const std::pmr::vector<VertexField>& vertex_fields);
#pragma endregion

#pragma region Debugging
        VkDebugUtilsMessengerEXT debug_callback{};

        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                                    VkDebugUtilsMessageTypeFlagsEXT message_types,
                                                                    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                                    void* render_device);
#pragma endregion
    };
} // namespace nova::renderer::rhi