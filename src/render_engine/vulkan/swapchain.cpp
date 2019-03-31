/*!
 * \author ddubois
 * \date 28-Apr-18.
 */

#include "swapchain.hpp"
#include "../../util/logger.hpp"
#include "vulkan_render_engine.hpp"
#include "vulkan_utils.hpp"

namespace nova::renderer {
    swapchain_manager::swapchain_manager(const uint32_t requested_num_swapchain_images,
                                         vulkan_render_engine& render_engine,
                                         const glm::ivec2 window_dimensions,
                                         const std::vector<VkPresentModeKHR>& present_modes)
        : render_engine(render_engine),
          swapchain_extent{static_cast<uint32_t>(window_dimensions.x), static_cast<uint32_t>(window_dimensions.y)} {
        const auto surface_format = choose_surface_format(render_engine.gpu.surface_formats);
        const auto present_mode = choose_present_mode(present_modes);
        const auto extent = choose_surface_extent(render_engine.gpu.surface_capabilities, window_dimensions);

        VkSwapchainCreateInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = render_engine.surface;

        info.minImageCount = requested_num_swapchain_images;

        info.imageFormat = surface_format.format;
        info.imageColorSpace = surface_format.colorSpace;
        info.imageExtent = extent;
        info.imageArrayLayers = 1;

        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.queueFamilyIndexCount = 1;
        info.pQueueFamilyIndices = &render_engine.graphics_family_index;

        info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = present_mode;

        info.clipped = VK_TRUE;

        vkCreateSwapchainKHR(render_engine.device, &info, nullptr, &swapchain);

        swapchain_format = surface_format.format;
        this->present_mode = present_mode;
        swapchain_extent = extent;

        NOVA_CHECK_RESULT(vkGetSwapchainImagesKHR(render_engine.device, swapchain, &num_swapchain_images, nullptr));
        swapchain_images.resize(num_swapchain_images);
        NOVA_CHECK_RESULT(vkGetSwapchainImagesKHR(render_engine.device, swapchain, &num_swapchain_images, swapchain_images.data()));
        swapchain_image_layouts.resize(num_swapchain_images);
        for(auto& swapchain_image_layout : swapchain_image_layouts) {
            swapchain_image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        if(swapchain_images.empty()) {
            NOVA_LOG(FATAL) << "The swapchain returned zero images";
        }

        // Create a dummy renderpass that writes to a single color attachment - the swapchain
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
        vkCreateRenderPass(render_engine.device, &render_pass_create_info, nullptr, &renderpass);

        // More than 255 images in the swapchain? Good lord what are you doing? and will you please stop?
        for(uint8_t i = 0; i < num_swapchain_images; i++) {
            VkImageViewCreateInfo image_view_create_info = {};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

            image_view_create_info.image = swapchain_images[i];
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

            VkImageView image_view;
            vkCreateImageView(render_engine.device, &image_view_create_info, nullptr, &image_view);

            swapchain_image_views.push_back(image_view);

            VkFramebufferCreateInfo framebuffer_create_info = {};
            framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.attachmentCount = 1;
            framebuffer_create_info.pAttachments = &image_view;
            framebuffer_create_info.renderPass = renderpass;
            framebuffer_create_info.width = swapchain_extent.width;
            framebuffer_create_info.height = swapchain_extent.height;
            framebuffer_create_info.layers = 1;

            VkFramebuffer framebuffer;
            vkCreateFramebuffer(render_engine.device, &framebuffer_create_info, nullptr, &framebuffer);
            framebuffers.push_back(framebuffer);

            VkFenceCreateInfo fence_create_info = {};
            fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            VkFence fence;
            vkCreateFence(render_engine.device, &fence_create_info, nullptr, &fence);
            fences.push_back(fence);
        }

        vkDestroyRenderPass(render_engine.device, renderpass, nullptr);

        // move the swapchain images into the correct layout cause I guess they aren't for some reason?
        transition_swapchain_images_into_correct_layout(swapchain_images);
    }

    VkSurfaceFormatKHR swapchain_manager::choose_surface_format(const std::vector<VkSurfaceFormatKHR>& formats) {
        VkSurfaceFormatKHR result;

        if(formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
            result.format = VK_FORMAT_B8G8R8A8_UNORM;
            result.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            return result;
        }

        // We want 32 bit rgba and srgb nonlinear... I think? Will have to read up on it more and figure out what's up
        for(auto& fmt : formats) {
            if(fmt.format == VK_FORMAT_B8G8R8A8_UNORM && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return fmt;
            }
        }

        // We can't have what we want, so I guess we'll just use what we got
        return formats[0];
    }

    VkPresentModeKHR swapchain_manager::choose_present_mode(const std::vector<VkPresentModeKHR>& modes) {
        const VkPresentModeKHR desired_mode = VK_PRESENT_MODE_MAILBOX_KHR;

        // Mailbox mode is best mode (also not sure why)
        for(auto& mode : modes) {
            if(mode == desired_mode) {
                return desired_mode;
            }
        }

        // FIFO, like FIFA, is forever
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D swapchain_manager::choose_surface_extent(const VkSurfaceCapabilitiesKHR& caps, const glm::ivec2& window_dimensions) {
        VkExtent2D extent;

        if(caps.currentExtent.width == 0xFFFFFFFF) {
            extent.width = static_cast<uint32_t>(window_dimensions.x);
            extent.height = static_cast<uint32_t>(window_dimensions.y);
        } else {
            extent = caps.currentExtent;
        }

        return extent;
    }

    void swapchain_manager::present_current_image(VkSemaphore wait_semaphores) const {

        VkResult swapchain_result = {};

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &wait_semaphores;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
        present_info.pImageIndices = &cur_swapchain_index;
        present_info.pResults = &swapchain_result;

        NOVA_CHECK_RESULT(vkQueuePresentKHR(render_engine.graphics_queue, &present_info));
    }

    void swapchain_manager::transition_swapchain_images_into_correct_layout(const std::vector<VkImage>& images) const {
        std::vector<VkImageMemoryBarrier> barriers;
        barriers.reserve(images.size());

        for(const VkImage& image : images) {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcQueueFamilyIndex = render_engine.graphics_family_index;
            barrier.dstQueueFamilyIndex = render_engine.graphics_family_index;
            barrier.image = image;
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Each swapchain image **will** be rendered to before it is
                                                                          // presented
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            barriers.push_back(barrier);
        }

        VkCommandPool command_pool = render_engine.get_command_buffer_pool_for_current_thread(render_engine.graphics_family_index);

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = command_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer cmds;
        vkAllocateCommandBuffers(render_engine.device, &alloc_info, &cmds);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmds, &begin_info);

        vkCmdPipelineBarrier(cmds,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
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

        vkCreateFence(render_engine.device, &fence_create_info, nullptr, &transition_done_fence);

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmds;

        vkQueueSubmit(render_engine.graphics_queue, 1, &submit_info, transition_done_fence);
        vkWaitForFences(render_engine.device, 1, &transition_done_fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

        vkFreeCommandBuffers(render_engine.device, command_pool, 1, &cmds);
    }

    void swapchain_manager::deinit() {
        for(auto& i : swapchain_images) {
            vkDestroyImage(render_engine.device, i, nullptr);
        }

        for(auto& iv : swapchain_image_views) {
            vkDestroyImageView(render_engine.device, iv, nullptr);
        }

        for(auto& f : fences) {
            vkDestroyFence(render_engine.device, f, nullptr);
        }
    }

    uint32_t swapchain_manager::get_current_index() const { return cur_swapchain_index; }

    uint32_t swapchain_manager::get_num_images() const { return num_swapchain_images; }

    void swapchain_manager::acquire_next_swapchain_image(VkSemaphore image_acquire_semaphore) {
        const auto acquire_result = vkAcquireNextImageKHR(render_engine.device,
                                                          swapchain,
                                                          std::numeric_limits<uint64_t>::max(),
                                                          image_acquire_semaphore,
                                                          VK_NULL_HANDLE,
                                                          &cur_swapchain_index);
        if(acquire_result == VK_ERROR_OUT_OF_DATE_KHR || acquire_result == VK_SUBOPTIMAL_KHR) {
            // TODO: Recreate the swapchain and all screen-relative textures
            NOVA_LOG(ERROR) << "Swapchain out of date! One day you'll write the code to recreate it";
            return;
        }
        if(acquire_result != VK_SUCCESS) {
            NOVA_LOG(ERROR) << __FILE__ << ":" << __LINE__ << "=> " << vk_result_to_string(acquire_result);
        }
    }

    void swapchain_manager::set_current_layout(VkImageLayout new_layout) { swapchain_image_layouts[cur_swapchain_index] = new_layout; }

    VkFramebuffer swapchain_manager::get_current_framebuffer() {
        NOVA_LOG(TRACE) << "Getting swapchain framebuffer " << cur_swapchain_index << " out of " << framebuffers.size();
        return framebuffers[cur_swapchain_index];
    }

    VkImage swapchain_manager::get_current_image() { return swapchain_images[cur_swapchain_index]; }

    VkImageLayout swapchain_manager::get_current_layout() { return swapchain_image_layouts[cur_swapchain_index]; }

    VkExtent2D swapchain_manager::get_swapchain_extent() const { return swapchain_extent; }

    VkFormat swapchain_manager::get_swapchain_format() const { return swapchain_format; }

    VkFence swapchain_manager::get_current_frame_fence() { return fences[cur_swapchain_index]; }
} // namespace nova::renderer
