/*!
 * \brief definitions for the opaque types
 *
 * Created by ddubois on 9/4/18.
 */

#ifndef NOVA_RENDERER_VULKAN_OPAQUE_TYPES_HPP
#define NOVA_RENDERER_VULKAN_OPAQUE_TYPES_HPP

#include <vulkan/vulkan.h>

namespace nova {
    enum class resource_type {
        IMAGE,
        BUFFER
    };

    union vulkan_resource {
        VkImage image;
        VkBuffer buffer;
    };

    struct iresource {
        vulkan_resource resource;
        resource_type type;
    };

    struct iframebuffer {
        VkFramebuffer framebuffer;
    };

    struct ifence {
        VkFence fence;
    };
}

#endif //NOVA_RENDERER_VULKAN_OPAQUE_TYPES_HPP
