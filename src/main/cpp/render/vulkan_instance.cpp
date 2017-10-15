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

    VkInstance create_instance(glfw_vk_window& window) {
        VkInstance instance;

        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Minecraft Nova Renderer";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 61);
        app_info.pEngineName = "Nova Renderer";
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

        for(auto extension : extensions) {
            LOG(TRACE) << "Want to enable extension " << extension;
        }
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
}
