//
// Created by jannis on 30.08.18.
//

#include "vulkan_render_engine.hpp"
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
namespace nova {

    vulkan_render_engine::vulkan_render_engine(const settings &settings) : render_engine(settings) {
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

    vulkan_render_engine::~vulkan_render_engine() {

    }

    void vulkan_render_engine::open_window(uint32_t width, uint32_t height) {

    }

    command_buffer *vulkan_render_engine::allocate_command_buffer() {
            return nullptr;
    }

    void vulkan_render_engine::free_command_buffer(command_buffer *buf) {

    }

    const std::string vulkan_render_engine::get_engine_name() {
        return "vulkan-1.1";
    }

    void vulkan_render_engine::create_device() {

        uint32_t device_count;
        NOVA_THROW_IF_VK_ERROR(vkEnumeratePhysicalDevices(vk_instance, &device_count, nullptr), render_engine_initialization_exception);
        auto *physical_devices = new VkPhysicalDevice[device_count];
        NOVA_THROW_IF_VK_ERROR(vkEnumeratePhysicalDevices(vk_instance, &device_count, physical_devices), render_engine_initialization_exception);

        VkPhysicalDevice choosen_device = nullptr;
        for(int i = 0; i < device_count; i++) {
            VkPhysicalDevice current_device = physical_devices[i];
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(current_device, &properties);

            if(properties.vendorID == 0x8086 && device_count - 1 > i) { // Intel GPU... they are not powerful and we have more available, so skip it
                continue;
            }

            uint32_t queue_familiy_count;
            vkGetPhysicalDeviceQueueFamilyProperties(current_device, &queue_familiy_count, nullptr);
            auto *family_properties = new VkQueueFamilyProperties[queue_familiy_count];
            vkGetPhysicalDeviceQueueFamilyProperties(current_device, &queue_familiy_count, family_properties);

            for(int j = 0; j < queue_familiy_count; j++) {
                VkQueueFamilyProperties current_properties = family_properties[i];
                if(current_properties.queueCount < 1) {
                    continue;
                }
            }

            delete[] family_properties;
        }

        delete[] physical_devices;
    }
}
#pragma clang diagnostic pop