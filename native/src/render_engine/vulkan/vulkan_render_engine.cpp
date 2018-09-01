//
// Created by jannis on 30.08.18.
//

#include "vulkan_render_engine.hpp"
#include <vector>
#include "../../util/logger.hpp"

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
#ifndef NDEBUG
        enabled_validation_layer_names.push_back("VK_LAYER_LUNARG_standard_validation");
#endif
        create_info.enabledLayerCount = static_cast<uint32_t>(enabled_validation_layer_names.size());
        create_info.ppEnabledLayerNames = enabled_validation_layer_names.data();

        std::vector<const char *> enabled_extension_names;
        enabled_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef NOVA_VK_XLIB
        enabled_extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#else
#error Unsupported window system
#endif

#ifndef NDEBUG
        enabled_extension_names.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
        create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extension_names.size());
        create_info.ppEnabledExtensionNames = enabled_extension_names.data();

        NOVA_THROW_IF_VK_ERROR(vkCreateInstance(&create_info, nullptr, &vk_instance), render_engine_initialization_exception);

#ifndef NDEBUG
        vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(vk_instance, "vkCreateDebugReportCallbackEXT"));
        vkDebugReportMessageEXT = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(vk_instance, "vkDebugReportMessageEXT"));
        vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugReportCallbackEXT"));

        VkDebugReportCallbackCreateInfoEXT debug_create_info;
        debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_create_info.pNext = nullptr;
        debug_create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debug_create_info.pfnCallback = &debug_report_callback;
        debug_create_info.pUserData = this;

        NOVA_THROW_IF_VK_ERROR(vkCreateDebugReportCallbackEXT(vk_instance, &debug_create_info, nullptr, &debug_callback), render_engine_initialization_exception);
#endif
    }

    vulkan_render_engine::~vulkan_render_engine() {
#ifdef NOVA_VK_XLIB
        delete window;
#endif
    }

    void vulkan_render_engine::open_window(uint32_t width, uint32_t height) {
#ifdef NOVA_VK_XLIB
        window = new x11_window(width, height);

        VkXlibSurfaceCreateInfoKHR x_surface_create_info;
        x_surface_create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        x_surface_create_info.pNext = nullptr;
        x_surface_create_info.flags = 0;
        x_surface_create_info.dpy = window->get_display();
        x_surface_create_info.window = window->get_x11_window();

        NOVA_THROW_IF_VK_ERROR(vkCreateXlibSurfaceKHR(vk_instance, &x_surface_create_info, nullptr, &surface), x_window_creation_exception);
#endif
        create_device();
        window->enter_loop();
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

        uint32_t graphics_family_idx = 0xFFFFFFFF;
        uint32_t present_familiy_idx = 0xFFFFFFFF;

        VkPhysicalDevice choosen_device = nullptr;
        for(int i = 0; i < device_count; i++) {
            graphics_family_idx = 0xFFFFFFFF;
            present_familiy_idx = 0xFFFFFFFF;
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

            for(uint32_t j = 0; j < queue_familiy_count; j++) {
                VkQueueFamilyProperties current_properties = family_properties[j];
                if(current_properties.queueCount < 1) {
                    continue;
                }

                VkBool32 supports_present = VK_FALSE;
                NOVA_THROW_IF_VK_ERROR(vkGetPhysicalDeviceSurfaceSupportKHR(current_device, j, surface, &supports_present), render_engine_initialization_exception);
                if(supports_present == VK_TRUE) {
                    present_familiy_idx = j;
                    break;
                }
            }

            for(uint32_t j = 0; j < queue_familiy_count; j++) {
                VkQueueFamilyProperties current_properties = family_properties[j];
                if(current_properties.queueCount < 1) {
                    continue;
                }

                if(current_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    graphics_family_idx = j;
                    break;
                }
            }

            delete[] family_properties;

            if(graphics_family_idx >= 0 && present_familiy_idx >= 0) {
                NOVA_LOG(INFO) << "Selected GPU " << properties.deviceName;
                choosen_device = current_device;
                break;
            }
        }

        if(!choosen_device) {
            throw render_engine_initialization_exception("Failed to find good GPU");
        }

        const float priority = 1.0;

        VkDeviceQueueCreateInfo graphics_queue_create_info{};
        graphics_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphics_queue_create_info.pNext = nullptr;
        graphics_queue_create_info.flags = 0;
        graphics_queue_create_info.queueCount = 1;
        graphics_queue_create_info.queueFamilyIndex = graphics_family_idx;
        graphics_queue_create_info.pQueuePriorities = &priority;

        VkDeviceQueueCreateInfo present_queue_create_info{};
        present_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        present_queue_create_info.pNext = nullptr;
        present_queue_create_info.flags = 0;
        present_queue_create_info.queueCount = 1;
        present_queue_create_info.queueFamilyIndex = present_familiy_idx;
        present_queue_create_info.pQueuePriorities = &priority;

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos = {graphics_queue_create_info, present_queue_create_info};

        VkPhysicalDeviceFeatures physical_device_features{};
        physical_device_features.geometryShader = VK_TRUE;
        physical_device_features.tessellationShader = VK_TRUE;
        physical_device_features.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pNext = nullptr;
        device_create_info.flags = 0;
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.pEnabledFeatures = &physical_device_features;
        device_create_info.enabledExtensionCount = 1;
        const char *swapchain_extension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        device_create_info.ppEnabledExtensionNames = &swapchain_extension;
        device_create_info.enabledLayerCount = static_cast<uint32_t>(enabled_validation_layer_names.size());
        if(!enabled_validation_layer_names.empty()) {
            device_create_info.ppEnabledLayerNames = enabled_validation_layer_names.data();
        }

        vkCreateDevice(choosen_device, &device_create_info, nullptr, &device);
        // NOVA_THROW_IF_VK_ERROR(, render_engine_initialization_exception);

        vkGetDeviceQueue(device, graphics_family_idx, 0, &graphics_queue);
        vkGetDeviceQueue(device, present_familiy_idx, 0, &present_queue);

        delete[] physical_devices;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_render_engine::debug_report_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                                uint64_t object, size_t location, int32_t message_code,
                                                const char *layer_prefix, const char *message, void *user_data) {
        auto *self = reinterpret_cast<vulkan_render_engine *>(user_data);
        NOVA_LOG(TRACE) << __FILE__ << ":" << __LINE__ << " >> VK Debug: [" << layer_prefix << "] " << message;
        return VK_FALSE;
    }
}
#pragma clang diagnostic pop