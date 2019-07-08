/*!
 * \author ddubois
 * \date 03-Apr-19.
 */

#pragma once

#include <memory>

#include "nova_renderer/render_engine.hpp"

#include "vulkan_swapchain.hpp"

#include "vk_structs.hpp"
#include "../configuration.hpp"

namespace nova::renderer::rhi {
    struct VulkanMemoryHeap : VkMemoryHeap {
        VkDeviceSize amount_allocated = 0;
    };

    /*!
     * \brief Vulkan implementation of a render engine
     */
    class VulkanRenderEngine final : public RenderEngine {
    public:
        // Global Vulkan objects
        VkInstance instance;

        VkDevice device;
        VmaAllocator vma_allocator;

        VkSurfaceKHR surface{};

        uint32_t graphics_family_index;
        uint32_t compute_family_index;
        uint32_t transfer_family_index;

        VkQueue graphics_queue;
        VkQueue compute_queue;
        VkQueue copy_queue;

        // Info about the hardware
        VulkanGpuInfo gpu;

        VulkanRenderEngine(NovaSettings& settings);

        VulkanRenderEngine(VulkanRenderEngine&& old) noexcept = delete;
        VulkanRenderEngine& operator=(VulkanRenderEngine&& old) noexcept = delete;

        VulkanRenderEngine(const VulkanRenderEngine& other) = delete;
        VulkanRenderEngine& operator=(const VulkanRenderEngine& other) = delete;

        ~VulkanRenderEngine() = default;

#pragma region Render engine interface
        [[nodiscard]] std::shared_ptr<Window> get_window() const override final;

        void set_num_renderpasses(uint32_t num_renderpasses) override final;

        Result<DeviceMemory*> allocate_device_memory(uint64_t size, MemoryUsage usage, ObjectType allowed_objects) override final;

        Result<Renderpass*> create_renderpass(const shaderpack::RenderPassCreateInfo& data) override final;

        Framebuffer* create_framebuffer(const Renderpass* renderpass,
                                        const std::vector<Image*>& attachments,
                                        const glm::uvec2& framebuffer_size) override final;

        Result<PipelineInterface*> create_pipeline_interface(
            const std::unordered_map<std::string, ResourceBindingDescription>& bindings,
            const std::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
            const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture) override final;

        DescriptorPool* create_descriptor_pool(uint32_t num_sampled_images,
                                               uint32_t num_samplers,
                                               uint32_t num_uniform_buffers) override final;

        std::vector<DescriptorSet*> create_descriptor_sets(const PipelineInterface* pipeline_interface,
                                                           DescriptorPool* pool) override final;

        void update_descriptor_sets(std::vector<DescriptorSetWrite>& writes) override final;

        Result<Pipeline*> create_pipeline(PipelineInterface* pipeline_interface,
                                          const shaderpack::PipelineCreateInfo& data) override final;

        Buffer* create_buffer(const BufferCreateInfo& info) override final;

        void write_data_to_buffer(const void* data, const uint64_t num_bytes, uint64_t offset, const Buffer* buffer) override final;

        Image* create_texture(const shaderpack::TextureCreateInfo& info) override final;
        Semaphore* create_semaphore() override final;
        std::vector<Semaphore*> create_semaphores(uint32_t num_semaphores) override final;

        Fence* create_fence(bool signaled = false) override final;

        std::vector<Fence*> create_fences(uint32_t num_fences, bool signaled = false) override final;

        void wait_for_fences(const std::vector<Fence*> fences) override final;

        void destroy_renderpass(Renderpass* pass) override final;

        void destroy_framebuffer(const Framebuffer* framebuffer) override final;

        void destroy_pipeline(Pipeline* pipeline) override final;
        void destroy_texture(Image* resource) override final;
        void destroy_semaphores(const std::vector<Semaphore*>& semaphores) override final;
        void destroy_fences(const std::vector<Fence*>& fences) override final;

        Swapchain* get_swapchain() override final;
        
        CommandList* get_command_list(uint32_t thread_idx, QueueType needed_queue_type, CommandList::Level level) override final;

        void submit_command_list(CommandList* cmds,
                                 QueueType queue,
                                 Fence* fence_to_signal = nullptr,
                                 const std::vector<Semaphore*>& wait_semaphores = {},
                                 const std::vector<Semaphore*>& signal_semaphores = {}) override final;
#pragma endregion

        [[nodiscard]] uint32_t get_queue_family_index(QueueType type) const;

    protected:
        void open_window_and_create_surface(const NovaSettings::WindowOptions& options);

    private:
        std::unique_ptr<VulkanSwapchain> swapchain;

        /*!
         * The index in the vector is the thread index, the key in the map is the queue family index
         */
        std::vector<std::unordered_map<uint32_t, VkCommandPool>> command_pools_by_thread_idx;

        /*!
         * \brief Keeps track of how much has been allocated from each heap
         *
         * In the same order as VulkanGpuInfo::memory_properties::memoryHeaps
         */
        std::vector<uint32_t> heap_usages;

        /*!
         * \brief Map from HOST_VISIBLE memory allocations to the memory address they're mapped to
         *
         * The Vulkan render engine maps memory when it's allocated, if the memory is for a uniform or a staging
         * buffer.
         */
        std::unordered_map<VkDeviceMemory, void*> heap_mappings;

        // Debugging things
        PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = nullptr;
        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;

#pragma region Initialization
        std::vector<const char*> enabled_layer_names;

        void create_instance();

        void enable_debug_output();

        void initialize_vma();

        void create_device_and_queues();

        static bool does_device_support_extensions(VkPhysicalDevice device);

        void create_swapchain();

        void create_per_thread_command_pools();

        [[nodiscard]] std::unordered_map<uint32_t, VkCommandPool> make_new_command_pools() const;
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

        [[nodiscard]] VkShaderModule create_shader_module(const std::vector<uint32_t>& spirv) const;

        [[nodiscard]] std::vector<VkDescriptorSetLayout> create_descriptor_set_layouts(
            const std::unordered_map<std::string, ResourceBindingDescription>& all_bindings) const;

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
        [[nodiscard]] VkImageView image_view_for_image(const Image* image);

        [[nodiscard]] static VkCommandBufferLevel to_vk_command_buffer_level(const CommandList::Level level);
#pragma endregion

#pragma region Debugging
        VkDebugUtilsMessengerEXT debug_callback{};

        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                    void* pUserData);
#pragma endregion
    };
} // namespace nova::renderer::rhi
