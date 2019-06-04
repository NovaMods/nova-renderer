#pragma once

//#ifdef OS_WIN
#if (defined(_WIN32) || defined(__MINGW32__) || defined(_MSC_VER_) || defined(__MINGW64__)) 
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#endif

//#ifdef OS_LNX
#ifdef __linux__
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#endif

//#define VRT_IMPLEMENTATION
#include "utils.hpp"
#include "structs.hpp"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace vkt
{

    class ComputeFramework {
    protected:


        // instance extensions
        std::vector<const char*> wantedExtensions = {
            "VK_KHR_get_physical_device_properties2",
            "VK_KHR_get_surface_capabilities2",
            "VK_KHR_display", "VK_KHR_surface",
            "VK_EXT_direct_mode_display",
            "VK_EXT_swapchain_colorspace"
        };

        // default device extensions
        std::vector<const char*> wantedDeviceExtensions = {
            "VK_EXT_swapchain_colorspace",
            "VK_EXT_external_memory_host",
            "VK_EXT_sample_locations",
            "VK_EXT_conservative_rasterization",
            "VK_EXT_hdr_metadata",
            "VK_EXT_queue_family_foreign",
            "VK_EXT_sampler_filter_minmax",
            "VK_EXT_descriptor_indexing",
            "VK_EXT_scalar_block_layout",

            "VK_AMD_gpu_shader_int16",
            "VK_AMD_gpu_shader_half_float",
            "VK_AMD_gcn_shader",
            "VK_AMD_buffer_marker",
            "VK_AMD_shader_info",
            "VK_AMD_texture_gather_bias_lod",
            "VK_AMD_shader_image_load_store_lod",
            "VK_AMD_shader_trinary_minmax",
            "VK_AMD_draw_indirect_count",

            "VK_KHR_16bit_storage",
            "VK_KHR_8bit_storage",
            "VK_KHR_incremental_present",
            "VK_KHR_push_descriptor",
            "VK_KHR_swapchain",
            "VK_KHR_sampler_ycbcr_conversion",
            "VK_KHR_image_format_list",
            "VK_KHR_shader_draw_parameters",
            "VK_KHR_variable_pointers",
            "VK_KHR_dedicated_allocation",
            "VK_KHR_relaxed_block_layout",
            "VK_KHR_descriptor_update_template",
            "VK_KHR_sampler_mirror_clamp_to_edge",
            "VK_KHR_storage_buffer_storage_class",
            "VK_KHR_vulkan_memory_model",
            "VK_KHR_dedicated_allocation",
            "VK_KHR_driver_properties",
            "VK_KHR_get_memory_requirements2",
            "VK_KHR_bind_memory2",
            "VK_KHR_maintenance1",
            "VK_KHR_maintenance2",
            "VK_KHR_maintenance3",
            "VK_KHX_shader_explicit_arithmetic_types",
            "VK_KHR_shader_atomic_int64",
            "VK_KHR_shader_float16_int8",
            "VK_KHR_shader_float_controls",

            "VK_NV_compute_shader_derivatives",
            "VK_NV_corner_sampled_image",
            "VK_NV_shader_image_footprint",
            "VK_NV_shader_subgroup_partitioned",

            "VK_NV_ray_tracing",
        };

        // instance layers
        std::vector<const char*> wantedLayers = {
            "VK_LAYER_LUNARG_assistant_layer",
            "VK_LAYER_LUNARG_standard_validation",
            "VK_LAYER_LUNARG_parameter_validation",
            "VK_LAYER_LUNARG_core_validation",

            //"VK_LAYER_LUNARG_api_dump",
            //"VK_LAYER_LUNARG_object_tracker",
            //"VK_LAYER_LUNARG_device_simulation",
            //"VK_LAYER_GOOGLE_threading",
            //"VK_LAYER_GOOGLE_unique_objects"
            //"VK_LAYER_RENDERDOC_Capture"
        };

        // default device layers
        std::vector<const char*> wantedDeviceValidationLayers = {
            "VK_LAYER_AMD_switchable_graphics"
        };


    public:
        ComputeFramework() {};

        vk::Queue queue = {};
        vk::Device device = {};
        vk::Instance instance = {};
        vk::PhysicalDevice physicalDevice = {};
        vk::Fence fence = {};
        vk::CommandPool commandPool = {};
        vk::RenderPass renderpass = {};
        uint32_t queueFamilyIndex = 0;

        std::vector<vk::PhysicalDevice> physicalDevices = {};
        std::vector<uint32_t> queueFamilyIndices = {};

        //vk::Device createDevice(bool isComputePrior = true, std::string shaderPath = "./", bool enableAdvancedAcceleration = true);

        const vk::PhysicalDevice& getPhysicalDevice(const uint32_t& gpuID) { return (physicalDevice = physicalDevices[gpuID]); };
        const vk::PhysicalDevice& getPhysicalDevice() const { return this->physicalDevice; };
        const vk::Device& getDevice() const { return this->device; };
        const vk::Queue& getQueue() const { return this->queue; };
        const vk::Fence& getFence() const { return this->fence; };
        const vk::Instance& getInstance() const { return this->instance; };
        const vk::CommandPool& getCommandPool() const { return this->commandPool; };

        void submitCommandWithSync(const vk::CommandBuffer & cmdBuf) {
            // submit command
            vk::SubmitInfo sbmi = {};
            sbmi.commandBufferCount = 1;//cmdBuffers.size();
            sbmi.pCommandBuffers = &cmdBuf;

            // submit commands
            auto fence = getFence(); {
                getQueue().submit(sbmi, fence);
                device.waitForFences({ fence }, true, INT32_MAX);
            };
            device.resetFences({ 1, &fence });
        }

        struct SurfaceWindow {
            SurfaceFormat surfaceFormat = {};
            vk::Extent2D surfaceSize = vk::Extent2D{ 0u, 0u };
            vk::SurfaceKHR surface = {};
            GLFWwindow* window = nullptr;
        } applicationWindow = {};

    public:
        vk::Instance createInstance() {

#ifdef VOLK_H_
            volkInitialize();
#endif

            auto supportedVkApiVersion = 0u;
            auto apiResult = vkEnumerateInstanceVersion(&supportedVkApiVersion);
            if (supportedVkApiVersion < VK_MAKE_VERSION(1, 1, 0)) return instance;

            // get required extensions
            unsigned int glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            // add glfw extensions to list
            for (uint32_t i = 0; i < glfwExtensionCount; i++) {
                wantedExtensions.push_back(glfwExtensions[i]);
            }

            // get our needed extensions
            auto installedExtensions = vk::enumerateInstanceExtensionProperties();
            auto extensions = std::vector<const char*>();
            for (auto w : wantedExtensions) {
                for (auto i : installedExtensions)
                {
                    if (std::string(i.extensionName).compare(w) == 0)
                    {
                        extensions.emplace_back(w);
                        break;
                    }
                }
            }

            // get validation layers
            auto installedLayers = vk::enumerateInstanceLayerProperties();
            auto layers = std::vector<const char*>();
            for (auto w : wantedLayers) {
                for (auto i : installedLayers)
                {
                    if (std::string(i.layerName).compare(w) == 0)
                    {
                        layers.emplace_back(w);
                        break;
                    }
                }
            }

            // app info
#ifdef VRT_ENABLE_VEZ_INTEROP
            auto appinfo = VezApplicationInfo{};
#else
            auto appinfo = VkApplicationInfo(vk::ApplicationInfo{});
#endif
            appinfo.pNext = nullptr;
            appinfo.pApplicationName = "VKTest";
#ifndef VRT_ENABLE_VEZ_INTEROP
            appinfo.apiVersion = VK_MAKE_VERSION(1, 1, 106);
#endif

            // create instance info
#ifdef VRT_ENABLE_VEZ_INTEROP
            auto cinstanceinfo = VezInstanceCreateInfo{};
#else
            auto cinstanceinfo = VkInstanceCreateInfo(vk::InstanceCreateInfo{});
#endif

            cinstanceinfo.pApplicationInfo = &appinfo;
            cinstanceinfo.enabledExtensionCount = extensions.size();
            cinstanceinfo.ppEnabledExtensionNames = extensions.data();
            cinstanceinfo.enabledLayerCount = layers.size();
            cinstanceinfo.ppEnabledLayerNames = layers.data();

            // create instance
#ifdef VRT_ENABLE_VEZ_INTEROP
            vezCreateInstance(&cinstanceinfo, (VkInstance*)& instance);
#else
            vkCreateInstance(&cinstanceinfo, {}, (VkInstance*)& instance);
#endif

#ifdef VOLK_H_
            volkLoadInstance(instance);
#endif

            // get physical device for application
            physicalDevices = instance.enumeratePhysicalDevices();

            return instance;
        };

        vk::Device createDevice(bool isComputePrior, std::string shaderPath, bool enableAdvancedAcceleration) {

            // use extensions
            auto deviceExtensions = std::vector<const char*>();
            auto gpuExtensions = physicalDevice.enumerateDeviceExtensionProperties();
            for (auto w : wantedDeviceExtensions) {
                for (auto i : gpuExtensions) {
                    if (std::string(i.extensionName).compare(w) == 0) {
                        deviceExtensions.emplace_back(w); break;
                    };
                };
            };

            // use layers
            auto layers = std::vector<const char*>();
            auto deviceValidationLayers = std::vector<const char*>();
            auto gpuLayers = physicalDevice.enumerateDeviceLayerProperties();
            for (auto w : wantedLayers) {
                for (auto i : gpuLayers) {
                    if (std::string(i.layerName).compare(w) == 0) {
                        layers.emplace_back(w); break;
                    };
                };
            };

            // minimal features
            auto gStorage16 = vk::PhysicalDevice16BitStorageFeatures{};
            auto gStorage8 = vk::PhysicalDevice8BitStorageFeaturesKHR{};
            auto gDescIndexing = vk::PhysicalDeviceDescriptorIndexingFeaturesEXT{};
            gStorage16.pNext = &gStorage8;
            gStorage8.pNext = &gDescIndexing;

            auto gFeatures = vk::PhysicalDeviceFeatures2{};
            gFeatures.pNext = &gStorage16;
            gFeatures.features.shaderInt16 = true;
            gFeatures.features.shaderInt64 = true;
            gFeatures.features.shaderUniformBufferArrayDynamicIndexing = true;
            physicalDevice.getFeatures2(&gFeatures);

            // get features and queue family properties
            //auto gpuFeatures = gpu.getFeatures();
            auto gpuQueueProps = physicalDevice.getQueueFamilyProperties();

            // queue family initial
            float priority = 1.0f;
            uint32_t computeFamilyIndex = -1, graphicsFamilyIndex = -1;
            auto queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>();

            // compute/graphics queue family
            for (auto queuefamily : gpuQueueProps) {
                graphicsFamilyIndex++;
                if (queuefamily.queueFlags & (vk::QueueFlagBits::eCompute) && queuefamily.queueFlags & (vk::QueueFlagBits::eGraphics) && physicalDevice.getSurfaceSupportKHR(graphicsFamilyIndex, surface())) {
                    queueCreateInfos.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags()).setQueueFamilyIndex(graphicsFamilyIndex).setQueueCount(1).setPQueuePriorities(&priority));
                    queueFamilyIndices.push_back(graphicsFamilyIndex);
                    break;
                };
            };

            // if have supported queue family, then use this device
            if (queueCreateInfos.size() > 0) {
                // create device
                this->physicalDevice = physicalDevice;
                this->device = physicalDevice.createDevice(vk::DeviceCreateInfo().setFlags(vk::DeviceCreateFlags())
                    .setPNext(&gFeatures) //.setPEnabledFeatures(&gpuFeatures)
                    .setPQueueCreateInfos(queueCreateInfos.data()).setQueueCreateInfoCount(queueCreateInfos.size())
                    .setPpEnabledExtensionNames(deviceExtensions.data()).setEnabledExtensionCount(deviceExtensions.size())
                    .setPpEnabledLayerNames(deviceValidationLayers.data()).setEnabledLayerCount(deviceValidationLayers.size()));
            };

            // return device with queue pointer
            const uint32_t qptr = 0;
            this->fence = this->device.createFence(vk::FenceCreateInfo().setFlags({}));
            this->queueFamilyIndex = queueFamilyIndices[qptr];
            this->commandPool = this->device.createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer), queueFamilyIndex));
            this->queue = this->device.getQueue(queueFamilyIndex, 0); // deferred getting of queue

            // 
            return device;
        }

        // create window and surface for this application (multi-window not supported)
        inline SurfaceWindow& createWindowSurface(GLFWwindow * window, uint32_t WIDTH, uint32_t HEIGHT, std::string title = "TestApp") {
            applicationWindow.window = window;
            applicationWindow.surfaceSize = vk::Extent2D{ WIDTH, HEIGHT };
            auto result = glfwCreateWindowSurface(instance, applicationWindow.window, nullptr, (VkSurfaceKHR*)& applicationWindow.surface);
            if (result != VK_SUCCESS) { glfwTerminate(); exit(result); };
            return applicationWindow;
        }

        // create window and surface for this application (multi-window not supported)
        inline SurfaceWindow& createWindowSurface(uint32_t WIDTH, uint32_t HEIGHT, std::string title = "TestApp") {
            applicationWindow.window = glfwCreateWindow(WIDTH, HEIGHT, title.c_str(), nullptr, nullptr);
            applicationWindow.surfaceSize = vk::Extent2D{ WIDTH, HEIGHT };
            auto result = glfwCreateWindowSurface(instance, applicationWindow.window, nullptr, (VkSurfaceKHR*)& applicationWindow.surface);
            if (result != VK_SUCCESS) { glfwTerminate(); exit(result); };
            return applicationWindow;
        }

        // getters
        vk::SurfaceKHR surface() const {
            return applicationWindow.surface;
        }

        GLFWwindow* window() const {
            return applicationWindow.window;
        }

        const SurfaceFormat& format() const {
            return applicationWindow.surfaceFormat;
        }

        const vk::Extent2D& size() const {
            return applicationWindow.surfaceSize;
        }


        // setters
        void format(SurfaceFormat format) {
            applicationWindow.surfaceFormat = format;
        }

        void size(const vk::Extent2D & size) {
            applicationWindow.surfaceSize = size;
        }


        inline SurfaceFormat getSurfaceFormat(vk::PhysicalDevice gpu)
        {
            auto surfaceFormats = gpu.getSurfaceFormatsKHR(applicationWindow.surface);

            const std::vector<vk::Format> preferredFormats = { vk::Format::eR8G8B8A8Unorm, vk::Format::eB8G8R8A8Unorm };

            vk::Format surfaceColorFormat =
                surfaceFormats.size() == 1 &&
                surfaceFormats[0].format == vk::Format::eUndefined
                ? vk::Format::eR8G8B8A8Unorm
                : surfaceFormats[0].format;

            // search preferred surface format support
            bool surfaceFormatFound = false;
            uint32_t surfaceFormatID = 0;
            for (int i = 0; i < preferredFormats.size(); i++)
            {
                if (surfaceFormatFound) break;
                for (int f = 0; f < surfaceFormats.size(); f++)
                {
                    if (surfaceFormats[f].format == preferredFormats[i])
                    {
                        surfaceFormatFound = true;
                        surfaceFormatID = f;
                        break;
                    }
                }
            }

            // get supported color format
            surfaceColorFormat = surfaceFormats[surfaceFormatID].format;
            vk::ColorSpaceKHR surfaceColorSpace = surfaceFormats[surfaceFormatID].colorSpace;

            // get format properties?
            auto formatProperties = gpu.getFormatProperties(surfaceColorFormat);

            // only if these depth formats
            std::vector<vk::Format> depthFormats = {
                vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat,
                vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint,
                vk::Format::eD16Unorm };

            // choice supported depth format
            vk::Format surfaceDepthFormat = depthFormats[0];
            for (auto format : depthFormats) {
                auto depthFormatProperties = gpu.getFormatProperties(format);
                if (depthFormatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
                    surfaceDepthFormat = format; break;
                }
            }

            // return format result
            SurfaceFormat sfd = {};
            sfd.colorSpace = surfaceColorSpace;
            sfd.colorFormat = surfaceColorFormat;
            sfd.depthFormat = surfaceDepthFormat;
            sfd.colorFormatProperties = formatProperties; // get properties about format
            return sfd;
        }

        inline vk::RenderPass createRenderpass(const std::shared_ptr<radx::Device> & devp)
        {
            auto formats = applicationWindow.surfaceFormat;

            // attachments
            std::vector<vk::AttachmentDescription> attachmentDescriptions = {

                vk::AttachmentDescription()
                    .setFormat(formats.colorFormat)
                    .setSamples(vk::SampleCountFlagBits::e1)
                    .setLoadOp(vk::AttachmentLoadOp::eLoad)
                    .setStoreOp(vk::AttachmentStoreOp::eStore)
                    .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                    .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                    .setInitialLayout(vk::ImageLayout::eUndefined)
                    .setFinalLayout(vk::ImageLayout::ePresentSrcKHR),

                vk::AttachmentDescription()
                    .setFormat(formats.depthFormat)
                    .setSamples(vk::SampleCountFlagBits::e1)
                    .setLoadOp(vk::AttachmentLoadOp::eClear)
                    .setStoreOp(vk::AttachmentStoreOp::eDontCare)
                    .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                    .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                    .setInitialLayout(vk::ImageLayout::eUndefined)
                    .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)

            };

            // attachments references
            std::vector<vk::AttachmentReference> colorReferences = { vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal) };
            std::vector<vk::AttachmentReference> depthReferences = { vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal) };

            // subpasses desc
            std::vector<vk::SubpassDescription> subpasses = {
                vk::SubpassDescription()
                    .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                    .setPColorAttachments(colorReferences.data())
                    .setColorAttachmentCount(colorReferences.size())
                    .setPDepthStencilAttachment(depthReferences.data()) };

            // dependency
            std::vector<vk::SubpassDependency> dependencies = {
                vk::SubpassDependency()
                    .setDependencyFlags(vk::DependencyFlagBits::eByRegion)
                    .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                    .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                     vk::PipelineStageFlagBits::eBottomOfPipe |
                                     vk::PipelineStageFlagBits::eTransfer)
                    .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)

                    .setDstSubpass(0)
                    .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead |
                                      vk::AccessFlagBits::eColorAttachmentWrite),

                vk::SubpassDependency()
                    .setDependencyFlags(vk::DependencyFlagBits::eByRegion)
                    .setSrcSubpass(0)
                    .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                    .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead |
                                      vk::AccessFlagBits::eColorAttachmentWrite)

                    .setDstSubpass(VK_SUBPASS_EXTERNAL)
                    .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                     vk::PipelineStageFlagBits::eTopOfPipe |
                                     vk::PipelineStageFlagBits::eTransfer)
                    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead |
                                      vk::AccessFlagBits::eColorAttachmentWrite)

            };

            // create renderpass
            return (renderpass = device.createRenderPass(vk::RenderPassCreateInfo(
                vk::RenderPassCreateFlags(), attachmentDescriptions.size(),
                attachmentDescriptions.data(), subpasses.size(), subpasses.data(),
                dependencies.size(), dependencies.data())));
        }

        // update swapchain framebuffer
        inline void updateSwapchainFramebuffer(const std::shared_ptr<radx::Device> & devp, vk::SwapchainKHR & swapchain, vk::RenderPass & renderpass, std::vector<Framebuffer> & swapchainBuffers)
        {
            // The swapchain handles allocating frame images.
            auto formats = applicationWindow.surfaceFormat;
            auto gpuMemoryProps = physicalDevice.getMemoryProperties();

#ifdef VRT_ENABLE_VEZ_INTEROP
            auto imageInfoVK = VezImageCreateInfo{};
#else
            auto imageInfoVK = VkImageCreateInfo(vk::ImageCreateInfo{});
            imageInfoVK.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfoVK.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfoVK.flags = 0;
#endif

            imageInfoVK.pNext = nullptr;
            imageInfoVK.arrayLayers = 1;
            imageInfoVK.extent = VkExtent3D{ applicationWindow.surfaceSize.width, applicationWindow.surfaceSize.height, 1 };
            imageInfoVK.format = VkFormat(formats.depthFormat);
            imageInfoVK.imageType = VK_IMAGE_TYPE_2D;
            imageInfoVK.mipLevels = 1;
            //imageInfoVK.pQueueFamilyIndices = &queue->device->queues[1]->familyIndex;
            //imageInfoVK.queueFamilyIndexCount = 1;
            imageInfoVK.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfoVK.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfoVK.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

            VkImage depthImage = {};
#ifdef VRT_ENABLE_VEZ_INTEROP
            vezCreateImage(queue->device->logical, VEZ_MEMORY_GPU_ONLY, &imageInfoVK, &depthImage);
#else
            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            VmaAllocation _allocation = {};
            vmaCreateImage(*devp, &imageInfoVK, &allocCreateInfo, &depthImage, &_allocation, nullptr); // allocators planned structs
#endif

            // image view for usage
#ifdef VRT_ENABLE_VEZ_INTEROP
            auto vinfo = VezImageViewCreateInfo{};
            vinfo.subresourceRange = VezImageSubresourceRange{ 0, 1, 0, 1 };
#else
            auto vinfo = VkImageViewCreateInfo(vk::ImageViewCreateInfo{});
            vinfo.subresourceRange = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1 };
            vinfo.flags = 0;
