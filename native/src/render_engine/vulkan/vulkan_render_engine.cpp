//
// Created by jannis on 30.08.18.
//

#include "vulkan_render_engine.hpp"
#include <vector>
#include <set>
#include "../../util/logger.hpp"
#include <fstream>
#include "vulkan_opaque_types.hpp"
#include <cstring>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include "vulkan_type_converters.hpp"
#include <queue>
#include "../../util/utils.hpp"

namespace nova {
    vulkan_render_engine::vulkan_render_engine(const nova_settings &settings) : render_engine(settings) {
        settings_options options = settings.get_options();
        const auto& version = options.api.vulkan.appliction_version;

        VkApplicationInfo application_info;
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pNext = nullptr;
        application_info.pApplicationName = options.api.vulkan.application_name.c_str();
        application_info.applicationVersion = VK_MAKE_VERSION(version.major, version.minor, version.patch);
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
        vkDeviceWaitIdle(device);
        cleanup_dynamic();
        destroy_synchronization_objects();
        destroy_vertex_buffer();
        destroy_command_pool();
        destroy_framebuffers();
        destroy_graphics_pipelines();
        destroy_shader_modules();
        destroy_render_pass();
        destroy_image_views();
        destroy_swapchain();
        destroy_memory_allocator();
        destroy_device();
    }

    void vulkan_render_engine::open_window(uint32_t width, uint32_t height) {
#ifdef NOVA_VK_XLIB
        window = std::make_shared<x11_window>(width, height);

        VkXlibSurfaceCreateInfoKHR x_surface_create_info;
        x_surface_create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        x_surface_create_info.pNext = nullptr;
        x_surface_create_info.flags = 0;
        x_surface_create_info.dpy = window->get_display();
        x_surface_create_info.window = window->get_x11_window();

        NOVA_THROW_IF_VK_ERROR(vkCreateXlibSurfaceKHR(vk_instance, &x_surface_create_info, nullptr, &surface), x_window_creation_exception);
#else
#error Unsuported window system
#endif
        create_device();
        create_memory_allocator();
        create_swapchain();
        create_image_views();
    }

    const std::string vulkan_render_engine::get_engine_name() {
        return "vulkan-1.1";
    }

