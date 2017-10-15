/*!
 * \author ddubois 
 * \date 15-Oct-17.
 */

#include "vulkan_instance.h"
#include <easylogging++.h>

namespace nova {
    std::vector<const char*> validation_layers = {
            "VK_LAYER_LUNARG_core_validation",
            "VK_LAYER_LUNARG_standard_validation"
    };

    bool layers_are_supported();

    std::vector<const char *> get_required_extensions(glfw_vk_window &window);

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
            VkDebugReportFlagsEXT flags,
            VkDebugReportObjectTypeEXT objType,
            uint64_t obj,
            size_t location,
            int32_t code,
            const char* layerPrefix,
            const char* msg,
            void* userData);

    VkInstance create_instance(glfw_vk_window& window) {
        VkInstance instance;

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

        auto extensions = get_required_extensions(window);
        create_info.ppEnabledExtensionNames = extensions.data();
        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());

#ifdef NDEBUG
        create_info.enabledLayerCount = 0;
#else
        if(!layers_are_supported()) {
            LOG(FATAL) << "The layers we need aren't available";
        }

        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
#endif

        VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
        if(result != VK_SUCCESS) {
            LOG(FATAL) << "Could not create Vulkan instance";
        }

        return instance;
    }

    VkDebugReportCallbackEXT setup_debug_callback(VkInstance instance) {
#ifndef NDEBUG
        VkDebugReportCallbackCreateInfoEXT create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        create_info.pfnCallback = debug_callback;

        VkDebugReportCallbackEXT callback;
        if(CreateDebugReportCallbackEXT(instance, &create_info, nullptr, &callback) != VK_SUCCESS) {
            LOG(FATAL) << "Could not set up debug callback";
        }

        return callback;
#endif

        return nullptr;
    }

    VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
        auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pCallback);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
        if (func != nullptr) {
            func(instance, callback, pAllocator);
        }
    }

    // This function should really be called outside of this file, but I want to keep vulkan creation things in here
    // to avoid making nova_renderer.cpp any larger than it needs to be
    std::vector<const char *> get_required_extensions(glfw_vk_window &window) {
        uint32_t glfw_extensions_count = 0;
        auto glfw_extensions = window.get_required_extensions(&glfw_extensions_count);

        std::vector<const char*> extensions;

        for(uint32_t i = 0; i < glfw_extensions_count; i++) {
            extensions.push_back(glfw_extensions[i]);
        }

#ifndef NDEBUG
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

        return extensions;
    }

    bool layers_are_supported() {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for(auto layer_name : validation_layers) {
            LOG(TRACE) << "Checking for layer " << layer_name;

            bool layer_found = false;

            for(const auto& layer_propeties : available_layers) {
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
            const char* layerPrefix,
            const char* msg,
            void* userData) {

        LOG(INFO) << "validation layer: " << msg;

        return VK_FALSE;
    }
}
