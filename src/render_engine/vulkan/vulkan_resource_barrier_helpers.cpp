//
// Created by ddubois on 9/4/18.
//

#include "vulkan_resource_barrier_helpers.hpp"
#include "../../util/logger.hpp"

namespace nova {
    VkPipelineStageFlags to_vk_stage_flags(stage_flags flags) {
        switch(flags) {
            case stage_flags::COLOR_ATTACHMENT_WRITE: return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            default: return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
    }

    VkAccessFlags to_vk_access_flags(stage_flags flags) {
        switch(flags) {
            case stage_flags::COLOR_ATTACHMENT_WRITE: return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            default: NOVA_LOG(ERROR) << "Unrecognized access flag " << static_cast<uint32_t>(flags); return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        }
    }

    VkImageLayout to_vk_image_layout(image_layout layout) {
        switch(layout) {
            case image_layout::RENDER_TARGET: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            case image_layout::PRESENT: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            default: return VK_IMAGE_LAYOUT_GENERAL;
        }
    }
}  // namespace nova