#include "vulkan_swapchain.hpp"

#include "nova_renderer/util/logger.hpp"

#include "vulkan_render_device.hpp"
#include "vulkan_utils.hpp"

#ifdef ERROR
#undef ERROR
#endif

namespace nova::renderer::rhi {
    VulkanSwapchain::VulkanSwapchain(const uint32_t num_swapchain_images,
                                     VulkanRenderDevice* render_device,
                                     const glm::uvec2 window_dimensions,
                                     const rx::vector<VkPresentModeKHR>& present_modes)
        : Swapchain(num_swapchain_images, window_dimensions), render_device(render_device), num_swapchain_images(num_swapchain_images) {

        create_swapchain(num_swapchain_images, present_modes, window_dimensions);

        rx::vector<VkImage> vk_images = get_swapchain_images();

        if(vk_images.is_empty()) {
            NOVA_LOG(FATAL) << "The swapchain returned zero images";
        }

        swapchain_image_layouts.resize(num_swapchain_images);
        swapchain_image_layouts.each_fwd(
            [&](VkImageLayout& swapchain_image_layout) { swapchain_image_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; });

        // Create a dummy renderpass that writes to a single color attachment - the swapchain
        const VkRenderPass renderpass = create_dummy_renderpass();

        const glm::uvec2 swapchain_size = {swapchain_extent.width, swapchain_extent.height};

        for(uint32_t i = 0; i < num_swapchain_images; i++) {
            create_resources_for_frame(vk_images[i], renderpass, swapchain_size);
        }

        vkDestroyRenderPass(render_device->device, renderpass, nullptr);

        // move the swapchain images into the correct layout cause I guess they aren't for some reason?
        transition_swapchain_images_into_color_attachment_layout(vk_images);
    }

    uint8_t VulkanSwapchain::acquire_next_swapchain_image(rx::memory::allocator* allocator) {
        auto* fence = render_device->create_fence(false, allocator);
        auto* vk_fence = static_cast<VulkanFence*>(fence);

        uint32_t acquired_image_idx;
        const auto acquire_result = vkAcquireNextImageKHR(render_device->device,
                                                          swapchain,
                                                          std::numeric_limits<uint64_t>::max(),
                                                          VK_NULL_HANDLE,
                                                          vk_fence->fence,
                                                          &acquired_image_idx);
        if(acquire_result == VK_ERROR_OUT_OF_DATE_KHR || acquire_result == VK_SUBOPTIMAL_KHR) {
            // TODO: Recreate the swapchain and all screen-relative textures
            NOVA_LOG(ERROR) << "Swapchain out of date! One day you'll write the code to recreate it";
            return 0;
        }
        if(acquire_result != VK_SUCCESS) {
            NOVA_LOG(ERROR) << __FILE__ << ":" << __LINE__ << "=> " << std::to_string(acquire_result);
        }

        // Block until we have the swapchain image in order to mimic D3D12. TODO: Reevaluate this decision
        rx::vector<Fence*> fences;
        fences.push_back(vk_fence);
        render_device->wait_for_fences(fences);

        return static_cast<uint8_t>(acquired_image_idx);
    }

    void VulkanSwapchain::present(const uint32_t image_idx) {
        VkResult swapchain_result = {};

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 0;
        present_info.pWaitSemaphores = nullptr;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
        present_info.pImageIndices = &image_idx;
        present_info.pResults = &swapchain_result;

        vkQueuePresentKHR(render_device->graphics_queue, &present_info);
    }

    void VulkanSwapchain::transition_swapchain_images_into_color_attachment_layout(const rx::vector<VkImage>& images) const {
        rx::vector<VkImageMemoryBarrier> barriers;
        barriers.reserve(images.size());

        images.each_fwd([&](const VkImage& image) {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcQueueFamilyIndex = render_device->graphics_family_index;
            barrier.dstQueueFamilyIndex = render_device->graphics_family_index;
            barrier.image = image;
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Each swapchain image **will** be rendered to before it is
                                                                 // presented
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            barriers.push_back(barrier);
        });

        VkCommandPool command_pool;

        VkCommandPoolCreateInfo command_pool_create_info = {};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.queueFamilyIndex = render_device->graphics_family_index;
        vkCreateCommandPool(render_device->device, &command_pool_create_info, nullptr, &command_pool);

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = command_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer cmds;
        vkAllocateCommandBuffers(render_device->device, &alloc_info, &cmds);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmds, &begin_info);