    void vulkan_render_engine::create_device() {
        uint32_t device_count;
        NOVA_THROW_IF_VK_ERROR(vkEnumeratePhysicalDevices(vk_instance, &device_count, nullptr), render_engine_initialization_exception);
        auto *physical_devices = new VkPhysicalDevice[device_count];
        NOVA_THROW_IF_VK_ERROR(vkEnumeratePhysicalDevices(vk_instance, &device_count, physical_devices), render_engine_initialization_exception);

        uint32_t graphics_family_idx    = 0xFFFFFFFF;
        uint32_t compute_family_idx     = 0xFFFFFFFF;
        uint32_t copy_family_idx        = 0xFFFFFFFF;

        VkPhysicalDevice choosen_device = nullptr;
        for(uint32_t device_idx = 0; device_idx < device_count; device_idx++) {
            graphics_family_idx = 0xFFFFFFFF;
            VkPhysicalDevice current_device = physical_devices[device_idx];
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(current_device, &properties);

            if(properties.vendorID == 0x8086 && device_count - 1 > device_idx) { // Intel GPU... they are not powerful and we have more available, so skip it
                continue;
            }

            if(!does_device_support_extensions(current_device)) {
                continue;
            }

            uint32_t queue_familiy_count;
            vkGetPhysicalDeviceQueueFamilyProperties(current_device, &queue_familiy_count, nullptr);
            auto *family_properties = new VkQueueFamilyProperties[queue_familiy_count];
            vkGetPhysicalDeviceQueueFamilyProperties(current_device, &queue_familiy_count, family_properties);

            for(uint32_t queue_idx = 0; queue_idx < queue_familiy_count; queue_idx++) {
                VkQueueFamilyProperties current_properties = family_properties[queue_idx];
                if(current_properties.queueCount < 1) {
                    continue;
                }

                VkBool32 supports_present = VK_FALSE;
                NOVA_THROW_IF_VK_ERROR(vkGetPhysicalDeviceSurfaceSupportKHR(current_device, queue_idx, surface, &supports_present), render_engine_initialization_exception);
                VkQueueFlags supports_graphics = current_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT;
                if(supports_graphics && supports_present == VK_TRUE && graphics_family_idx == 0xFFFFFFFF) {
                    graphics_family_idx = queue_idx;
                }

                VkQueueFlags supports_compute = current_properties.queueFlags & VK_QUEUE_COMPUTE_BIT;
                if(supports_compute && compute_family_idx == 0xFFFFFFFF) {
                    compute_family_idx = queue_idx;
                }

                VkQueueFlags supports_copy = current_properties.queueFlags & VK_QUEUE_TRANSFER_BIT;
                if(supports_copy && copy_family_idx == 0xFFFFFFFF) {
                    copy_family_idx = queue_idx;
                }
            }

            delete[] family_properties;

            if(graphics_family_idx != 0xFFFFFFFF) {
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

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos = {graphics_queue_create_info};

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

        NOVA_THROW_IF_VK_ERROR(vkCreateDevice(choosen_device, &device_create_info, nullptr, &device), render_engine_initialization_exception);

        graphics_queue_index = graphics_family_idx;
        vkGetDeviceQueue(device, graphics_family_idx, 0, &graphics_queue);
        compute_queue_index = compute_family_idx;
        vkGetDeviceQueue(device, compute_family_idx, 0, &compute_queue);
        copy_queue_index = copy_family_idx;
        vkGetDeviceQueue(device, copy_family_idx, 0, &copy_queue);

        delete[] physical_devices;

        physical_device = choosen_device;
    }

    bool vulkan_render_engine::does_device_support_extensions(VkPhysicalDevice device) {
        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> available(extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available.data());

        std::set<std::string> required = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        for(const auto &extension : available) {
            required.erase(extension.extensionName);
        }

        return required.empty();
    }

    void vulkan_render_engine::create_memory_allocator() {
        VmaAllocatorCreateInfo allocator_create_info;
        allocator_create_info.physicalDevice = physical_device;
        allocator_create_info.device = device;
        allocator_create_info.flags = 0;
        allocator_create_info.frameInUseCount = 0;
        allocator_create_info.pAllocationCallbacks = nullptr;
        allocator_create_info.pDeviceMemoryCallbacks = nullptr;
        allocator_create_info.pHeapSizeLimit = nullptr;
        allocator_create_info.preferredLargeHeapBlockSize = 0;
        allocator_create_info.pVulkanFunctions = nullptr;

        NOVA_THROW_IF_VK_ERROR(vmaCreateAllocator(&allocator_create_info, &memory_allocator), render_engine_initialization_exception);
    }

    void vulkan_render_engine::create_swapchain() {
        uint32_t format_count;
        NOVA_THROW_IF_VK_ERROR(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr), render_engine_initialization_exception);
        if(format_count == 0) {
            throw render_engine_initialization_exception("No supported surface formats... something went really wrong");
        }
        std::vector<VkSurfaceFormatKHR> formats(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats.data());

        uint32_t present_mode_count;
        NOVA_THROW_IF_VK_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count,
                                                                         nullptr), render_engine_initialization_exception);
        if(present_mode_count == 0) {
            throw render_engine_initialization_exception("No supported present modes... something went really wrong");
        }
        std::vector<VkPresentModeKHR> present_modes(present_mode_count);
        NOVA_THROW_IF_VK_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data()), render_engine_initialization_exception);

        VkSurfaceFormatKHR surface_format = choose_swapchain_format(formats);
        VkPresentModeKHR present_mode = choose_present_mode(present_modes);

        VkSurfaceCapabilitiesKHR capabilities;
        NOVA_THROW_IF_VK_ERROR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities), render_engine_initialization_exception);

        uint32_t image_count = std::max(capabilities.minImageCount, (uint32_t) 3);
        if(capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
            image_count = capabilities.maxImageCount;
        }

        VkExtent2D extend = choose_swapchain_extend();

        VkSwapchainCreateInfoKHR swapchain_create_info;
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.pNext = nullptr;
        swapchain_create_info.flags = 0;
        swapchain_create_info.surface = surface;
        swapchain_create_info.minImageCount = image_count;
        swapchain_create_info.imageFormat = surface_format.format;
        swapchain_create_info.imageColorSpace = surface_format.colorSpace;
        swapchain_create_info.imageExtent = extend;
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
        swapchain_create_info.preTransform = capabilities.currentTransform;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_create_info.presentMode = present_mode;
        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

        NOVA_THROW_IF_VK_ERROR(vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain), render_engine_initialization_exception);

        swapchain_images.clear();
        vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
        swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images.data());
        swapchain_format = surface_format.format;
        swapchain_extend = extend;
    }

    VkSurfaceFormatKHR vulkan_render_engine::choose_swapchain_format(const std::vector<VkSurfaceFormatKHR> &available) {
        if(available.size() == 1 && available.at(0).format == VK_FORMAT_UNDEFINED) {
            return {VK_FORMAT_B8G8R8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        }

        for(const auto &format : available) {
            if(format.format == VK_FORMAT_B8G8R8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }

        return available.at(0);
    }

    VkPresentModeKHR vulkan_render_engine::choose_present_mode(const std::vector<VkPresentModeKHR> &available) {
        for(const auto &mode : available) {
            if(mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D vulkan_render_engine::choose_swapchain_extend() {
        VkExtent2D extend;
        extend.width = window->get_window_size().width;
        extend.height = window->get_window_size().height;
        return extend;
    }

    void vulkan_render_engine::create_image_views() {
        swapchain_image_views.resize(swapchain_images.size());

        for(size_t i = 0; i < swapchain_images.size(); i++) {
            VkImageViewCreateInfo image_view_create_info;
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.pNext = nullptr;
            image_view_create_info.flags = 0;
            image_view_create_info.image = swapchain_images.at(i);
            image_view_create_info.format = swapchain_format;
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;

            NOVA_THROW_IF_VK_ERROR(vkCreateImageView(device, &image_view_create_info, nullptr, &swapchain_image_views.at(i)), render_engine_initialization_exception);
        }
    }

    void vulkan_render_engine::create_render_pass() {
        VkAttachmentDescription color_attachment;
        color_attachment.flags = 0;
        color_attachment.format = swapchain_format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_reference;
        color_attachment_reference.attachment = 0;
        color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass_description;
        subpass_description.flags = 0;
        subpass_description.colorAttachmentCount = 1;
        subpass_description.pColorAttachments = &color_attachment_reference;
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.inputAttachmentCount = 0;
        subpass_description.pInputAttachments = nullptr;
        subpass_description.preserveAttachmentCount = 0;
        subpass_description.pPreserveAttachments = nullptr;
        subpass_description.pResolveAttachments = nullptr;
        subpass_description.pDepthStencilAttachment = nullptr;

        VkSubpassDependency image_available_dependency;
        image_available_dependency.dependencyFlags = 0;
        image_available_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        image_available_dependency.dstSubpass = 0;
        image_available_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        image_available_dependency.srcAccessMask = 0;
        image_available_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        image_available_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo render_pass_create_info;
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.pNext = nullptr;
        render_pass_create_info.flags = 0;
        render_pass_create_info.attachmentCount = 1;
        render_pass_create_info.pAttachments = &color_attachment;
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass_description;
        render_pass_create_info.dependencyCount = 1;
        render_pass_create_info.pDependencies = &image_available_dependency;

        NOVA_THROW_IF_VK_ERROR(vkCreateRenderPass(device, &render_pass_create_info, nullptr, &render_pass), render_engine_initialization_exception);
    }

    void vulkan_render_engine::create_graphics_pipelines() {
        std::queue<pipeline_data> queued_data(std::deque(shaderpack.pipelines.begin(), shaderpack.pipelines.end()));

        uint64_t noop_count = 0;
        while (!queued_data.empty()) {
            // TODO: Ugliest sorting ever, but I have no idea how to improve this right now
            pipeline_data data = queued_data.front();
            queued_data.pop();
            if(!(data.parent_name && std::find(pipelines.begin(), pipelines.end(), data.parent_name.value()) != pipelines.end())) {
                if(noop_count >= queued_data.size()) {
                    NOVA_LOG(ERROR) << "Unresolved parent '" << data.parent_name.value() << " for pipeline " << data.name;
                    while (!queued_data.empty()) {
                        NOVA_LOG(ERROR) << "Unresolved parent '" << data.parent_name.value() << " for pipeline " << data.name;
                    }
                    throw render_engine_initialization_exception("Pipelines with unresolved parents left over!");
                }
                queued_data.push(data);
                noop_count++;
                continue;
            }
            noop_count = 0;

            vk_pipeline nova_pipeline;
            nova_pipeline.nova_data = data;

            std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

            std::unordered_map<std::string, VkShaderModule> shader_modules;
            for(const std::pair<const std::string, std::vector<uint32_t>> &pair : data.sources) {
                VkShaderModuleCreateInfo shader_module_create_info;
                shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                shader_module_create_info.pNext = nullptr;
                shader_module_create_info.flags = 0;
                shader_module_create_info.pCode = pair.second.data();
                shader_module_create_info.codeSize = pair.second.size();

                VkShaderModule module;
                NOVA_THROW_IF_VK_ERROR(vkCreateShaderModule(device, &shader_module_create_info, nullptr, &module), render_engine_initialization_exception);
                shader_modules.insert(std::make_pair(pair.first, module));

                all_shader_modules.push_back(module);
            }

            for(const std::pair<const std::string, VkShaderModule> &pair : shader_modules) {
                VkPipelineShaderStageCreateInfo shader_stage_create_info;
                shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shader_stage_create_info.pNext = nullptr;
                shader_stage_create_info.flags = 0;
                if(ends_with(pair.first, ".vert")) { // TODO: Maybe more endings?
                    shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
                } else if(ends_with(pair.first, ".frag")) {
                    shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                } else {
                    throw render_engine_initialization_exception("Failed to determine shadertype for file extension " + pair.first);
                }
                shader_stage_create_info.module = pair.second;
                shader_stage_create_info.pName = "main";
                shader_stage_create_info.pSpecializationInfo = nullptr;

                shader_stages.push_back(shader_stage_create_info);
            }

            auto vertex_binding_description = vulkan::vulkan_vertex::get_binding_description();
            auto vertex_attribute_description = vulkan::vulkan_vertex::get_attribute_description();

            VkPipelineVertexInputStateCreateInfo vertext_input_state_create_info;
            vertext_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertext_input_state_create_info.pNext = nullptr;
            vertext_input_state_create_info.flags = 0;
            vertext_input_state_create_info.vertexBindingDescriptionCount = 1;
            vertext_input_state_create_info.pVertexBindingDescriptions = &vertex_binding_description;
            vertext_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_description.size());
            vertext_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_description.data();

            VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info;
            input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_create_info.pNext = nullptr;
            input_assembly_create_info.flags = 0;
            input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

            VkViewport viewport;
            viewport.x = 0;
            viewport.y = 0;
            viewport.width = swapchain_extend.width;
            viewport.height = swapchain_extend.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor;
            scissor.offset = {0, 0};
            scissor.extent = swapchain_extend;

            VkPipelineViewportStateCreateInfo viewport_state_create_info;
            viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state_create_info.pNext = nullptr;
            viewport_state_create_info.flags = 0;
            viewport_state_create_info.viewportCount = 1;
            viewport_state_create_info.pViewports = &viewport;
            viewport_state_create_info.scissorCount = 1;
            viewport_state_create_info.pScissors = &scissor;

            VkPipelineRasterizationStateCreateInfo rasterizer_create_info;
            rasterizer_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer_create_info.pNext = nullptr;
            rasterizer_create_info.flags = 0;
            rasterizer_create_info.depthClampEnable = VK_FALSE;
            rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;
            rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer_create_info.lineWidth = 1.0f;
            rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterizer_create_info.depthBiasEnable = VK_TRUE;
            rasterizer_create_info.depthClampEnable = VK_FALSE;
            rasterizer_create_info.depthBiasConstantFactor = data.depth_bias;
            rasterizer_create_info.depthBiasClamp = 0.0f;
            rasterizer_create_info.depthBiasSlopeFactor = data.slope_scaled_depth_bias;

            VkPipelineMultisampleStateCreateInfo multisample_create_info;
            multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisample_create_info.pNext = nullptr;
            multisample_create_info.flags = 0;
            multisample_create_info.sampleShadingEnable = VK_FALSE;
            multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            multisample_create_info.minSampleShading = 1.0f;
            multisample_create_info.pSampleMask = nullptr;
            multisample_create_info.alphaToCoverageEnable = VK_FALSE;
            multisample_create_info.alphaToOneEnable = VK_FALSE;

            VkPipelineColorBlendAttachmentState color_blend_attachment;
            color_blend_attachment.colorWriteMask =
                    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                    VK_COLOR_COMPONENT_A_BIT;
            color_blend_attachment.blendEnable = VK_TRUE;
            color_blend_attachment.srcColorBlendFactor = vulkan::type_converters::blend_factor(data.source_blend_factor);
            color_blend_attachment.dstColorBlendFactor = vulkan::type_converters::blend_factor(data.destination_blend_factor);
            color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
            color_blend_attachment.srcAlphaBlendFactor = vulkan::type_converters::blend_factor(data.alpha_src);
            color_blend_attachment.dstAlphaBlendFactor = vulkan::type_converters::blend_factor(data.alpha_dst);
            color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

            VkPipelineColorBlendStateCreateInfo color_blend_create_info;
            color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blend_create_info.pNext = nullptr;
            color_blend_create_info.flags = 0;
            color_blend_create_info.logicOpEnable = VK_FALSE;
            color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
            color_blend_create_info.attachmentCount = 1;
            color_blend_create_info.pAttachments = &color_blend_attachment;
            color_blend_create_info.blendConstants[0] = 0.0f;
            color_blend_create_info.blendConstants[1] = 0.0f;
            color_blend_create_info.blendConstants[2] = 0.0f;
            color_blend_create_info.blendConstants[3] = 0.0f;

            VkPipelineLayoutCreateInfo pipeline_layout_create_info;
            pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.pNext = nullptr;
            pipeline_layout_create_info.flags = 0;
            pipeline_layout_create_info.setLayoutCount = 0;
            pipeline_layout_create_info.pSetLayouts = nullptr;
            pipeline_layout_create_info.pushConstantRangeCount = 0;
            pipeline_layout_create_info.pPushConstantRanges = nullptr;

            NOVA_THROW_IF_VK_ERROR(
                    vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &nova_pipeline.vulkan_layout),
                    render_engine_initialization_exception);

            VkGraphicsPipelineCreateInfo pipeline_create_info;
            pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_create_info.pNext = nullptr;
            pipeline_create_info.flags = 0;
            pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
            pipeline_create_info.pStages = shader_stages.data();
            pipeline_create_info.pVertexInputState = &vertext_input_state_create_info;
            pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
            pipeline_create_info.pViewportState = &viewport_state_create_info;
            pipeline_create_info.pRasterizationState = &rasterizer_create_info;
            pipeline_create_info.pMultisampleState = &multisample_create_info;
            pipeline_create_info.pDepthStencilState = nullptr;
            pipeline_create_info.pColorBlendState = &color_blend_create_info;
            pipeline_create_info.pDynamicState = nullptr;
            pipeline_create_info.layout = nova_pipeline.vulkan_layout;
            pipeline_create_info.renderPass = /* TODO */;
            pipeline_create_info.subpass = 0;
            if(data.parent_name) {
                pipeline_create_info.basePipelineHandle = pipelines.at(data.parent_name.value()).vulkan_pipeline;
            } else {
                pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
            }
            pipeline_create_info.basePipelineIndex = -1;

            NOVA_THROW_IF_VK_ERROR(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr,
                                                             &nova_pipeline.vulkan_pipeline),
                                   render_engine_initialization_exception);
            pipelines.insert(std::make_pair(data.name, nova_pipeline));
        }
    }

    void vulkan_render_engine::create_framebuffers() {
        swapchain_framebuffers.resize(swapchain_image_views.size());
        for(size_t i = 0; i < swapchain_framebuffers.size(); i++) {
            VkImageView attachments[] = {swapchain_image_views[i]};
            VkFramebufferCreateInfo framebuffer_create_info;
            framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.pNext = nullptr;
            framebuffer_create_info.flags = 0;
            framebuffer_create_info.renderPass = render_pass;
            framebuffer_create_info.attachmentCount = 1;
            framebuffer_create_info.pAttachments = attachments;
            framebuffer_create_info.width = swapchain_extend.width;
            framebuffer_create_info.height = swapchain_extend.height;
            framebuffer_create_info.layers = 1;

            NOVA_THROW_IF_VK_ERROR(vkCreateFramebuffer(device, &framebuffer_create_info, nullptr, &swapchain_framebuffers.at(i)), render_engine_initialization_exception);
        }
    }

    void vulkan_render_engine::create_command_pool() {
        VkCommandPoolCreateInfo command_pool_create_info;
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.pNext = nullptr;
        command_pool_create_info.flags = 0;
        command_pool_create_info.queueFamilyIndex = graphics_queue_index;

        NOVA_THROW_IF_VK_ERROR(vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool), render_engine_initialization_exception);
    }

    void vulkan_render_engine::create_vertex_buffer() {
        VkBufferCreateInfo buffer_create_info;
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.pNext = nullptr;
        buffer_create_info.flags = 0;
        buffer_create_info.size = sizeof(verticies[0]) * verticies.size();
        buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_create_info.queueFamilyIndexCount = 0;
        buffer_create_info.pQueueFamilyIndices = nullptr;

        VmaAllocationCreateInfo allocation_create_info;
        allocation_create_info.flags = 0;
        allocation_create_info.usage = VMA_MEMORY_USAGE_UNKNOWN;
        allocation_create_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        allocation_create_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        allocation_create_info.memoryTypeBits = 0;
        allocation_create_info.pUserData = nullptr;
        allocation_create_info.pool = VK_NULL_HANDLE;

        NOVA_THROW_IF_VK_ERROR(vmaCreateBuffer(memory_allocator, &buffer_create_info, &allocation_create_info, &vertex_buffer, &vertex_buffer_allocation,
                        nullptr), render_engine_initialization_exception);

        // vmaBindBufferMemory(memory_allocator, vertex_buffer_allocation, vertex_buffer);

        void *data;
        vmaMapMemory(memory_allocator, vertex_buffer_allocation, &data);
        std::memcpy(data, verticies.data(), (size_t) buffer_create_info.size);
        vmaUnmapMemory(memory_allocator, vertex_buffer_allocation);
    }

    void vulkan_render_engine::create_command_buffers() {
        command_buffers.resize(swapchain_framebuffers.size());

        VkCommandBufferAllocateInfo buffer_allocate_info;
        buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        buffer_allocate_info.pNext = nullptr;
        buffer_allocate_info.commandPool = command_pool;
        buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        buffer_allocate_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

        NOVA_THROW_IF_VK_ERROR(vkAllocateCommandBuffers(device, &buffer_allocate_info, command_buffers.data()), render_engine_initialization_exception);
    }

    void vulkan_render_engine::create_synchronization_objects() {
        image_available_semaphores.resize(MAX_FRAMES_IN_QUEUE);
        render_finished_semaphores.resize(MAX_FRAMES_IN_QUEUE);
        submit_fences.resize(MAX_FRAMES_IN_QUEUE);

        VkSemaphoreCreateInfo semaphore_create_info;
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphore_create_info.pNext = nullptr;
        semaphore_create_info.flags = 0;

        VkFenceCreateInfo fence_create_info;
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.pNext = nullptr;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for(uint8_t i = 0; i < MAX_FRAMES_IN_QUEUE; i++) {
            NOVA_THROW_IF_VK_ERROR(
                    vkCreateSemaphore(device, &semaphore_create_info, nullptr, &image_available_semaphores.at(i)),
                    render_engine_initialization_exception);
            NOVA_THROW_IF_VK_ERROR(
                    vkCreateSemaphore(device, &semaphore_create_info, nullptr, &render_finished_semaphores.at(i)),
                    render_engine_initialization_exception);
            NOVA_THROW_IF_VK_ERROR(vkCreateFence(device, &fence_create_info, nullptr, &submit_fences.at(i)),
                    render_engine_rendering_exception);
        }
    }

    void vulkan_render_engine::destroy_synchronization_objects() {
        for(uint8_t i = 0; i < MAX_FRAMES_IN_QUEUE; i++) {
            vkDestroySemaphore(device, image_available_semaphores.at(i), nullptr);
            vkDestroySemaphore(device, render_finished_semaphores.at(i), nullptr);
            vkDestroyFence(device, submit_fences.at(i), nullptr);
        }
    }

    void vulkan_render_engine::destroy_vertex_buffer() {
        vmaDestroyBuffer(memory_allocator, vertex_buffer, vertex_buffer_allocation);
    }

    void vulkan_render_engine::destroy_command_pool() {
        vkDestroyCommandPool(device, command_pool, nullptr);
    }

    void vulkan_render_engine::destroy_framebuffers() {
        for(auto framebuffer : swapchain_framebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
    }

    void vulkan_render_engine::destroy_graphics_pipelines() {
        for (const auto &[_, pipeline] : pipelines) {
            vkDestroyPipeline(device, pipeline.vulkan_pipeline, nullptr);
            vkDestroyPipelineLayout(device, pipeline.vulkan_layout, nullptr);
        }
    }

    void vulkan_render_engine::destroy_render_pass() {
        vkDestroyRenderPass(device, render_pass, nullptr);
    }

    void vulkan_render_engine::cleanup_dynamic() {

    }

    void vulkan_render_engine::destroy_image_views() {
        for(auto image_view : swapchain_image_views) {
            vkDestroyImageView(device, image_view, nullptr);
        }
    }

    void vulkan_render_engine::destroy_swapchain() {
        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }

    void vulkan_render_engine::destroy_memory_allocator() {
        vmaDestroyAllocator(memory_allocator);
    }

    void vulkan_render_engine::destroy_device() {
        vkDestroyDevice(device, nullptr);
    }

    void vulkan_render_engine::DEBUG_record_command_buffers() {
        for(size_t i = 0; i < command_buffers.size(); i++) {
            VkCommandBufferBeginInfo buffer_begin_info;
            buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            buffer_begin_info.pNext = nullptr;
            buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            buffer_begin_info.pInheritanceInfo = nullptr;

            NOVA_THROW_IF_VK_ERROR(vkBeginCommandBuffer(command_buffers.at(i), &buffer_begin_info), render_engine_initialization_exception);

            VkRenderPassBeginInfo render_pass_begin_info;
            render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_begin_info.pNext = nullptr;
            render_pass_begin_info.renderPass = render_pass;
            render_pass_begin_info.framebuffer = swapchain_framebuffers.at(i);
            render_pass_begin_info.renderArea.offset = {0, 0};
            render_pass_begin_info.renderArea.extent = swapchain_extend;

            VkClearValue clear_value;
            clear_value.color = {0.0f, 0.0f, 0.0f, 0.0f};
            render_pass_begin_info.clearValueCount = 1;
            render_pass_begin_info.pClearValues = &clear_value;

            vkCmdBeginRenderPass(command_buffers.at(i), &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(command_buffers.at(i), VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(command_buffers.at(i), 0, 1, &vertex_buffer, offsets);

            vkCmdDraw(command_buffers.at(i), static_cast<uint32_t>(verticies.size()), 1, 0, 0);
            vkCmdEndRenderPass(command_buffers.at(i));
            NOVA_THROW_IF_VK_ERROR(vkEndCommandBuffer(command_buffers.at(i)), render_engine_initialization_exception);
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_render_engine::debug_report_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                                uint64_t object, size_t location, int32_t message_code,
                                                const char *layer_prefix, const char *message, void *user_data) {
        NOVA_LOG(TRACE) << __FILE__ << ":" << __LINE__ << " >> VK Debug: [" << layer_prefix << "]" << message;
        return VK_FALSE;
    }

    std::shared_ptr<iwindow> vulkan_render_engine::get_window() const {
        return window;
    }

    void vulkan_render_engine::render_frame() {
        vkWaitForFences(device, 1, &submit_fences.at(current_frame), VK_TRUE, std::numeric_limits<uint64_t>::max());

        auto acquire_result = vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint64_t>::max(), image_available_semaphores.at(current_frame), VK_NULL_HANDLE,
                &current_swapchain_index);
        if(acquire_result == VK_ERROR_OUT_OF_DATE_KHR || acquire_result == VK_SUBOPTIMAL_KHR) {
            recreate_swapchain();
            return;
        } else if(acquire_result != VK_SUCCESS) {
            throw render_engine_rendering_exception(
                    std::string(__FILE__) + ":" + std::to_string(__LINE__) +  "=> " + nova::vulkan::vulkan_utils::vk_result_to_string(acquire_result)); \
        }

        vkResetFences(device, 1, &submit_fences.at(current_frame));

        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = nullptr;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &image_available_semaphores.at(current_frame);
        VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submit_info.pWaitDstStageMask = &wait_stages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffers.at(current_swapchain_index);
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &render_finished_semaphores.at(current_frame);
        NOVA_THROW_IF_VK_ERROR(vkQueueSubmit(graphics_queue, 1, &submit_info, submit_fences.at(current_frame)), render_engine_rendering_exception);

        VkPresentInfoKHR present_info;
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.pNext = nullptr;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &render_finished_semaphores.at(current_frame);
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
        present_info.pImageIndices = &current_swapchain_index;
        present_info.pResults = nullptr;
        vkQueuePresentKHR(graphics_queue, &present_info);

        current_frame = (current_frame + 1) % MAX_FRAMES_IN_QUEUE;
    }

    void vulkan_render_engine::recreate_swapchain() {
        vkDeviceWaitIdle(device);
        destroy_framebuffers();
        vkFreeCommandBuffers(device, command_pool, static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
        destroy_graphics_pipelines();
        destroy_render_pass();
        destroy_image_views();
        destroy_swapchain();

        create_swapchain();
        create_image_views();
        create_render_pass();
        create_graphics_pipelines();
        create_framebuffers();
        create_command_buffers();
        DEBUG_record_command_buffers();
    }

    void vulkan_render_engine::set_shaderpack(shaderpack_data data) {
        if(shaderpack_loaded) {
            destroy_synchronization_objects();
            destroy_vertex_buffer();
            destroy_command_pool();
            destroy_framebuffers();
            destroy_graphics_pipelines();
            destroy_shader_modules();
        } else {
            create_render_pass();
            create_shader_modules();
        }

        create_graphics_pipelines();
        create_framebuffers();
        create_command_pool();
        create_vertex_buffer();
        create_command_buffers();
        DEBUG_record_command_buffers();
        create_synchronization_objects();

        shaderpack_loaded = true;
    }

}