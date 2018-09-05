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
        MEMORY,
        IMAGE,
        BUFFER
    };

    union vulkan_resource {
        void* memory;
        VkImage image;
        VkBuffer buffer;
    };

    struct iresource {
        vulkan_resource resource;

    };
}

#endif //NOVA_RENDERER_VULKAN_OPAQUE_TYPES_HPP
