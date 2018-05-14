//
// Created by David on 26-Apr-16.
//

#include "texture2D.h"
#include <stdexcept>
#include <easylogging++.h>
#include <vk_mem_alloc.h>
#include "../../../utils/utils.h"
#include "../../vulkan/render_context.h"
#include "../../vulkan/command_pool.h"
#include "../../nova_renderer.h"

namespace nova {
    texture2D::texture2D(vk::Extent2D dimensions, vk::Format format, vk::ImageUsageFlags usage, std::shared_ptr<render_context> context) : context(context) {
        this->format = format;
        size = dimensions;

        vk::ImageCreateInfo image_create_info = {};
        image_create_info.samples = vk::SampleCountFlagBits::e1;
        image_create_info.imageType = vk::ImageType::e2D;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.extent = vk::Extent3D{dimensions.width, dimensions.height, 1};
        image_create_info.tiling = vk::ImageTiling::eOptimal;
        image_create_info.format = format;
        image_create_info.usage = usage;
        image_create_info.queueFamilyIndexCount = 1;
        image_create_info.pQueueFamilyIndices = &context->graphics_family_idx;
        image_create_info.initialLayout = vk::ImageLayout::eUndefined;

        VmaAllocationCreateInfo alloc_create_info = {};
        alloc_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        auto result = vmaCreateImage(context->allocator, reinterpret_cast<VkImageCreateInfo*>(&image_create_info), &alloc_create_info,
                                     reinterpret_cast<VkImage*>(&image), &allocation, nullptr);

        if(result != (VkResult)vk::Result::eSuccess) {
            LOG(FATAL) << "Could not create image";
        }

        LOG(INFO) << "Created image " << (VkImage)image;

        vk::ImageSubresourceRange subresource_range = {};
        subresource_range.layerCount = 1;
        subresource_range.baseArrayLayer = 0;
        subresource_range.levelCount = 1;
        subresource_range.baseMipLevel = 0;
        subresource_range.aspectMask = vk::ImageAspectFlagBits::eColor;

        vk::ImageViewCreateInfo img_view_create_info = {};
        img_view_create_info.image = image;
        img_view_create_info.viewType = vk::ImageViewType::e2D;
        img_view_create_info.format = format;
        img_view_create_info.subresourceRange = subresource_range;

        image_view = context->device.createImageView(img_view_create_info);
        LOG(TRACE) << "Created image view";
        layout = image_create_info.initialLayout;

        LOG(DEBUG) << "Created new image";
    }

    void texture2D::set_data(void* pixel_data, vk::Extent2D dimensions) {
        // Create a staging buffer and use that to upload the data
        upload_data_with_staging_buffer(pixel_data, vk::Extent3D{dimensions.width, dimensions.height, 1});
    }

    vk::Extent2D texture2D::get_size() const {
        return size;
    }

    vk::Format  texture2D::get_format() const {
        return format;
    }

    const vk::Image texture2D::get_vk_image() const {
        return image;
    }

    void texture2D::set_name(const std::string name) {
        this->name = name;
    }

    const std::string &texture2D::get_name() const {
        return name;
    }

    void texture2D::upload_data_with_staging_buffer(void *data, vk::Extent3D image_size) {
        LOG(INFO) << "Setting data for texture " << name;
        vk::Buffer staging_buffer;
        VmaAllocation staging_buffer_allocation;
        auto buffer_size = image_size.width * image_size.height * image_size.depth * 4;

        vk::BufferCreateInfo buffer_create_info = {};
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &context->graphics_family_idx;
        buffer_create_info.usage = vk::BufferUsageFlagBits::eTransferSrc;

        // A standard image from disk is 32 bpp in Nova. I'm going to hate myself when Joey wants something else
        buffer_create_info.size = buffer_size;

        VmaAllocationCreateInfo staging_buffer_allocation_info = {};
        staging_buffer_allocation_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        staging_buffer_allocation_info.flags = 0;

        vmaCreateBuffer(context->allocator, reinterpret_cast<VkBufferCreateInfo*>(&buffer_create_info), &staging_buffer_allocation_info,
                        reinterpret_cast<VkBuffer*>(&staging_buffer), &staging_buffer_allocation, nullptr);
        LOG(INFO) << "Allocated staging buffer " << (VkBuffer)staging_buffer;

        void* mapped_data;
        vmaMapMemory(context->allocator, staging_buffer_allocation, &mapped_data);
        std::memcpy(mapped_data, data, buffer_size);
        vmaUnmapMemory(context->allocator, staging_buffer_allocation);
        LOG(INFO) << "Copied data to staging buffer";

        auto command_buffer = context->command_buffer_pool->get_command_buffer(0);
        command_buffer.buffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        command_buffer.begin_as_single_commend();

        transfer_image_format(command_buffer.buffer, image, layout, vk::ImageLayout::eTransferDstOptimal);
        copy_buffer_to_image(command_buffer.buffer, staging_buffer, image, size.width, size.height);
        transfer_image_format(command_buffer.buffer, image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        layout = vk::ImageLayout::eShaderReadOnlyOptimal;

        command_buffer.end_as_single_command();

        vmaDestroyBuffer(context->allocator, (VkBuffer)staging_buffer, staging_buffer_allocation);
        LOG(INFO) << "Destroyed staging buffer";
    }

    void texture2D::destroy() {
        LOG(INFO) << "Destroying image view " << (VkImageView)image_view;

        vmaDestroyImage(context->allocator, (VkImage)image, allocation);
        context->device.destroyImageView(image_view);
    }

    vk::ImageView texture2D::get_image_view() const {
        return image_view;
    }

    vk::ImageLayout texture2D::get_layout() const {
        return layout;
    }

    void texture2D::set_layout(vk::ImageLayout layout) {
        this->layout = layout;
    }

    void transfer_image_format(vk::CommandBuffer command_buffer, vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout) {
        vk::ImageMemoryBarrier barrier = {};
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        // This block seems weirdly hardcoded and not scalable but idk
        vk::PipelineStageFlags source_stage;
        vk::PipelineStageFlags destination_stage;
        if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal) {
            barrier.srcAccessMask = vk::AccessFlags();
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
            destination_stage = vk::PipelineStageFlagBits::eTransfer;

        } else if (old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            source_stage = vk::PipelineStageFlagBits::eTransfer;
            destination_stage = vk::PipelineStageFlagBits::eFragmentShader;

        } else if (old_layout == vk::ImageLayout::eGeneral && new_layout == vk::ImageLayout::eTransferDstOptimal) {
            barrier.srcAccessMask = vk::AccessFlags();
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
            destination_stage = vk::PipelineStageFlagBits::eTransfer;

        } else {
            auto ss = std::stringstream{};
            ss << "Unsupported layout transition! Can't transition from " << vk::to_string(old_layout) << " to " << vk::to_string(new_layout);
            throw std::invalid_argument(ss.str());
        }

        command_buffer.pipelineBarrier(
                source_stage, destination_stage,
                vk::DependencyFlags(),
                0, nullptr,
                0, nullptr,
                1, &barrier);
    }

    void copy_buffer_to_image(vk::CommandBuffer command_buffer, vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) {

        vk::BufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = vk::Offset3D{0, 0, 0};
        region.imageExtent = vk::Extent3D{width, height, 1};

        command_buffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

        LOG(INFO) << "Recorded command into command buffer " << (VkCommandBuffer)command_buffer << " to copy data to image " << (VkImage)image;
    }
}
