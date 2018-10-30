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
#include "../../loading/shaderpack/render_graph_builder.hpp"
#include "../../../3rdparty/SPIRV-Cross/spirv_glsl.hpp"
#include "../dx12/win32_window.hpp"
#include "../../loading/shaderpack/shaderpack_loading.hpp"

namespace nova {
    vulkan_render_engine::vulkan_render_engine(const nova_settings &settings) : render_engine(settings) {
        settings_options options = settings.get_options();
        const auto &version = options.api.vulkan.application_version;

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
#ifdef linux
        enabled_extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif _WIN32
        enabled_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
#error Unsupported Operating system
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
        debug_create_info.pfnCallback = reinterpret_cast<PFN_vkDebugReportCallbackEXT>(&debug_report_callback);
        debug_create_info.pUserData = this;

        NOVA_THROW_IF_VK_ERROR(vkCreateDebugReportCallbackEXT(vk_instance, &debug_create_info, nullptr, &debug_callback), render_engine_initialization_exception);
#endif
    }

    vulkan_render_engine::~vulkan_render_engine() {
        vkDeviceWaitIdle(device);
        cleanup_dynamic();
        destroy_synchronization_objects();
        destroy_command_pool();
        destroy_framebuffers();
        destroy_graphics_pipelines();
        destroy_render_passes();
        destroy_image_views();
        destroy_swapchain();
        destroy_memory_allocator();
        destroy_device();
    }

    void vulkan_render_engine::open_window(uint32_t width, uint32_t height) {
#ifdef linux
        window = std::make_shared<x11_window>(width, height);

        VkXlibSurfaceCreateInfoKHR x_surface_create_info;
        x_surface_create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        x_surface_create_info.pNext = nullptr;
        x_surface_create_info.flags = 0;
        x_surface_create_info.dpy = window->get_display();
        x_surface_create_info.window = window->get_x11_window();

        NOVA_THROW_IF_VK_ERROR(vkCreateXlibSurfaceKHR(vk_instance, &x_surface_create_info, nullptr, &surface), render_engine_initialization_exception);
#elif _WIN32
        window = std::make_shared<win32_window>(width, height);

        VkWin32SurfaceCreateInfoKHR win32_surface_create = {};
        win32_surface_create.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        win32_surface_create.hwnd = window->get_window_handle();

        NOVA_THROW_IF_VK_ERROR(vkCreateWin32SurfaceKHR(vk_instance, &win32_surface_create, nullptr, &surface), render_engine_initialization_exception);
#else
#error Unsuported window system
#endif
        create_device();
        create_memory_allocator();
        create_swapchain();
        create_swapchain_image_views();
    }

    void vulkan_render_engine::create_device() {
        uint32_t device_count;
        NOVA_THROW_IF_VK_ERROR(vkEnumeratePhysicalDevices(vk_instance, &device_count, nullptr), render_engine_initialization_exception);
        auto *physical_devices = new VkPhysicalDevice[device_count];
        NOVA_THROW_IF_VK_ERROR(vkEnumeratePhysicalDevices(vk_instance, &device_count, physical_devices), render_engine_initialization_exception);

        uint32_t graphics_family_idx = 0xFFFFFFFF;
        uint32_t compute_family_idx = 0xFFFFFFFF;
        uint32_t copy_family_idx = 0xFFFFFFFF;

        VkPhysicalDevice chosen_device = nullptr;
        for(uint32_t device_idx = 0; device_idx < device_count; device_idx++) {
            graphics_family_idx = 0xFFFFFFFF;
            VkPhysicalDevice current_device = physical_devices[device_idx];
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(current_device, &properties);

            if(properties.vendorID == 0x8086 && device_count - 1 > device_idx) {  // Intel GPU... they are not powerful and we have more available, so skip it
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
                chosen_device = current_device;
                break;
            }
        }

        if(!chosen_device) {
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

        NOVA_THROW_IF_VK_ERROR(vkCreateDevice(chosen_device, &device_create_info, nullptr, &device), render_engine_initialization_exception);

        graphics_queue_index = graphics_family_idx;
        vkGetDeviceQueue(device, graphics_family_idx, 0, &graphics_queue);
        compute_queue_index = compute_family_idx;
        vkGetDeviceQueue(device, compute_family_idx, 0, &compute_queue);
        copy_queue_index = copy_family_idx;
        vkGetDeviceQueue(device, copy_family_idx, 0, &copy_queue);

        delete[] physical_devices;

        physical_device = chosen_device;
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
        NOVA_THROW_IF_VK_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr), render_engine_initialization_exception);
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
        swapchain_extent = extend;
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

