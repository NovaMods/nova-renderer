/*!
 * \author ddubois 
 * \date 15-Oct-17.
 */

#include <easylogging++.h>
#include <unordered_set>
#include "render_device.h"

namespace nova {
    bool layers_are_supported(std::vector<const char*>& validation_layers);

    std::vector<const char *> get_required_extensions(glfw_vk_window &window);

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
            VkDebugReportFlagsEXT flags,
            VkDebugReportObjectTypeEXT objType,
            uint64_t obj,
            size_t location,
            int32_t code,
            const char *layerPrefix,
            const char *msg,
            void *userData);

    void render_device::create_instance(glfw_vk_window &window) {
        validation_layers = {
                "VK_LAYER_LUNARG_core_validation",
                "VK_LAYER_LUNARG_standard_validation"
        };

        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Minecraft Nova Renderer";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "Nova Renderer 0.5";
        app_info.engineVersion = VK_MAKE_VERSION(0, 5, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;
        LOG(TRACE) << "Created VkApplicationInfo struct";

        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        extensions = get_required_extensions(window);
        create_info.ppEnabledExtensionNames = extensions.data();
        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());

#ifdef NDEBUG
        create_info.enabledLayerCount = 0;
#else
        if(!layers_are_supported(validation_layers)) {
            LOG(FATAL) << "The layers we need aren't available";
        }

        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
#endif

        VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
        if(result != VK_SUCCESS) {
            LOG(FATAL) << "Could not create Vulkan instance";
        }
    }


    void render_device::setup_debug_callback() {
#ifndef NDEBUG
        VkDebugReportCallbackCreateInfoEXT create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        create_info.pfnCallback = debug_callback;

        if(CreateDebugReportCallbackEXT(instance, &create_info, nullptr, &callback) != VK_SUCCESS) {
            LOG(FATAL) << "Could not set up debug callback";
        }
#endif
    }

    void render_device::find_device_and_queues() {
        if(instance == nullptr) {
            LOG(FATAL) << "Don't call this before creating the Vulkan instance and assigning it to me";
        }

        enumerate_gpus();
        select_physical_device();
        create_logical_device_and_queues();
    }

    void render_device::enumerate_gpus() {
        uint32_t num_devices = 0;
        auto err = vkEnumeratePhysicalDevices(this->instance, &num_devices, nullptr);
        if(err != VK_SUCCESS) {
            LOG(FATAL) << "Could not enumerate devices. Are you sure you have a GPU?";
        }
        if(num_devices == 0) {
            LOG(FATAL) << "Apparently you have zero devices. You know you need a GPU to run Nova, right>";
        }

        std::vector<VkPhysicalDevice> devices(num_devices);
        err = vkEnumeratePhysicalDevices(this->instance, &num_devices, devices.data());
        if(err != VK_SUCCESS) {
            LOG(FATAL) << "Could not enumerate physical devices";
        }
        if(num_devices == 0) {
            LOG(FATAL) << "Apparently you have zero devices. You know you need a GPU to run Nova, right>";
        }

        this->gpus.reserve(num_devices);
        for(uint32_t i = 0; i < num_devices; i++) {
            gpu_info gpu = this->gpus[i];
            gpu.device = devices[i];

            // get the queues the device supports
            uint32_t num_queues = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(gpu.device, &num_queues, nullptr);
            gpu.queue_family_props.reserve(num_queues);
            vkGetPhysicalDeviceQueueFamilyProperties(gpu.device, &num_queues, gpu.queue_family_props.data());

            // Get the extensions the device supports
            uint32_t num_extensions;
            vkEnumerateDeviceExtensionProperties(gpu.device, nullptr, &num_extensions, nullptr);
            gpu.extention_props.reserve(num_extensions);
            vkEnumerateDeviceExtensionProperties(gpu.device, nullptr, &num_extensions, gpu.extention_props.data());

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.device, this->surface, &gpu.surface_capabilities);

            uint32_t num_formats = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.device, this->surface, &num_formats, nullptr);
            gpu.surface_formats.reserve(num_formats);
            vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.device, this->surface, &num_formats, gpu.surface_formats.data());

            uint32_t num_present_modes;
            vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.device, surface, &num_present_modes, nullptr);
            gpu.present_modes.reserve(num_present_modes);
            vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.device, surface, &num_present_modes,
                                                      gpu.present_modes.data());

            vkGetPhysicalDeviceMemoryProperties(gpu.device, &gpu.mem_props);
            vkGetPhysicalDeviceProperties(gpu.device, &gpu.props);
        }
    }

    void render_device::select_physical_device() {
        // TODO: More complex logic to try and use a non-Intel GPU if possible (Vulkan book page 9)
        for(auto &gpu : gpus) {
            uint32_t graphics_idx = 0xFFFFFFFF;
            uint32_t present_idx = 0xFFFFFFFF;

            if(gpu.surface_formats.size() == 0) {
                continue;
            }

            if(gpu.present_modes.size() == 0) {
                continue;
            }

            // Find graphics queue family
            for(int i = 0; i < gpu.queue_family_props.size(); i++) {
                auto &props = gpu.queue_family_props[i];
                if(props.queueCount == 0) {
                    continue;
                }

                if(props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    graphics_idx = i;
                    break;
                }
            }

            // Find present queue family
            for(int i = 0; i < gpu.queue_family_props.size(); i++) {
                auto &props = gpu.queue_family_props[i];

                if(props.queueCount == 0) {
                    continue;
                }

                VkBool32 supports_present = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(gpu.device, i, surface, &supports_present);
                if(supports_present == VK_TRUE) {
                    present_idx = i;
                    break;
                }
            }

            if(graphics_idx >= 0 && present_idx >= 0) {
                graphics_family_idx = graphics_idx;
                present_family_idx = present_idx;
                physical_device = gpu.device;
                this->gpu = &gpu;
                return;
            }
        }

        LOG(FATAL) << "Could not find a device with both present and graphics queues";
    }

    void render_device::create_logical_device_and_queues() {
        std::unordered_set<uint32_t> unique_idx;
        unique_idx.insert(graphics_family_idx);
        unique_idx.insert(present_family_idx);

        std::vector<VkDeviceQueueCreateInfo> devq_info;

        const float priority = 1.0;
        for(auto idx : unique_idx) {
            VkDeviceQueueCreateInfo qinfo = {};
            qinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            qinfo.queueFamilyIndex = idx;
            qinfo.queueCount = 1;

            qinfo.pQueuePriorities = &priority;
            devq_info.push_back(qinfo);
        }

        // Do I have to look at the loaded shaderpack and see what features it needs? For now I'll just add whatever looks
        // good
        VkPhysicalDeviceFeatures device_features = {};
        device_features.geometryShader = VK_TRUE;
        device_features.tessellationShader = VK_TRUE;
        device_features.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.queueCreateInfoCount = static_cast<uint32_t>(devq_info.size());
        info.pQueueCreateInfos = devq_info.data();
        info.pEnabledFeatures = &device_features;

        info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        if(!validation_layers.empty()) {
            info.ppEnabledLayerNames = validation_layers.data();
        }

        auto err = vkCreateDevice(physical_device, &info, nullptr, &device);
        if(err != VK_SUCCESS) {
            LOG(FATAL) << "Could not create logical device";
        }

        vkGetDeviceQueue(device, graphics_family_idx, 0, &graphics_queue);
        vkGetDeviceQueue(device, graphics_family_idx, 0, &present_queue);
    }

    VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkDebugReportCallbackEXT *pCallback) {
        auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance,
                                                                               "vkCreateDebugReportCallbackEXT");
        if(func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pCallback);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                       const VkAllocationCallbacks *pAllocator) {
        auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance,
                                                                                "vkDestroyDebugReportCallbackEXT");
        if(func != nullptr) {
            func(instance, callback, pAllocator);
        }
    }

