//
// Created by jannis on 30.08.18.
//

#include "vulkan_render_engine.hpp"
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
namespace nova {

    vulkan_render_engine::vulkan_render_engine() {

    }

    vulkan_render_engine::~vulkan_render_engine() {

    }

    void vulkan_render_engine::init(settings settings) {
        settings_options options = settings.get_options();

        VkApplicationInfo application_info;
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pNext = nullptr;
        application_info.pApplicationName = options.application_name.c_str();
        application_info.applicationVersion = VK_MAKE_VERSION(options.application_version.major, options.application_version.minor, options.application_version.patch);
        application_info.pEngineName = "Nova renderer 0.1";
        application_info.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.pApplicationInfo = &application_info;

        std::vector<const char *> enabled_layer_names;
        create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layer_names.size());
        create_info.ppEnabledLayerNames = enabled_layer_names.data();

        std::vector<const char *> enabled_extension_names;
#ifdef NOVA_VK_XLIB
        enabled_extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#else
#error Unsupported window system
#endif
        create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extension_names.size());
        create_info.ppEnabledExtensionNames = enabled_extension_names.data();

        NOVA_THROW_IF_VK_ERROR(vkCreateInstance(&create_info, nullptr, &vk_instance), render_engine_initialization_exception);
    }

    std::string vulkan_render_engine::get_engine_name() const {
        return "vulkan-1.1";
    }
}
#pragma clang diagnostic pop