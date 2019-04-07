/*!
 * \author ddubois
 * \date 03-Apr-19.
 */

#pragma once

#include <memory>

#include <nova_renderer/render_engine_t.hpp>

#include "swapchain.hpp"

#include "../../util/vma_usage.hpp"
#include "vk_structs.hpp"

namespace nova::renderer::rhi {
    /*!
     * \brief Vulkan implementation of a render engine
     */
    class vk_render_engine : public render_engine_t {
    public:
        vk_render_engine(nova_settings& settings);

        vk_render_engine(vk_render_engine&& old) noexcept = delete;
        vk_render_engine& operator=(vk_render_engine&& old) noexcept = delete;

        vk_render_engine(const vk_render_engine& other) = delete;
        vk_render_engine& operator=(const vk_render_engine& other) = delete;

        // Inherited via render_engine
        std::shared_ptr<window_t> get_window() const override final;

        result<renderpass_t*> create_renderpass(const shaderpack::render_pass_create_info_t& data) override final;
        pipeline_t* create_pipeline(const shaderpack::pipeline_create_info_t& data) override final;
        resource_t* create_buffer(const buffer_create_info_t& info) override final;
        resource_t* create_texture(const texture2d_create_info_t& info) override final;
        semaphore_t* create_semaphore() override final;
        std::vector<semaphore_t*> create_semaphores(uint32_t num_semaphores) override final;
        fence_t* create_fence(bool signaled = false) override final;
        std::vector<fence_t*> create_fences(uint32_t num_fences, bool signaled = false) override final;

        void destroy_renderpass(renderpass_t* pass) override final;
        void destroy_pipeline(pipeline_t* pipeline) override final;
        void destroy_resource(resource_t* resource) override final;
        void destroy_semaphores(const std::vector<semaphore_t*>& semaphores) override final;
        void destroy_fences(const std::vector<fence_t*>& fences) override final;

        command_list_t* allocate_command_list(uint32_t thread_idx, queue_type needed_queue_type, command_list_t::level level) override final;
        void submit_command_list(command_list_t* cmds,
                                 queue_type queue,
                                 fence_t* fence_to_signal = nullptr,
                                 const std::vector<semaphore_t*>& wait_semaphores = {},
                                 const std::vector<semaphore_t*>& signal_semaphores = {}) override final;

    protected:
        void open_window_and_create_surface(const nova_settings::window_options& options) override final;

    private:
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

        std::unique_ptr<swapchain_manager> swapchain;
        uint32_t max_in_flight_frames = 3;

        /*!
         * The index in the vector is the thread index, the key in the map is the queue family index
         */
        std::vector<std::unordered_map<uint32_t, VkCommandPool>> command_pools_by_thread_idx;

        // Info about the hardware
        vk_gpu_info gpu;

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

        bool does_device_support_extensions(VkPhysicalDevice device);

        void create_swapchain();

        void create_per_thread_command_pools();

        std::unordered_map<uint32_t, VkCommandPool> make_new_command_pools() const;

        VkDescriptorPool make_new_descriptor_pool() const;
#pragma endregion

#pragma region Debugging
        VkDebugUtilsMessengerEXT debug_callback{};

        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                    void* pUserData);
#pragma endregion
    };
} // namespace nova::renderer
