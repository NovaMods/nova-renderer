//
// Created by ddubois on 9/4/18.
//

#include "vulkan_resource_barrier_helpers.hpp"
#include "../../util/logger.hpp"

namespace nova {
    VkAccessFlags to_vk_access_flags(resource_layout layout) {
        switch(layout) {
            case resource_layout::RENDER_TARGET:
                return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            case resource_layout::PRESENT:
                return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            default:
                NOVA_LOG(ERROR) << "Unrecognized resource state " << layout.to_string();
                return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        }
    }
}