        vkCmdPipelineBarrier(cmds,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             static_cast<uint32_t>(barriers.size()),
                             barriers.data());

        vkEndCommandBuffer(cmds);

        VkFence transition_done_fence;

        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        vkCreateFence(render_device->device, &fence_create_info, nullptr, &transition_done_fence);

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmds;

        vkQueueSubmit(render_device->graphics_queue, 1, &submit_info, transition_done_fence);
        vkWaitForFences(render_device->device, 1, &transition_done_fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

        vkFreeCommandBuffers(render_device->device, command_pool, 1, &cmds);
    }

    void VulkanSwapchain::deinit() {
        swapchain_images.each_fwd([&](const Image* i) {
            const VulkanImage* vk_image = static_cast<const VulkanImage*>(i);
            vkDestroyImage(render_device->device, vk_image->image, nullptr);
            delete i;
        });
        swapchain_images.clear();

        swapchain_image_views.each_fwd([&](const VkImageView& iv) { vkDestroyImageView(render_device->device, iv, nullptr); });
        swapchain_image_views.clear();

        fences.each_fwd([&](const Fence* f) {
            const VulkanFence* vk_fence = static_cast<const VulkanFence*>(f);
            vkDestroyFence(render_device->device, vk_fence->fence, nullptr);
            delete f;
        });
        fences.clear();
    }

    uint32_t VulkanSwapchain::get_num_images() const { return num_swapchain_images; }

    VkImageLayout VulkanSwapchain::get_layout(const uint32_t frame_idx) { return swapchain_image_layouts[frame_idx]; }

    VkExtent2D VulkanSwapchain::get_swapchain_extent() const { return swapchain_extent; }

    VkFormat VulkanSwapchain::get_swapchain_format() const { return swapchain_format; }

    VkSurfaceFormatKHR VulkanSwapchain::choose_surface_format(const rx::vector<VkSurfaceFormatKHR>& formats) {
        VkSurfaceFormatKHR result;

        if(formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
            result.format = VK_FORMAT_B8G8R8A8_UNORM;
            result.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            return result;
        }

        // We want 32 bit rgba and srgb nonlinear... I think? Will have to read up on it more and figure out what's up
        if(const auto idx = formats.find_if([&](VkSurfaceFormatKHR& fmt) {
               return fmt.format == VK_FORMAT_B8G8R8A8_UNORM && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
           }) != rx::vector<VkSurfaceFormatKHR>::k_npos) {
            return formats[idx];
        }

        // We can't have what we want, so I guess we'll just use what we got
        return formats[0];
    }