#endif

            vinfo.pNext = nullptr;
            vinfo.components = VkComponentMapping{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
            vinfo.format = VkFormat(formats.depthFormat);
            vinfo.image = depthImage;
            vinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

            VkImageView depthImageView = {};
#ifdef VRT_ENABLE_VEZ_INTEROP
            vezCreateImageView(queue->device->logical, &vinfo, &depthImageView);
#else
            depthImageView = vk::Device(*devp).createImageView(vk::ImageViewCreateInfo(vinfo));
#endif

            auto swapchainImages = vk::Device(*devp).getSwapchainImagesKHR(swapchain);
            swapchainBuffers.resize(swapchainImages.size());
            for (int i = 0; i < swapchainImages.size(); i++)
            { // create framebuffers
                vk::Image image = swapchainImages[i]; // prelink images
                std::array<vk::ImageView, 2> views = {}; // predeclare views
                views[0] = vk::Device(*devp).createImageView(vk::ImageViewCreateInfo{ {}, image, vk::ImageViewType::e2D, formats.colorFormat, vk::ComponentMapping(), vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1} }); // color view
                views[1] = depthImageView; // depth view
                swapchainBuffers[i].frameBuffer = vk::Device(*devp).createFramebuffer(vk::FramebufferCreateInfo{ {}, renderpass, uint32_t(views.size()), views.data(), applicationWindow.surfaceSize.width, applicationWindow.surfaceSize.height, 1 });
            }
        }

        inline std::vector<Framebuffer> createSwapchainFramebuffer(const std::shared_ptr<radx::Device> & devp, vk::SwapchainKHR swapchain, vk::RenderPass renderpass) {
            // framebuffers vector
            std::vector<Framebuffer> swapchainBuffers = {};
            updateSwapchainFramebuffer(devp, swapchain, renderpass, swapchainBuffers);
            for (int i = 0; i < swapchainBuffers.size(); i++)
            { // create semaphore
                swapchainBuffers[i].semaphore = vk::Device(*devp).createSemaphore(vk::SemaphoreCreateInfo());
                swapchainBuffers[i].waitFence = vk::Device(*devp).createFence(vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled));
            }
            return swapchainBuffers;
        }

        // create swapchain template
        inline vk::SwapchainKHR createSwapchain(const std::shared_ptr<radx::Device> & devp)
        {
            vk::SurfaceKHR surface = applicationWindow.surface;
            SurfaceFormat& formats = applicationWindow.surfaceFormat;

            auto surfaceCapabilities = vk::PhysicalDevice(*devp).getSurfaceCapabilitiesKHR(surface);
            auto surfacePresentModes = vk::PhysicalDevice(*devp).getSurfacePresentModesKHR(surface);

            // check the surface width/height.
            if (!(surfaceCapabilities.currentExtent.width == -1 ||
                surfaceCapabilities.currentExtent.height == -1))
            {
                applicationWindow.surfaceSize = surfaceCapabilities.currentExtent;
            }

            // get supported present mode, but prefer mailBox
            auto presentMode = vk::PresentModeKHR::eImmediate;
            std::vector<vk::PresentModeKHR> priorityModes = { vk::PresentModeKHR::eImmediate, vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifoRelaxed, vk::PresentModeKHR::eFifo };

            bool found = false;
            for (auto pm : priorityModes) {
                if (found) break;
                for (auto sfm : surfacePresentModes) { if (pm == sfm) { presentMode = pm; found = true; break; } }
            }

            // swapchain info
            auto swapchainCreateInfo = vk::SwapchainCreateInfoKHR();
            swapchainCreateInfo.surface = surface;
            swapchainCreateInfo.minImageCount = std::min(surfaceCapabilities.maxImageCount, 3u);
            swapchainCreateInfo.imageFormat = formats.colorFormat;
            swapchainCreateInfo.imageColorSpace = formats.colorSpace;
            swapchainCreateInfo.imageExtent = applicationWindow.surfaceSize;
            swapchainCreateInfo.imageArrayLayers = 1;
            swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
            swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
            //swapchainCreateInfo.queueFamilyIndexCount = 1;
            //swapchainCreateInfo.pQueueFamilyIndices = &queue->device->queues[1]->familyIndex;
            swapchainCreateInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
            swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
            swapchainCreateInfo.presentMode = presentMode;
            swapchainCreateInfo.clipped = true;

            // create swapchain
            return vk::Device(*devp).createSwapchainKHR(swapchainCreateInfo, nullptr);
        }
    };

}; // namespace NSM
