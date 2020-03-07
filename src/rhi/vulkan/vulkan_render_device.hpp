#pragma once

#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

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

    struct VulkanInputAssemblerLayout {
        rx::vector<VkVertexInputAttributeDescription> attributes;
        rx::vector<VkVertexInputBindingDescription> bindings;
    };

    /*!
     * \brief Vulkan implementation of a render engine
     */
    class VulkanRenderDevice final : public RenderDevice {
    public:
        VkAllocationCallbacks vk_internal_allocator;

        // Global Vulkan objects
        vkb::Instance instance;

        vkb::Device vkb_device;
        VkDevice device;

        VkSurfaceKHR surface{};

        uint32_t graphics_family_index = 0xFFFFFFFF;
        uint32_t compute_family_index = 0xFFFFFFFF;
        uint32_t transfer_family_index = 0xFFFFFFFF;

        VkQueue graphics_queue;
        VkQueue compute_queue;
        VkQueue copy_queue;

        // Info about the hardware
        VulkanGpuInfo gpu;

        // Debugging things
        PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = nullptr;
        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;

        VulkanRenderDevice(NovaSettingsAccessManager& settings, NovaWindow& window, rx::memory::allocator& allocator);

        VulkanRenderDevice(VulkanRenderDevice&& old) noexcept = delete;
        VulkanRenderDevice& operator=(VulkanRenderDevice&& old) noexcept = delete;

        VulkanRenderDevice(const VulkanRenderDevice& other) = delete;
        VulkanRenderDevice& operator=(const VulkanRenderDevice& other) = delete;

        ~VulkanRenderDevice() = default;

#pragma region Render engine interface
        void set_num_renderpasses(uint32_t num_renderpasses) override;

        ntl::Result<RhiDeviceMemory*> allocate_device_memory(mem::Bytes size,
                                                             MemoryUsage usage,
                                                             ObjectType allowed_objects,
                                                             rx::memory::allocator& allocator) override;

        ntl::Result<RhiRenderpass*> create_renderpass(const renderpack::RenderPassCreateInfo& data,
                                                      const glm::uvec2& framebuffer_size,
                                                      rx::memory::allocator& allocator) override;

        RhiFramebuffer* create_framebuffer(const RhiRenderpass* renderpass,
                                           const rx::vector<RhiImage*>& color_attachments,
                                           const rx::optional<RhiImage*> depth_attachment,
                                           const glm::uvec2& framebuffer_size,
                                           rx::memory::allocator& allocator) override;

        ntl::Result<RhiPipelineInterface*> create_pipeline_interface(const rx::map<rx::string, RhiResourceBindingDescription>& bindings,
                                                                     const rx::vector<renderpack::TextureAttachmentInfo>& color_attachments,
                                                                     const rx::optional<renderpack::TextureAttachmentInfo>& depth_texture,
                                                                     rx::memory::allocator& allocator) override;

        RhiDescriptorPool* create_descriptor_pool(const rx::map<DescriptorType, uint32_t>& descriptor_capacity,
                                                  rx::memory::allocator& allocator) override;

        rx::vector<RhiDescriptorSet*> create_descriptor_sets(const RhiPipelineInterface* pipeline_interface,
                                                             RhiDescriptorPool* pool,
                                                             rx::memory::allocator& allocator) override;

        void update_descriptor_sets(rx::vector<RhiDescriptorSetWrite>& writes) override;

        void reset_descriptor_pool(RhiDescriptorPool* pool) override;

        ntl::Result<RhiPipeline*> create_pipeline(RhiPipelineInterface* pipeline_interface,
                                                  const PipelineStateCreateInfo& data,
                                                  rx::memory::allocator& allocator) override;

        RhiBuffer* create_buffer(const RhiBufferCreateInfo& info, rx::memory::allocator& allocator) override;

        void write_data_to_buffer(const void* data, mem::Bytes num_bytes, mem::Bytes offset, const RhiBuffer* buffer) override;

        RhiSampler* create_sampler(const RhiSamplerCreateInfo& create_info, rx::memory::allocator& allocator) override;

        RhiImage* create_image(const renderpack::TextureCreateInfo& info, rx::memory::allocator& allocator) override;

        RhiSemaphore* create_semaphore(rx::memory::allocator& allocator) override;

        rx::vector<RhiSemaphore*> create_semaphores(uint32_t num_semaphores, rx::memory::allocator& allocator) override;

        RhiFence* create_fence(bool signaled, rx::memory::allocator& allocator) override;

        rx::vector<RhiFence*> create_fences(uint32_t num_fences, bool signaled, rx::memory::allocator& allocator) override;

        void wait_for_fences(rx::vector<RhiFence*> fences) override;

        void reset_fences(const rx::vector<RhiFence*>& fences) override;

        void destroy_renderpass(RhiRenderpass* pass, rx::memory::allocator& allocator) override;

        void destroy_framebuffer(RhiFramebuffer* framebuffer, rx::memory::allocator& allocator) override;

        void destroy_pipeline_interface(RhiPipelineInterface* pipeline_interface, rx::memory::allocator& allocator) override;

        void destroy_pipeline(RhiPipeline* pipeline, rx::memory::allocator& allocator) override;

        void destroy_texture(RhiImage* resource, rx::memory::allocator& allocator) override;

        void destroy_semaphores(rx::vector<RhiSemaphore*>& semaphores, rx::memory::allocator& allocator) override;

        void destroy_fences(const rx::vector<RhiFence*>& fences, rx::memory::allocator& allocator) override;

        RhiRenderCommandList* create_command_list(uint32_t thread_idx,
                                                  QueueType needed_queue_type,
                                                  RhiRenderCommandList::Level level,
                                                  rx::memory::allocator& allocator) override;

        void submit_command_list(RhiRenderCommandList* cmds,
                                 QueueType queue,
                                 RhiFence* fence_to_signal = nullptr,
                                 const rx::vector<RhiSemaphore*>& wait_semaphores = {},
                                 const rx::vector<RhiSemaphore*>& signal_semaphores = {}) override;
#pragma endregion

        [[nodiscard]] uint32_t get_queue_family_index(QueueType type) const;

    protected:
        void create_surface();

    private:
        VulkanDeviceInfo vk_info;

        VmaAllocator vma;

        /*!
         * The index in the vector is the thread index, the key in the map is the queue family index
         */
        rx::vector<rx::map<uint32_t, VkCommandPool>> command_pools_by_thread_idx;

        /*!
         * \brief Keeps track of how much has been allocated from each heap
         *
         * In the same order as VulkanGpuInfo::memory_properties::memoryHeaps
         */
        rx::vector<uint32_t> heap_usages;

        /*!
         * \brief Map from HOST_VISIBLE memory allocations to the memory address they're mapped to
         *
         * The Vulkan render engine maps memory when it's allocated, if the memory is for a uniform or a staging
         * buffer.
         */
        rx::map<VkDeviceMemory, void*> heap_mappings;

#pragma region Initialization
        rx::vector<const char*> enabled_layer_names;

        void create_instance();

        /*!
         * \brief Copies device information, such as hardware limits and memory architecture, to the API-agnostic DeviceInfo struct
         *
         * This allows things outside of a render engine to make decisions based on GPU information
         */
        void save_device_info();

        void initialize_vma();

        void create_device_and_queues();

        bool does_device_support_extensions(VkPhysicalDevice device, const rx::vector<char*>& required_device_extensions);

        void create_swapchain();

        void create_per_thread_command_pools();

        [[nodiscard]] rx::map<uint32_t, VkCommandPool> make_new_command_pools() const;
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
        [[nodiscard]] uint32_t find_memory_type_with_flags(uint32_t search_flags,
                                                           MemorySearchMode search_mode = MemorySearchMode::Fuzzy) const;

        [[nodiscard]] rx::optional<VkShaderModule> create_shader_module(const rx::vector<uint32_t>& spirv) const;

        [[nodiscard]] rx::vector<VkDescriptorSetLayout> create_descriptor_set_layouts(
            const rx::map<rx::string, RhiResourceBindingDescription>& all_bindings,
            rx::vector<uint32_t>& variable_descriptor_counts,
            rx::memory::allocator& allocator) const;

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
        [[nodiscard]] static VkImageView image_view_for_image(const RhiImage* image);

        [[nodiscard]] static VkCommandBufferLevel to_vk_command_buffer_level(RhiRenderCommandList::Level level);

        [[nodiscard]] static VulkanInputAssemblerLayout get_input_assembler_setup(const rx::vector<RhiVertexField>& vertex_fields);
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
