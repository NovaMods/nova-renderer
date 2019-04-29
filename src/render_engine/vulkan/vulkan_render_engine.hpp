/*!
 * \author ddubois
 * \date 03-Apr-19.
 */

#pragma once

#include <memory>

#include "nova_renderer/render_engine.hpp"

#include "swapchain.hpp"

#include "../../util/vma_usage.hpp"
#include "vk_structs.hpp"

namespace nova::renderer::rhi {
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
        std::shared_ptr<Window> get_window() const override final;

        void set_num_renderpasses(uint32_t num_renderpasses) override final;

        Result<Renderpass*> create_renderpass(const shaderpack::RenderPassCreateInfo& data) override final;

        Framebuffer* create_framebuffer(const Renderpass* renderpass,
                                        const std::vector<Image*>& attachments,
                                        const glm::uvec2& framebuffer_size) override final;

        Result<PipelineInterface*> create_pipeline_interface(
            const std::unordered_map<std::string, ResourceBindingDescription>& bindings,
            const std::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
            const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture) override final;

        Result<Pipeline*> create_pipeline(const PipelineInterface* pipeline_interface,
                                          const shaderpack::PipelineCreateInfo& data) override final;

        Buffer* create_buffer(const BufferCreateInfo& info) override final;
        Image* create_texture(const shaderpack::TextureCreateInfo& info) override final;
        Semaphore* create_semaphore() override final;
        std::vector<Semaphore*> create_semaphores(uint32_t num_semaphores) override final;
        Fence* create_fence(bool signaled = false) override final;
        std::vector<Fence*> create_fences(uint32_t num_fences, bool signaled = false) override final;

        void destroy_renderpass(Renderpass* pass) override final;

        void destroy_framebuffer(const Framebuffer* framebuffer) override final;

        void destroy_pipeline(Pipeline* pipeline) override final;
        void destroy_texture(Image* resource) override final;
        void destroy_semaphores(const std::vector<Semaphore*>& semaphores) override final;
        void destroy_fences(const std::vector<Fence*>& fences) override final;

        CommandList* allocate_command_list(uint32_t thread_idx, QueueType needed_queue_type, CommandList::Level level) override final;
        void submit_command_list(CommandList* cmds,
                                 QueueType queue,
                                 Fence* fence_to_signal = nullptr,
                                 const std::vector<Semaphore*>& wait_semaphores = {},
                                 const std::vector<Semaphore*>& signal_semaphores = {}) override final;
#pragma endregion

        VkCommandPool get_command_pool_for_thread(uint32_t thread_idx, uint32_t queue_family_index);

        uint32_t get_queue_family_index(QueueType type) const;

    protected:
        void open_window_and_create_surface(const NovaSettings::WindowOptions& options) override final;

    private:
        std::unique_ptr<VulkanSwapchainManager> swapchain;
        uint32_t max_in_flight_frames = 3;

        /*!
         * The index in the vector is the thread index, the key in the map is the queue family index
         */
        std::vector<std::unordered_map<uint32_t, VkCommandPool>> command_pools_by_thread_idx;

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

        std::unordered_map<uint32_t, VkCommandPool> make_new_command_pools() const;

        VkDescriptorPool make_new_descriptor_pool() const;
#pragma endregion

#pragma region Helpers
        VkShaderModule create_shader_module(const std::vector<uint32_t>& spirv) const;

        std::vector<VkDescriptorSetLayout> create_descriptor_set_layouts(
            const std::unordered_map<std::string, ResourceBindingDescription>& all_bindings) const;
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