// This function should really be called outside of this file, but I want to keep vulkan creation things in here
// to avoid making nova_renderer.cpp any larger than it needs to be
    std::vector<const char *> get_required_extensions(glfw_vk_window &window) {
        uint32_t glfw_extensions_count = 0;
        auto glfw_extensions = window.get_required_extensions(&glfw_extensions_count);

        std::vector<const char *> extensions;

        for(uint32_t i = 0; i < glfw_extensions_count; i++) {
            extensions.push_back(glfw_extensions[i]);
        }

#ifndef NDEBUG
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

        return extensions;
    }

    bool layers_are_supported(std::vector<const char*>& validation_layers) {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for(auto layer_name : validation_layers) {
            LOG(TRACE) << "Checking for layer " << layer_name;

            bool layer_found = false;

            for(const auto &layer_propeties : available_layers) {
                if(strcmp(layer_name, layer_propeties.layerName) == 0) {
                    LOG(TRACE) << "Found it!";
                    layer_found = true;
                    break;
                }
            }

            if(!layer_found) {
                LOG(ERROR) << "Could not find layer " << layer_name;
                return false;
            }
        }

        return true;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
            VkDebugReportFlagsEXT flags,
            VkDebugReportObjectTypeEXT objType,
            uint64_t obj,
            size_t location,
            int32_t code,
            const char *layerPrefix,
            const char *msg,
            void *userData) {

        LOG(INFO) << "validation layer: " << msg;

        return VK_FALSE;
    }
}