    VkPresentModeKHR VulkanSwapchain::choose_present_mode(const rx::vector<VkPresentModeKHR>& modes) {
        const VkPresentModeKHR desired_mode = VK_PRESENT_MODE_MAILBOX_KHR;

        // Mailbox mode is best mode (also not sure why)
        if(modes.find(desired_mode) != rx::vector<VkPresentModeKHR>::k_npos) {
            return desired_mode;
        }

        // FIFO, like FIFA, is forever
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanSwapchain::choose_surface_extent(const VkSurfaceCapabilitiesKHR& caps, const glm::ivec2& window_dimensions) {
        VkExtent2D extent;

        if(caps.currentExtent.width == 0xFFFFFFFF) {
            extent.width = static_cast<uint32_t>(window_dimensions.x);
            extent.height = static_cast<uint32_t>(window_dimensions.y);
        } else {
            extent = caps.currentExtent;
        }

        return extent;
    }

    void VulkanSwapchain::create_swapchain(const uint32_t requested_num_swapchain_images,
                                           const rx::vector<VkPresentModeKHR>& present_modes,
                                           const glm::uvec2& window_dimensions) {

        const auto surface_format = choose_surface_format(render_device->gpu.surface_formats);
        const auto present_mode = choose_present_mode(present_modes);
        const auto extent = choose_surface_extent(render_device->gpu.surface_capabilities, window_dimensions);

        VkSwapchainCreateInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = render_device->surface;

        info.minImageCount = requested_num_swapchain_images;

        info.imageFormat = surface_format.format;
        info.imageColorSpace = surface_format.colorSpace;
        info.imageExtent = extent;
        info.imageArrayLayers = 1;

        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.queueFamilyIndexCount = 1;
        info.pQueueFamilyIndices = &render_device->graphics_family_index;

        info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = present_mode;

        info.clipped = VK_TRUE;

        auto res = vkCreateSwapchainKHR(render_device->device, &info, nullptr, &swapchain);

        NOVA_LOG(ERROR) << res;

        swapchain_format = surface_format.format;
        this->present_mode = present_mode;
        swapchain_extent = extent;
    }

    void VulkanSwapchain::create_resources_for_frame(const VkImage image, const VkRenderPass renderpass, const glm::uvec2& swapchain_size) {
        auto* vk_image = new VulkanImage;
        vk_image->type = ResourceType::Image;
        vk_image->is_dynamic = true;
        vk_image->image = image;

        VkImageViewCreateInfo image_view_create_info = {};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        image_view_create_info.image = image;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = swapchain_format;

        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;

        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;

        vkCreateImageView(render_device->device, &image_view_create_info, nullptr, &vk_image->image_view);
        swapchain_image_views.push_back(vk_image->image_view);

        swapchain_images.push_back(vk_image);

        VkFramebufferCreateInfo framebuffer_create_info = {};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments = &vk_image->image_view;
        framebuffer_create_info.renderPass = renderpass;
        framebuffer_create_info.width = swapchain_extent.width;
        framebuffer_create_info.height = swapchain_extent.height;
        framebuffer_create_info.layers = 1;

        auto* vk_framebuffer = new VulkanFramebuffer;
        vk_framebuffer->size = swapchain_size;
        vk_framebuffer->num_attachments = 1;
        vkCreateFramebuffer(render_device->device, &framebuffer_create_info, nullptr, &vk_framebuffer->framebuffer);

        framebuffers.push_back(vk_framebuffer);

        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VkFence fence;
        vkCreateFence(render_device->device, &fence_create_info, nullptr, &fence);
        fences.push_back(new VulkanFence{{}, fence});
    }

    rx::vector<VkImage> VulkanSwapchain::get_swapchain_images() {
        rx::vector<VkImage> vk_images;

        vkGetSwapchainImagesKHR(render_device->device, swapchain, &num_swapchain_images, nullptr);
        vk_images.resize(num_swapchain_images);
        vkGetSwapchainImagesKHR(render_device->device, swapchain, &num_swapchain_images, vk_images.data());

        if(render_device->settings.settings.debug.enabled) {
            for(uint32_t i = 0; i < vk_images.size(); i++) {
                const auto image_name = "Swapchain image " + std::to_string(i);

                VkDebugUtilsObjectNameInfoEXT object_name = {};
                object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                object_name.objectType = VK_OBJECT_TYPE_IMAGE;
                object_name.objectHandle = reinterpret_cast<uint64_t>(vk_images[i]);
                object_name.pObjectName = image_name.c_str();
                NOVA_CHECK_RESULT(render_device->vkSetDebugUtilsObjectNameEXT(render_device->device, &object_name));
            }
        }

        return vk_images;
    }

    VkRenderPass VulkanSwapchain::create_dummy_renderpass() const {
        VkAttachmentDescription color_attachment = {};
        color_attachment.format = swapchain_format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkAttachmentReference color_ref = {};
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = static_cast<uint32_t>(1);
        subpass.pColorAttachments = &color_ref;

        VkRenderPassCreateInfo render_pass_create_info = {};
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.attachmentCount = 1;
        render_pass_create_info.pAttachments = &color_attachment;
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass;
        render_pass_create_info.dependencyCount = 0;

        VkRenderPass renderpass;
        vkCreateRenderPass(render_device->device, &render_pass_create_info, nullptr, &renderpass);

        return renderpass;
    }

} // namespace nova::renderer::rhi
