/*!
 * \author ddubois 
 * \date 28-Apr-18.
 */

#include <easylogging++.h>
#include "swapchain_manager.h"

namespace nova {
    swapchain_manager::swapchain_manager(uint32_t num_swapchain_images, std::shared_ptr<nova::render_context> context,
                                         glm::ivec2 window_dimensions) : context(context), swapchain_extent(static_cast<uint32_t>(window_dimensions.x), static_cast<uint32_t>(window_dimensions.y)) {
        auto surface_format = choose_surface_format(context->gpu.surface_formats);
        auto present_mode = choose_present_mode(context->gpu.present_modes);
        auto extent = choose_surface_extent(context->gpu.surface_capabilities, window_dimensions);

        vk::SwapchainCreateInfoKHR info = {};
        info.surface = context->surface;

        info.minImageCount = num_swapchain_images;

        info.imageFormat = surface_format.format;
        info.imageColorSpace = surface_format.colorSpace;
        info.imageExtent = extent;
        info.imageArrayLayers = 1;

        info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;

        if(context->graphics_family_idx != context->present_family_idx) {
            // If the indices are different then we need to share the images
            uint32_t indices[] = {context->graphics_family_idx, context->present_family_idx};

            info.imageSharingMode = vk::SharingMode::eConcurrent;
            info.queueFamilyIndexCount = 2;
            info.pQueueFamilyIndices = indices;
        } else {
            // If the indices are the same, we can have exclusive access
            info.imageSharingMode = vk::SharingMode::eExclusive;
            info.queueFamilyIndexCount = 1;
            info.pQueueFamilyIndices = &context->present_family_idx;
        }

        info.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
        info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        info.presentMode = present_mode;

        info.clipped = VK_TRUE;

        swapchain = context->device.createSwapchainKHR(info);

        swapchain_format = surface_format.format;
        this->present_mode = present_mode;
        swapchain_extent = extent;

        swapchain_images = context->device.getSwapchainImagesKHR(swapchain);
        for(uint32_t i = 0; i < swapchain_images.size(); i++) {
            swapchain_image_layouts.push_back(vk::ImageLayout::eUndefined);
        }

        if(swapchain_images.empty()) {
            LOG(FATAL) << "The swapchain returned zero images";
        }

        // More than 255 images in the swapchain? Good lord what are you doing? and will you please stop?
        for(uint8_t i = 0; i < num_swapchain_images; i++) {
            vk::ImageViewCreateInfo image_view_create_info = {};

            image_view_create_info.image = swapchain_images[i];
            image_view_create_info.viewType = vk::ImageViewType::e2D;
            image_view_create_info.format = swapchain_format;

            image_view_create_info.components.r = vk::ComponentSwizzle::eR;
            image_view_create_info.components.g = vk::ComponentSwizzle::eG;
            image_view_create_info.components.b = vk::ComponentSwizzle::eB;
            image_view_create_info.components.a = vk::ComponentSwizzle::eA;

            image_view_create_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;

            vk::ImageView image_view = context->device.createImageView(image_view_create_info);

            swapchain_image_views.push_back(image_view);
        }

        // This block just kinda checks that the depth buffer we want is available
        {
            vk::Format formats[] = {
                    vk::Format::eD32SfloatS8Uint,
                    vk::Format::eD24UnormS8Uint
            };
            depth_format = choose_supported_format(formats, 2, vk::ImageTiling::eOptimal,
                                                   vk::FormatFeatureFlagBits::eDepthStencilAttachment);
        }

        // move the swapchain images into the correct layout cause I guess they aren't for some reason?
        move_swapchain_images_into_correct_format(swapchain_images);
    }

    vk::SurfaceFormatKHR swapchain_manager::choose_surface_format(std::vector<vk::SurfaceFormatKHR> &formats) {
        vk::SurfaceFormatKHR result = {};

        if(formats.size() == 1 && formats[0].format == vk::Format::eUndefined) {
            result.format = vk::Format::eB8G8R8A8Unorm;
            result.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
            return result;
        }

        // We want 32 bit rgba and srgb nonlinear... I think? Will have to read up on it more and figure out what's up
        for(auto& fmt : formats) {
            if(fmt.format == vk::Format::eB8G8R8A8Unorm && fmt.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return fmt;
            }
        }

        // We can't have what we want, so I guess we'll just use what we got
        return formats[0];
    }

