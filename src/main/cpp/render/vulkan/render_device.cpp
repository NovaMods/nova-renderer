/*!
 * \author ddubois 
 * \date 15-Oct-17.
 */

#include <easylogging++.h>
#include <unordered_set>
#include "render_device.h"

namespace nova {
    render_device render_device::instance;

    bool layers_are_supported(std::vector<const char*>& validation_layers);

    std::vector<const char *> get_required_extensions(glfw_vk_window &window);

    VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_callback(
            vk::DebugReportFlagsEXT flags,
            vk::DebugReportObjectTypeEXT objType,
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

        vk::ApplicationInfo app_info = {};
        app_info.pApplicationName = "Minecraft Nova Renderer";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "Nova Renderer 0.5";
        app_info.engineVersion = VK_MAKE_VERSION(0, 5, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;
        LOG(TRACE) << "Created vk::ApplicationInfo struct";

        vk::InstanceCreateInfo create_info = {};
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

        vk_instance = vk::createInstance(create_info, nullptr);
    }

    void render_device::setup_debug_callback() {
#ifndef NDEBUG
        /*vk::DebugReportCallbackCreateInfoEXT create_info = {};
        create_info.flags = vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning;
        create_info.pfnCallback = debug_callback;

        if(CreateDebugReportCallbackEXT(vk_instance, &create_info, nullptr, &callback) != vk::Result::eSuccess) {
            LOG(FATAL) << "Could not set up debug callback";
        }*/
#endif
    }

    void render_device::find_device_and_queues() {
        enumerate_gpus();
        LOG(TRACE) << "Enumerated GPUs";
        select_physical_device();
        LOG(TRACE) << "Found a physical device that will work I guess";
        create_logical_device_and_queues();
        LOG(TRACE) << "Basic queue and logical device was found";
    }

    void render_device::enumerate_gpus() {
        auto devices = vk_instance.enumeratePhysicalDevices();
        LOG(TRACE) << "There are " << devices.size() << " physical devices";
        if(devices.empty()) {
            LOG(FATAL) << "Apparently you have zero devices. You know you need a GPU to run Nova, right?";
        }

        this->gpus.resize(devices.size());
        LOG(TRACE) << "Reserved " << devices.size() << " slots for devices";
        for(uint32_t i = 0; i < devices.size(); i++) {
            gpu_info& gpu = this->gpus[i];
            gpu.device = devices[i];

            // get the queues the device supports
            gpu.queue_family_props = gpu.device.getQueueFamilyProperties();
            LOG(TRACE) << "Got the physical device queue properties";

            // Get the extensions the device supports
            gpu.extention_props = gpu.device.enumerateDeviceExtensionProperties();
            LOG(TRACE) << "Got the device extension properties";

            gpu.surface_capabilities = gpu.device.getSurfaceCapabilitiesKHR(surface);
            LOG(TRACE) << "Got the physical device surface capabilities";

            gpu.surface_formats = gpu.device.getSurfaceFormatsKHR(surface);
            LOG(TRACE) << "Got the physical device's surface formats";

            gpu.present_modes = gpu.device.getSurfacePresentModesKHR(surface);
            LOG(TRACE) << "Got the surface present modes";

            gpu.mem_props = gpu.device.getMemoryProperties();
            gpu.props = gpu.device.getProperties();
            gpu.supported_features = gpu.device.getFeatures();
            LOG(TRACE) << "Got the memory properties and deice properties";
        }
    }

    void render_device::select_physical_device() {
        // TODO: More complex logic to try and use a non-Intel GPU if possible (Vulkan book page 9)
        for(auto &gpu : gpus) {
            uint32_t graphics_idx = 0xFFFFFFFF;
            uint32_t present_idx = 0xFFFFFFFF;

            if(gpu.surface_formats.empty()) {
                continue;
            }

            if(gpu.present_modes.empty()) {
                continue;
            }

            // Find graphics queue family
            for(int i = 0; i < gpu.queue_family_props.size(); i++) {
                auto &props = gpu.queue_family_props[i];
                if(props.queueCount == 0) {
                    continue;
                }

                if(props.queueFlags & vk::QueueFlagBits::eGraphics) {
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

                vk::Bool32 supports_present = VK_FALSE;
                gpu.device.getSurfaceSupportKHR(i, surface, &supports_present);
                if(supports_present == VK_TRUE) {
                    present_idx = i;
                    break;
                }
            }

            if(graphics_idx >= 0 && present_idx >= 0) {
                graphics_family_idx = graphics_idx;
                present_family_idx = present_idx;
                physical_device = gpu.device;
                this->gpu = gpu;
                return;
            }
        }

        LOG(FATAL) << "Could not find a device with both present and graphics queues";
    }

    void render_device::create_logical_device_and_queues() {
        std::unordered_set<uint32_t> unique_idx;
        unique_idx.insert(graphics_family_idx);
        unique_idx.insert(present_family_idx);

        std::vector<vk::DeviceQueueCreateInfo> devq_info;

        // TODO: Possibly create a queue for texture streaming and another for geometry streaming?
        const float priority = 1.0;
        for(auto idx : unique_idx) {
            vk::DeviceQueueCreateInfo qinfo = {};
            qinfo.queueFamilyIndex = idx;
            qinfo.queueCount = 1;

            qinfo.pQueuePriorities = &priority;
            devq_info.push_back(qinfo);
        }

        // Do I have to look at the loaded shaderpack and see what features it needs? For now I'll just add whatever looks
        // good
        vk::PhysicalDeviceFeatures device_features = {};
        device_features.geometryShader = VK_TRUE;
        device_features.tessellationShader = VK_TRUE;
        device_features.samplerAnisotropy = VK_TRUE;

        vk::DeviceCreateInfo info = {};
        info.queueCreateInfoCount = static_cast<uint32_t>(devq_info.size());
        info.pQueueCreateInfos = devq_info.data();
        info.pEnabledFeatures = &device_features;

        info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        if(!validation_layers.empty()) {
            info.ppEnabledLayerNames = validation_layers.data();
        }

        device = physical_device.createDevice(info, nullptr);

        graphics_queue = device.getQueue(graphics_family_idx, 0);
        present_queue = device.getQueue(graphics_family_idx, 0);
    }

    void render_device::create_semaphores() {
        acquire_semaphores.resize(NUM_FRAME_DATA);
        render_complete_semaphores.resize(NUM_FRAME_DATA);

        vk::SemaphoreCreateInfo semaphore_create_info = {};
        for(int i = 0; i < NUM_FRAME_DATA; i++) {
            acquire_semaphores[i] = device.createSemaphore(semaphore_create_info, nullptr);
            render_complete_semaphores[i] = device.createSemaphore(semaphore_create_info, nullptr);
        }
    }

    void render_device::create_command_pool_and_command_buffers() {
        // TODO: Get the number of threads dynamically based on the user's CPU core count
        command_buffer_pool = std::make_unique<command_pool>(device, graphics_family_idx, 8);
    }

    // This function should really be outside of this file, but I want to keep vulkan creation things in here
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
        auto available_layers = vk::enumerateInstanceLayerProperties();

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

    VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_callback(
            vk::DebugReportFlagsEXT flags,
            vk::DebugReportObjectTypeEXT objType,
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