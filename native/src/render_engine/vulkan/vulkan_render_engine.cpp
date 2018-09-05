//
// Created by jannis on 30.08.18.
//

#include "vulkan_render_engine.hpp"
#include <vector>
#include <set>
#include "../../util/logger.hpp"
#include <fstream>

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
        destroy_graphics_pipeline();
        destroy_render_pass();
        DEBUG_destroy_shaders();
        destroy_image_views();
        destroy_swapchain();
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
        create_swapchain();
        create_image_views();
        DEBUG_create_shaders();
        create_render_pass();
        create_graphics_pipeline();
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

        VkPhysicalDevice choosen_device = nullptr;
        for(int i = 0; i < device_count; i++) {
            graphics_family_idx = 0xFFFFFFFF;
            VkPhysicalDevice current_device = physical_devices[i];
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(current_device, &properties);

            if(properties.vendorID == 0x8086 && device_count - 1 > i) { // Intel GPU... they are not powerful and we have more available, so skip it
                continue;
            }

            if(!does_device_support_extensions(current_device)) {
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
                if(current_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT && supports_present == VK_TRUE) {
                    graphics_family_idx = j;
                    break;
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

        vkGetDeviceQueue(device, graphics_family_idx, 0, &graphics_queue);

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
        VkExtent2D extend;

        {
            auto window_size = window->get_window_size();
            extend.width = window_size.width;
            extend.height = window_size.height;
        }

        VkSurfaceCapabilitiesKHR capabilities;
        NOVA_THROW_IF_VK_ERROR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities), render_engine_initialization_exception);

        uint32_t image_count = std::max(capabilities.minImageCount, (uint32_t) 3);
        if(capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
            image_count = capabilities.maxImageCount;
        }
        NOVA_LOG(DEBUG) << "Creating swapchain with " << image_count << " images";

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
        NOVA_LOG(DEBUG) << "Swapchain created";

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

    void vulkan_render_engine::destroy_image_views() {
        for(auto image_view : swapchain_image_views) {
            vkDestroyImageView(device, image_view, nullptr);
        }
    }

    void vulkan_render_engine::destroy_swapchain() {
        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }

    void vulkan_render_engine::destroy_device() {
        vkDestroyDevice(device, nullptr);
    }

    void vulkan_render_engine::DEBUG_create_shaders() {
        auto vert_shader_code = DEBUG_read_file("../tests/src/vert.spv");
        auto frag_shader_code = DEBUG_read_file("../tests/src/frag.spv");

        VkShaderModuleCreateInfo vert_shader_create_info;
        vert_shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vert_shader_create_info.pNext = nullptr;
        vert_shader_create_info.flags = 0;
        vert_shader_create_info.codeSize = vert_shader_code.size();
        vert_shader_create_info.pCode = reinterpret_cast<const uint32_t *>(vert_shader_code.data());
        NOVA_THROW_IF_VK_ERROR(vkCreateShaderModule(device, &vert_shader_create_info, nullptr, &vert_shader), render_engine_initialization_exception);

        VkShaderModuleCreateInfo frag_shader_create_info;
        frag_shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        frag_shader_create_info.pNext = nullptr;
        frag_shader_create_info.flags = 0;
        frag_shader_create_info.codeSize = frag_shader_code.size();
        frag_shader_create_info.pCode = reinterpret_cast<const uint32_t *>(frag_shader_code.data());
        NOVA_THROW_IF_VK_ERROR(vkCreateShaderModule(device, &frag_shader_create_info, nullptr, &frag_shader), render_engine_initialization_exception);
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

        VkRenderPassCreateInfo render_pass_create_info;
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.pNext = nullptr;
        render_pass_create_info.flags = 0;
        render_pass_create_info.attachmentCount = 1;
        render_pass_create_info.pAttachments = &color_attachment;
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass_description;
        render_pass_create_info.dependencyCount = 0;
        render_pass_create_info.pDependencies = nullptr;

        NOVA_THROW_IF_VK_ERROR(vkCreateRenderPass(device, &render_pass_create_info, nullptr, &render_pass), render_engine_initialization_exception);
    }

    void vulkan_render_engine::create_graphics_pipeline() {
        VkPipelineShaderStageCreateInfo vert_shader_create_info;
        vert_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_shader_create_info.pNext = nullptr;
        vert_shader_create_info.flags = 0;
        vert_shader_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vert_shader_create_info.module = vert_shader;
        vert_shader_create_info.pName = "main";
        vert_shader_create_info.pSpecializationInfo = nullptr;

        VkPipelineShaderStageCreateInfo frag_shader_create_info;
        frag_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_shader_create_info.pNext = nullptr;
        frag_shader_create_info.flags = 0;
        frag_shader_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_shader_create_info.module = frag_shader;
        frag_shader_create_info.pName = "main";
        frag_shader_create_info.pSpecializationInfo = nullptr;

        VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_create_info, frag_shader_create_info};

        VkPipelineVertexInputStateCreateInfo vertext_input_state_create_info;
        vertext_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertext_input_state_create_info.pNext = nullptr;
        vertext_input_state_create_info.flags = 0;
        vertext_input_state_create_info.vertexBindingDescriptionCount = 0;
        vertext_input_state_create_info.pVertexBindingDescriptions = nullptr;
        vertext_input_state_create_info.vertexAttributeDescriptionCount = 0;
        vertext_input_state_create_info.pVertexAttributeDescriptions = nullptr;

        VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info;
        input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.pNext = nullptr;
        input_assembly_create_info.flags = 0;
        input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
        rasterizer_create_info.depthBiasEnable = VK_FALSE;
        rasterizer_create_info.depthClampEnable = VK_FALSE;
        rasterizer_create_info.depthBiasConstantFactor = 0.0f;
        rasterizer_create_info.depthBiasClamp = VK_FALSE;
        rasterizer_create_info.depthBiasSlopeFactor = 0.0f;

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
        color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
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

        NOVA_THROW_IF_VK_ERROR(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipeline_layout), render_engine_initialization_exception);

        VkGraphicsPipelineCreateInfo pipeline_create_info;
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_create_info.pNext = nullptr;
        pipeline_create_info.flags = 0;
        pipeline_create_info.stageCount = 2;
        pipeline_create_info.pStages = shader_stages;
        pipeline_create_info.pVertexInputState = &vertext_input_state_create_info;
        pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
        pipeline_create_info.pViewportState = &viewport_state_create_info;
        pipeline_create_info.pRasterizationState = &rasterizer_create_info;
        pipeline_create_info.pMultisampleState = &multisample_create_info;
        pipeline_create_info.pDepthStencilState = nullptr;
        pipeline_create_info.pColorBlendState = &color_blend_create_info;
        pipeline_create_info.pDynamicState = nullptr;
        pipeline_create_info.layout = pipeline_layout;
        pipeline_create_info.renderPass = render_pass;
        pipeline_create_info.subpass = 0;
        pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_create_info.basePipelineIndex = -1;

        NOVA_THROW_IF_VK_ERROR(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline), render_engine_initialization_exception);
    }

    void vulkan_render_engine::destroy_graphics_pipeline() {
        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
    }

    void vulkan_render_engine::destroy_render_pass() {
        vkDestroyRenderPass(device, render_pass, nullptr);
    }

    void vulkan_render_engine::DEBUG_destroy_shaders() {
        vkDestroyShaderModule(device, frag_shader, nullptr);
        vkDestroyShaderModule(device, vert_shader, nullptr);
    }

    std::vector<char> vulkan_render_engine::DEBUG_read_file(std::string path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);

        if(!file.is_open()) {
            throw std::runtime_error("Failed to open file");
        }

        auto file_size = static_cast<size_t>(file.tellg());
        std::vector<char> content(file_size);
        file.seekg(0);
        file.read(content.data(), file_size);
        file.close();

        return content;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_render_engine::debug_report_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                                uint64_t object, size_t location, int32_t message_code,
                                                const char *layer_prefix, const char *message, void *user_data) {
        auto *self = reinterpret_cast<vulkan_render_engine *>(user_data);
        NOVA_LOG(TRACE) << __FILE__ << ":" << __LINE__ << " >> VK Debug: [" << layer_prefix << "] " << message;
        return VK_FALSE;
    }

    void vulkan_render_engine::present_swapchain_image() {

    }

    std::shared_ptr<iwindow> vulkan_render_engine::get_window() const {
        return window;
    }

    std::unique_ptr<command_buffer_base> vulkan_render_engine::allocate_command_buffer(command_buffer_type type) {
        return nullptr;
    }

    void vulkan_render_engine::execute_command_buffers(const std::vector<command_buffer_base *> &buffers) {

    }

    void vulkan_render_engine::free_command_buffer(std::unique_ptr<command_buffer_base> buf) {

    }

    std::shared_ptr<iframebuffer> vulkan_render_engine::get_current_swapchain_framebuffer() const {
        return std::shared_ptr<iframebuffer>();
    }

    std::shared_ptr<iresource> vulkan_render_engine::get_current_swapchain_image() const {
        return std::shared_ptr<iresource>();
    }
}
#pragma clang diagnostic pop