    void vulkan_render_engine::create_swapchain_image_views() {
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

    void vulkan_render_engine::set_shaderpack(const shaderpack_data& data, ftl::TaskScheduler& scheduler) {
        NOVA_LOG(DEBUG) << "Vulkan render engine loading new shaderpack";
        if(shaderpack_loaded) {
            destroy_render_passes();

            destroy_synchronization_objects();
            destroy_command_pool();
            destroy_framebuffers();
            destroy_graphics_pipelines();
            NOVA_LOG(DEBUG) << "Resources from old shaderpacks destroyed";
        }

        dynamic_textures_by_name.clear();
        create_textures(data.resources.textures);
        for(const material_data& mat_data : data.materials) {
            materials[mat_data.name] = mat_data;
        }

        create_render_passes(data.passes);

        create_graphics_pipelines();
        create_framebuffers();
        create_command_pool();
        create_command_buffers();
        create_synchronization_objects();

        shaderpack_loaded = true;
    }

    bool vulkan_render_engine::vk_resource_binding::operator==(const vk_resource_binding& other) const {
        return other.set == set && other.binding == binding && other.descriptorCount == descriptorCount && other.descriptorType == descriptorType;
    }

    bool vulkan_render_engine::vk_resource_binding::operator!=(const vk_resource_binding& other) const {
        return !(*this == other);
    }

    void vulkan_render_engine::create_render_passes(const std::vector<render_pass_data>& passes) {
        NOVA_LOG(DEBUG) << "Flattening frame graph...";

        std::unordered_map<std::string, render_pass_data> regular_render_passes;
        regular_render_passes.reserve(passes.size());
        render_passes_by_name.reserve(passes.size());
        for(const render_pass_data& pass_data : passes) {
            render_passes_by_name[pass_data.name].nova_data = pass_data;
            regular_render_passes[pass_data.name] = pass_data;
        }

        render_passes_by_order = order_passes(regular_render_passes);

        for(const std::string &pass_name : render_passes_by_order) {
            VkSubpassDescription subpass_description;
            subpass_description.flags = 0;
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
            render_pass_create_info.subpassCount= 1;
            render_pass_create_info.pSubpasses = &subpass_description;
            render_pass_create_info.dependencyCount = 1;
            render_pass_create_info.pDependencies = &image_available_dependency;

            std::optional<input_textures> inputs_maybe = render_passes_by_name.at(pass_name).nova_data.texture_inputs;
            if(inputs_maybe) {
                std::vector<std::string>& color_inputs = inputs_maybe->bound_textures;
                auto [attachments, references] = to_vk_attachment_info(color_inputs);

                subpass_description.colorAttachmentCount = static_cast<uint32_t>(references.size());
                subpass_description.pColorAttachments = references.data();

                render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
                render_pass_create_info.pAttachments = attachments.data();
            }

            VkRenderPass render_pass;
            NOVA_THROW_IF_VK_ERROR(vkCreateRenderPass(device, &render_pass_create_info, nullptr, &render_pass), render_engine_initialization_exception);
            render_passes_by_name[pass_name].pass = render_pass;
        }
    }

    void vulkan_render_engine::create_graphics_pipelines() {
        std::queue<pipeline_data> queued_data(std::deque<pipeline_data>(shaderpack.pipelines.begin(), shaderpack.pipelines.end()));

        uint64_t noop_count = 0;
        while(!queued_data.empty()) {
            // TODO: Ugliest sorting ever, but I have no idea how to improve this right now
            pipeline_data data = queued_data.front();
            queued_data.pop();
            if(!(data.parent_name && std::find(pipelines.begin(), pipelines.end(), data.parent_name.value()) != pipelines.end())) {
                if(noop_count >= queued_data.size()) {
                    NOVA_LOG(ERROR) << "Unresolved parent '" << data.parent_name.value() << " for pipeline " << data.name;
                    while(!queued_data.empty()) {
                        queued_data.pop();
                        data = queued_data.front();
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
            std::unordered_map<VkShaderStageFlagBits, VkShaderModule> shader_modules;
            std::unordered_map<std::string, vk_resource_binding> bindings;

            shader_modules[VK_SHADER_STAGE_VERTEX_BIT] = create_shader_module(data.vertex_shader.source);
            get_shader_module_descriptors(data.vertex_shader.source, bindings);

            if(data.geometry_shader) {
                shader_modules[VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT] = create_shader_module(data.geometry_shader->source);
                get_shader_module_descriptors(data.geometry_shader->source, bindings);
            }

            if(data.tessellation_control_shader) {
                shader_modules[VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT] = create_shader_module(data.tessellation_control_shader->source);
                get_shader_module_descriptors(data.tessellation_control_shader->source, bindings);
            }

            if(data.tessellation_evaluation_shader) {
                shader_modules[VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT] = create_shader_module(data.tessellation_evaluation_shader->source);
            }
            get_shader_module_descriptors(data.tessellation_evaluation_shader->source, bindings);

            if(data.fragment_shader) {
                shader_modules[VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT] = create_shader_module(data.fragment_shader->source);
                get_shader_module_descriptors(data.fragment_shader->source, bindings);
            }

            std::vector<VkDescriptorSetLayout> layout_data = create_descriptor_set_layouts(bindings);

            VkPipelineLayoutCreateInfo pipeline_layout_create_info;
            pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.pNext = nullptr;
            pipeline_layout_create_info.flags = 0;
            pipeline_layout_create_info.setLayoutCount = layout_data.size();
            pipeline_layout_create_info.pSetLayouts = layout_data.data();
            pipeline_layout_create_info.pushConstantRangeCount = 0;
            pipeline_layout_create_info.pPushConstantRanges = nullptr;

            NOVA_THROW_IF_VK_ERROR(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &nova_pipeline.layout), render_engine_initialization_exception);

            for(const auto &pair : shader_modules) {
                VkPipelineShaderStageCreateInfo shader_stage_create_info;
                shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shader_stage_create_info.pNext = nullptr;
                shader_stage_create_info.flags = 0;
                shader_stage_create_info.stage = pair.first;
                shader_stage_create_info.module = pair.second;
                shader_stage_create_info.pName = "main";
                shader_stage_create_info.pSpecializationInfo = nullptr;

                shader_stages.push_back(shader_stage_create_info);
            }

            auto vertex_binding_description = vulkan::vulkan_vertex::get_binding_description();
            auto vertex_attribute_description = vulkan::vulkan_vertex::get_attribute_description();

            VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info;
            vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state_create_info.pNext = nullptr;
            vertex_input_state_create_info.flags = 0;
            vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
            vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_binding_description;
            vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_description.size());
            vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_description.data();

            VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info;
            input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_create_info.pNext = nullptr;
            input_assembly_create_info.flags = 0;
            input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

            VkViewport viewport;
            viewport.x = 0;
            viewport.y = 0;
            viewport.width = swapchain_extent.width;
            viewport.height = swapchain_extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor;
            scissor.offset = {0, 0};
            scissor.extent = swapchain_extent;

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
            color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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

            VkGraphicsPipelineCreateInfo pipeline_create_info;
            pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_create_info.pNext = nullptr;
            pipeline_create_info.flags = 0;
            pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
            pipeline_create_info.pStages = shader_stages.data();
            pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
            pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
            pipeline_create_info.pViewportState = &viewport_state_create_info;
            pipeline_create_info.pRasterizationState = &rasterizer_create_info;
            pipeline_create_info.pMultisampleState = &multisample_create_info;
            pipeline_create_info.pDepthStencilState = nullptr;
            pipeline_create_info.pColorBlendState = &color_blend_create_info;
            pipeline_create_info.pDynamicState = nullptr;
            pipeline_create_info.layout = nova_pipeline.layout;
            pipeline_create_info.renderPass = render_passes_by_name.at(data.pass).pass;
            pipeline_create_info.subpass = 0;
            if(data.parent_name) {
                pipeline_create_info.basePipelineHandle = pipelines.at(data.parent_name.value()).pipeline;
            } else {
                pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
            }
            pipeline_create_info.basePipelineIndex = -1;

            NOVA_THROW_IF_VK_ERROR(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &nova_pipeline.pipeline), render_engine_initialization_exception);
            pipelines.insert(std::make_pair(data.name, nova_pipeline));
        }
    }

    VkShaderModule vulkan_render_engine::create_shader_module(std::vector<uint32_t> spirv) const {
        VkShaderModuleCreateInfo shader_module_create_info;
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.pNext = nullptr;
        shader_module_create_info.flags = 0;
        shader_module_create_info.pCode = spirv.data();
        shader_module_create_info.codeSize = spirv.size();

        VkShaderModule module;
        NOVA_THROW_IF_VK_ERROR(vkCreateShaderModule(device, &shader_module_create_info, nullptr, &module),
                               render_engine_initialization_exception);

        return module;
    }

    void vulkan_render_engine::create_framebuffers(const VkRenderPass render_pass) {
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
            framebuffer_create_info.width = swapchain_extent.width;
            framebuffer_create_info.height = swapchain_extent.height;
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
            NOVA_THROW_IF_VK_ERROR(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &image_available_semaphores.at(i)), render_engine_initialization_exception);
            NOVA_THROW_IF_VK_ERROR(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &render_finished_semaphores.at(i)), render_engine_initialization_exception);
            NOVA_THROW_IF_VK_ERROR(vkCreateFence(device, &fence_create_info, nullptr, &submit_fences.at(i)), render_engine_rendering_exception);
        }
    }

    void vulkan_render_engine::destroy_synchronization_objects() {
        for(uint8_t i = 0; i < MAX_FRAMES_IN_QUEUE; i++) {
            vkDestroySemaphore(device, image_available_semaphores.at(i), nullptr);
            vkDestroySemaphore(device, render_finished_semaphores.at(i), nullptr);
            vkDestroyFence(device, submit_fences.at(i), nullptr);
        }
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
        for(const auto &[_, pipeline] : pipelines) {
            vkDestroyPipeline(device, pipeline.pipeline, nullptr);
            vkDestroyPipelineLayout(device, pipeline.layout, nullptr);
        }
    }

    void vulkan_render_engine::destroy_render_passes() {
        for(const auto &render_pass : render_passes_by_name) {
            vkDestroyRenderPass(device, render_pass.second.pass, nullptr);
        }
        render_passes_by_order.clear();
        render_passes_by_name.clear();
    }

    void vulkan_render_engine::cleanup_dynamic() {}

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

    VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_render_engine::debug_report_callback(
        VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t message_code, const char *layer_prefix, const char *message, void *user_data) {
        NOVA_LOG(TRACE) << __FILE__ << ":" << __LINE__ << " >> VK Debug: [" << layer_prefix << "]" << message;
        return VK_FALSE;
    }

    std::shared_ptr<iwindow> vulkan_render_engine::get_window() const {
        return window;
    }

    void vulkan_render_engine::render_frame() {
        vkWaitForFences(device, 1, &submit_fences.at(current_frame), VK_TRUE, std::numeric_limits<uint64_t>::max());

        auto acquire_result = vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint64_t>::max(), image_available_semaphores.at(current_frame), VK_NULL_HANDLE, &current_swapchain_index);
        if(acquire_result == VK_ERROR_OUT_OF_DATE_KHR || acquire_result == VK_SUBOPTIMAL_KHR) {
            recreate_swapchain();
            return;
        } else if(acquire_result != VK_SUCCESS) {
            throw render_engine_rendering_exception(std::string(__FILE__) + ":" + std::to_string(__LINE__) + "=> " + nova::vulkan::vulkan_utils::vk_result_to_string(acquire_result));
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
        destroy_render_passes();
        destroy_image_views();
        destroy_swapchain();

        create_swapchain();
        create_swapchain_image_views();
        create_render_passes();
        create_graphics_pipelines();
        create_framebuffers();
        create_command_buffers();
    }

    std::pair<std::vector<VkAttachmentDescription>, std::vector<VkAttachmentReference>>
    vulkan_render_engine::to_vk_attachment_info(std::vector<std::string> &attachment_names) {
        std::vector<VkAttachmentDescription> attachment_descriptions;
        attachment_descriptions.reserve(attachment_names.size());

        std::vector<VkAttachmentReference> attachment_references;
        attachment_references.reserve(attachment_names.size());

        for(const std::string& name : attachment_names) {
            const vk_texture& tex = dynamic_textures_by_name.at(name);

            VkAttachmentDescription color_attachment;
            color_attachment.flags = 0;
            color_attachment.format = to_vk_format(tex.nova_data.format.pixel_format);
            color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            color_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            attachment_descriptions.push_back(color_attachment);

            VkAttachmentReference color_attachment_reference;
            color_attachment_reference.attachment = static_cast<uint32_t>(attachment_references.size());
            color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachment_references.push_back(color_attachment_reference);
        }

        return {attachment_descriptions, attachment_references};
    }

    VkFormat vulkan_render_engine::to_vk_format(pixel_format_enum format) {
        switch(format) {
            case pixel_format_enum::RGBA8:
                return VK_FORMAT_R8G8B8A8_UNORM;

            case pixel_format_enum::RGBA16F:
                return VK_FORMAT_R16G16B16A16_SFLOAT;

            case pixel_format_enum::RGBA32F:
                return VK_FORMAT_R32G32B32A32_SFLOAT;

            case pixel_format_enum::Depth:
                return VK_FORMAT_D32_SFLOAT;

            case pixel_format_enum::DepthStencil:
                return VK_FORMAT_D24_UNORM_S8_UINT;

        }


        return VK_FORMAT_R10X6G10X6_UNORM_2PACK16;
    }

    void vulkan_render_engine::create_textures(const std::vector<texture_resource_data>& texture_datas) {
        for(const texture_resource_data& texture_data : texture_datas) {
            vk_texture texture;
            texture.nova_data = texture_data;

            VkImageCreateInfo image_create_info = {};
            image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.format = to_vk_format(texture_data.format.pixel_format);
            glm::uvec2 texture_size = texture_data.format.get_size_in_pixels(swapchain_extent);
            image_create_info.extent.width = texture_size.x;
            image_create_info.extent.height = texture_size.y;
            image_create_info.extent.depth = 1;
            image_create_info.mipLevels = 0;
            image_create_info.arrayLayers = 1;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            image_create_info.queueFamilyIndexCount = 1;
            image_create_info.pQueueFamilyIndices = &graphics_queue_index;
            image_create_info.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VmaAllocationCreateInfo alloc_create_info = {};
            alloc_create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            alloc_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            alloc_create_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            alloc_create_info.preferredFlags = 0;
            alloc_create_info.memoryTypeBits = 0;
            alloc_create_info.pool = VK_NULL_HANDLE;
            alloc_create_info.pUserData = nullptr;

            vmaCreateImage(memory_allocator, &image_create_info, &alloc_create_info, &texture.image,
                           &texture.allocation, &texture.vma_info);

            VkImageViewCreateInfo image_view_create_info;
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.image = texture.image;
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.format = image_create_info.format;
            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;

            vkCreateImageView(device, &image_view_create_info, nullptr, &texture.image_view);

            dynamic_textures_by_name[texture_data.name] = texture;
        }

    }

    void vulkan_render_engine::get_shader_module_descriptors(std::vector<uint32_t> spirv,
                                                          std::unordered_map<std::string, vk_resource_binding>& bindings) {
        const spirv_cross::CompilerGLSL shader_compiler(spirv);
        const spirv_cross::ShaderResources resources = shader_compiler.get_shader_resources();

        for(const spirv_cross::Resource& resource : resources.sampled_images) {
            const uint32_t set = shader_compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            const uint32_t binding = shader_compiler.get_decoration(resource.id, spv::DecorationBinding);

            vk_resource_binding new_binding = {};
            new_binding.set = set;
            new_binding.binding = binding;
            new_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            new_binding.descriptorCount = 1;

            if(bindings.find(resource.name) == bindings.end()) {
                // Totally new binding!
                bindings[resource.name] = new_binding;

            } else {
                // Existing binding. Is it the same as our binding?
                const vk_resource_binding& existing_binding = bindings.at(resource.name);
                if(existing_binding != new_binding) {
                    // They have two different bindings with the same name. Not allowed
                    NOVA_LOG(ERROR) << "You have two different uniforms named " << resource.name 
                    << " in different shader stages. This is not allowed. Use unique names";

                } else {
                    // Extend the old binding's shader stages to include this one   
                }
            }

        }
    }

    std::vector<VkDescriptorSetLayout> vulkan_render_engine::create_descriptor_set_layouts(std::unordered_map<std::string, vk_resource_binding> all_bindings) const {
        std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> bindings_by_set;

        for(const auto& named_binding : all_bindings) {
            const vk_resource_binding& binding = named_binding.second;
            VkDescriptorSetLayoutBinding new_binding = {};
            new_binding.binding = binding.binding;
            new_binding.descriptorCount = binding.descriptorCount;
            new_binding.descriptorType = binding.descriptorType;
            new_binding.pImmutableSamplers = binding.pImmutableSamplers;
            new_binding.stageFlags = VK_SHADER_STAGE_ALL;

            bindings_by_set[binding.set].push_back(new_binding);
        }

        std::vector<VkDescriptorSetLayoutCreateInfo> dsl_create_infos = {};
        dsl_create_infos.reserve(bindings_by_set.size());
        for(uint32_t i = 0; i < bindings_by_set.size(); i++) {
            if(bindings_by_set.find(i) == bindings_by_set.end()) {
                NOVA_LOG(ERROR) << "Could not get information for descriptor set " << i << "; most likely you skipped"
                << " a descriptor set in your shader. Ensure that all shaders for this pipeline together don't have"
                << " any gaps in the descriptor sets they declare";
                throw shader_layout_creation_failed("Descriptor set " + std::to_string(i) + " not present");
            }

            const std::vector<VkDescriptorSetLayoutBinding>& bindings = bindings_by_set.at(i);

            VkDescriptorSetLayoutCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            create_info.bindingCount = bindings.size();
            create_info.pBindings = bindings.data();

            dsl_create_infos.push_back(create_info);
        }

        std::vector<VkDescriptorSetLayout> layouts;
        layouts.resize(dsl_create_infos.size());
        for(uint32_t i = 0; i < dsl_create_infos.size(); i++) {
            vkCreateDescriptorSetLayout(device, &dsl_create_infos[i], nullptr, &layouts[i]);
        }

        return layouts;
    }

}  // namespace nova