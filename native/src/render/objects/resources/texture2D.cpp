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
    texture2D::texture2D(const std::string name, vk::Extent2D dimensions, vk::Format format, vk::ImageUsageFlags usage,
                         std::shared_ptr<render_context> context, bool frequently_updated) :
            context(context), name(name), format(format), frequently_updated(frequently_updated) {
        size = dimensions;

        LOG(DEBUG) << "Creating image " << name << " with usage " << vk::to_string(usage);

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
            LOG(ERROR) << "Could not create image" << name;
            throw std::runtime_error("Could not create image " + name);
        }

        vk::ImageSubresourceRange subresource_range = {};
        subresource_range.layerCount = 1;
        subresource_range.baseArrayLayer = 0;
        subresource_range.levelCount = 1;
        subresource_range.baseMipLevel = 0;
        subresource_range.aspectMask = vk::ImageAspectFlags();
        if((usage & vk::ImageUsageFlagBits::eColorAttachment) != vk::ImageUsageFlagBits()) {
            subresource_range.aspectMask |= vk::ImageAspectFlagBits::eColor;
        }

        if((usage & vk::ImageUsageFlagBits::eDepthStencilAttachment) != vk::ImageUsageFlagBits()) {
            subresource_range.aspectMask |= vk::ImageAspectFlagBits::eDepth;

        } else if((usage & vk::ImageUsageFlagBits::eSampled) != vk::ImageUsageFlagBits()) {
            subresource_range.aspectMask |= vk::ImageAspectFlagBits::eColor;
        }


        vk::ImageViewCreateInfo img_view_create_info = {};
        img_view_create_info.image = image;
        img_view_create_info.viewType = vk::ImageViewType::e2D;
        img_view_create_info.format = format;
        img_view_create_info.subresourceRange = subresource_range;

        image_view = context->device.createImageView(img_view_create_info);
        layout = image_create_info.initialLayout;

        if(frequently_updated) {
            buffer_size = dimensions.width * dimensions.height * 4;

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

            vmaMapMemory(context->allocator, staging_buffer_allocation, &mapped_data);

            data_upload_cmd_buffer = context->command_buffer_pool->get_command_buffer(0);
        }

        LOG(DEBUG) << "Image creation successful";
    }

    texture2D::texture2D(texture2D&& other) noexcept {
        move_into_self(other);
    }

    texture2D& texture2D::operator==(nova::texture2D &&other) {
        move_into_self(other);

        return *this;
    }

    void texture2D::move_into_self(texture2D &other) {
        destroy();

        context = other.context;

        size = other.size;
        name = std::move(other.name);

        image = other.image;
        format = other.format;
        layout = other.layout;
        allocation = other.allocation;

        frequently_updated = other.frequently_updated;

        buffer_size = other.buffer_size;
        staging_buffer = other.staging_buffer;
        staging_buffer_allocation = other.staging_buffer_allocation;
        mapped_data = other.mapped_data;
        data_upload_cmd_buffer = std::move(other.data_upload_cmd_buffer);

        other.size = {0, 0};
    }

    texture2D::~texture2D() {
        destroy();
    }

    void texture2D::destroy() {
        if(size == vk::Extent2D(0, 0)) {
            // Texture was either never initialized or was already destroyed
            return;
        }

        if(frequently_updated) {
            vmaUnmapMemory(context->allocator, staging_buffer_allocation);
            vmaDestroyBuffer(context->allocator, (VkBuffer) staging_buffer, staging_buffer_allocation);

            context->command_buffer_pool->free(data_upload_cmd_buffer);
        }

        context->device.destroyImageView(image_view);
        vmaDestroyImage(context->allocator, (VkImage)image, allocation);
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
        LOG(TRACE) << "Setting data for texture " << name;
        if(!frequently_updated) {
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

            vmaCreateBuffer(context->allocator, reinterpret_cast<VkBufferCreateInfo *>(&buffer_create_info),
                            &staging_buffer_allocation_info,
                            reinterpret_cast<VkBuffer *>(&staging_buffer), &staging_buffer_allocation, nullptr);
            LOG(TRACE) << "Allocated staging buffer " << (VkBuffer) staging_buffer;

            vmaMapMemory(context->allocator, staging_buffer_allocation, &mapped_data);
        }

        std::memcpy(mapped_data, data, buffer_size);
        LOG(TRACE) << "Copied data to staging buffer";

        if(!frequently_updated) {
            vmaUnmapMemory(context->allocator, staging_buffer_allocation);

            data_upload_cmd_buffer = context->command_buffer_pool->get_command_buffer(0);
        }

        data_upload_cmd_buffer.buffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        data_upload_cmd_buffer.begin_as_single_commend();

        transfer_image_format(data_upload_cmd_buffer.buffer, image, layout, vk::ImageLayout::eTransferDstOptimal);
        copy_buffer_to_image(data_upload_cmd_buffer.buffer, staging_buffer, image, size.width, size.height);
        transfer_image_format(data_upload_cmd_buffer.buffer, image, vk::ImageLayout::eTransferDstOptimal,
                              vk::ImageLayout::eShaderReadOnlyOptimal);
        layout = vk::ImageLayout::eShaderReadOnlyOptimal;

        data_upload_cmd_buffer.end_as_single_command();

        if(!frequently_updated) {
            vmaDestroyBuffer(context->allocator, (VkBuffer) staging_buffer, staging_buffer_allocation);
            LOG(TRACE) << "Destroyed staging buffer";

            context->command_buffer_pool->free(data_upload_cmd_buffer);
        }
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
        switch(old_layout) {
            case vk::ImageLayout::eGeneral:
            case vk::ImageLayout::eUndefined:
                barrier.srcAccessMask = vk::AccessFlags();
                source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
                break;

            case vk::ImageLayout::eTransferDstOptimal:
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                source_stage = vk::PipelineStageFlagBits::eTransfer;
                break;

            case vk::ImageLayout::eColorAttachmentOptimal:
                barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
                source_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
                break;

            case vk::ImageLayout::eShaderReadOnlyOptimal:
                barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
                source_stage = vk::PipelineStageFlagBits::eFragmentShader;
                break;

            default:
                LOG(ERROR) << "Unsupported layout transition! Can't transition from " << vk::to_string(old_layout);
        }

        switch(new_layout) {
            case vk::ImageLayout::eTransferDstOptimal:
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
                destination_stage = vk::PipelineStageFlagBits::eTransfer;
                break;

            case vk::ImageLayout::eShaderReadOnlyOptimal:
                barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
                destination_stage = vk::PipelineStageFlagBits::eFragmentShader;
                break;

            case vk::ImageLayout::eGeneral:
                barrier.dstAccessMask = vk::AccessFlags();
                destination_stage = vk::PipelineStageFlagBits::eTopOfPipe;
                break;

            case vk::ImageLayout::eColorAttachmentOptimal:
                barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
                destination_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
                break;

            default:
                LOG(ERROR) << "Unsupported layout transition! Can't transition to " << vk::to_string(new_layout);
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

        LOG(TRACE) << "Recorded command into command buffer " << (VkCommandBuffer)command_buffer << " to copy data to image " << (VkImage)image;
    }
}
