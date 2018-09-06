//
// Created by ddubois on 9/4/18.
//

#ifndef NOVA_RENDERER_VULKAN_RESOURCE_BARRIER_HELPERS_H
#define NOVA_RENDERER_VULKAN_RESOURCE_BARRIER_HELPERS_H

#include <vulkan/vulkan.h>
#include "../resource_barrier.hpp"

namespace nova {
    VkPipelineStageFlags to_vk_stage_flags(stage_flags flags);

    VkAccessFlags to_vk_access_flags(stage_flags flags);

    VkImageLayout to_vk_image_layout(image_layout layout);
}

#endif //NOVA_RENDERER_VULKAN_RESOURCE_BARRIER_HELPERS_H