    vk::PresentModeKHR swapchain_manager::choose_present_mode(std::vector<vk::PresentModeKHR> &modes) {
        const vk::PresentModeKHR desired_mode = vk::PresentModeKHR::eMailbox;

        // Mailbox mode is best mode (also not sure why)
        for(auto& mode : modes) {
            if(mode == desired_mode) {
                return desired_mode;
            }
        }

        // FIFO, like FIFA, is forever
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D swapchain_manager::choose_surface_extent(vk::SurfaceCapabilitiesKHR &caps, glm::ivec2& window_dimensions) {
        vk::Extent2D extent;

        if(caps.currentExtent.width == 0xFFFFFFFF) {
            extent.width = static_cast<uint32_t>(window_dimensions.x);
            extent.height = static_cast<uint32_t>(window_dimensions.y);

        } else {
            extent = caps.currentExtent;
        }

        return extent;
    }

    void swapchain_manager::present_current_image(const std::vector<vk::Semaphore> &wait_semaphores) {

        vk::Result swapchain_result = {};

        vk::PresentInfoKHR present_info = {};
        present_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());
        present_info.pWaitSemaphores = wait_semaphores.data();
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
        present_info.pImageIndices = &cur_swapchain_index;
        present_info.pResults = &swapchain_result;

        context->present_queue.presentKHR(present_info);
    }

    vk::Format swapchain_manager::choose_supported_format(vk::Format *formats, int num_formats, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
        for(int i = 0; i < num_formats; i++) {
            vk::Format& format = formats[i];

            vk::FormatProperties props = context->physical_device.getFormatProperties(format);

            if(tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if(tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        // A fatal log causes a crash so not having a return here is fine
        LOG(FATAL) << "Failed to find a suitable depth buffer format";
        return {};
    }

    void swapchain_manager::move_swapchain_images_into_correct_format(const std::vector<vk::Image> &images) {
        vk::ImageMemoryBarrier move_to_swapchain_format = {};
        move_to_swapchain_format.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        move_to_swapchain_format.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
        move_to_swapchain_format.oldLayout = vk::ImageLayout::eUndefined;
        move_to_swapchain_format.newLayout = vk::ImageLayout::ePresentSrcKHR;


    }

    void swapchain_manager::deinit() {
        for(auto& iv : swapchain_image_views) {
            context->device.destroyImageView(iv);
        }
    }

    void swapchain_manager::aqcuire_next_swapchain_image(vk::Semaphore image_acquire_semaphore) {
        vk::ResultValue<uint32_t> result = context->device.acquireNextImageKHR(swapchain,
                                                                               std::numeric_limits<uint64_t>::max(),
                                                                               image_acquire_semaphore,
                                                                               vk::Fence());
        if (result.result != vk::Result::eSuccess) {
            LOG(ERROR) << "Could not acquire swapchain image! vkResult: " << result.result;
        }
        cur_swapchain_index = result.value;

        LOG(INFO) << "Acquired swapchain image " << cur_swapchain_index;
    }

    void swapchain_manager::set_current_layout(vk::ImageLayout new_layout) {
        swapchain_image_layouts[cur_swapchain_index] = new_layout;
    }

    vk::Framebuffer swapchain_manager::get_current_framebuffer() {
        LOG(INFO) << "Getting swapchain framebuffer " << cur_swapchain_index << " out of " << framebuffers.size();
        return framebuffers[cur_swapchain_index];
    }

    vk::Image swapchain_manager::get_current_image() {
        return swapchain_images[cur_swapchain_index];
    }

    vk::ImageLayout swapchain_manager::get_current_layout() {
        return swapchain_image_layouts[cur_swapchain_index];
    }

    vk::Extent2D swapchain_manager::get_swapchain_extent() {
        return swapchain_extent;
    }

    vk::Format swapchain_manager::get_swapchain_format() {
        return swapchain_format;
    }
}
