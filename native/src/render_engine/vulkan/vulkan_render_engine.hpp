//
// Created by jannis on 30.08.18.
//

#ifndef NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
#define NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP

#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR // Use X11 for window creating on Linux... TODO: Wayland?
#define NOVA_VK_XLIB
#endif
#include <vulkan/vulkan.h>
#include <thread>
#include <mutex>
#include "../render_engine.hpp"
#include "vulkan_utils.hpp"
#include "x11_window.hpp"

namespace nova {
    struct vulkan_queue {
        VkQueue queue;
        uint32_t queue_idx;
    };

    class vulkan_render_engine : public render_engine {
    private:
        std::vector<const char *> enabled_validation_layer_names;

        VkInstance vk_instance;
#ifdef NOVA_VK_XLIB
        x11_window *window = nullptr;
#endif
        VkSurfaceKHR surface;
        VkDevice device;

        void create_device();

#ifndef NDEBUG
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
        PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;

        static VkBool32 debug_report_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type, uint64_t object, size_t location, int32_t messageCode,
                const char *layer_prefix, const char *message, void *user_data);

        VkDebugReportCallbackEXT debug_callback;
#endif

    public:
        explicit vulkan_render_engine(const settings &settings);
        ~vulkan_render_engine();

        void open_window(uint32_t width, uint32_t height) override;

        std::unique_ptr<command_buffer_base> allocate_command_buffer(command_buffer_type type) override;
        void free_command_buffer(std::unique_ptr<command_buffer_base> buf) override;

        void present_swapchain_image() override;

        iwindow* get_window() const override;

        static const std::string get_engine_name();

    private:
        /*!
         * \brief A CommandPool can't be used from more than one thread at once, so we need to figure out what thread
         * the code requesting a command buffer is in and use the appropriate thread pool
         */
        std::unordered_map<std::thread::id, VkCommandPool> thread_local_pools;
        std::mutex thread_local_pools_lock;

        /*!
         * \brief Same as above - a command buffer is tied to a command pool, so they need to be used in the same thread
         */
        std::unordered_map<std::thread::id, std::unordered_map<uint32_t, std::vector<std::unique_ptr<command_buffer_base>>>> thread_local_buffers;
        std::mutex thread_local_buffers_lock;

        /*!
         * \brief The queue that supports the operations that each command buffer type needs
         */
        std::unordered_map<command_buffer_type, vulkan_queue> queues_per_type;
    };
}


#endif //NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
