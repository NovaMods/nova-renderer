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
#include "../render_engine.hpp"
#include "vulkan_utils.hpp"

namespace nova {
    class vulkan_render_engine : public render_engine {
    private:
        VkInstance vk_instance;

    public:
        explicit vulkan_render_engine(const settings &settings);
        ~vulkan_render_engine();

        void open_window(uint32_t width, uint32_t height) override;

        command_buffer* allocate_command_buffer() override;

        void free_command_buffer(command_buffer* buf) override;

        static const std::string get_engine_name();
    };
}


#endif //NOVA_RENDERER_VULKAN_RENDER_ENGINE_HPP
