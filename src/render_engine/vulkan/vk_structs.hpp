/*!
 * \brief Vulkan definition of the structs forward-declared in render_engine.hpp
 *
 * \author ddubois 
 * \date 01-Apr-19.
 */

#pragma once

#include <nova_renderer/render_engine.hpp>

namespace nova::renderer {
    struct vk_resource_t : resource_t {
        enum class type {
            IMAGE,
            BUFFER,
        };

        type resource_type;

        union {
            VkBuffer buffer;
            VkImage image;
        };
    };

    struct vk_renderpass_t : renderpass_t {
        VkRenderPass renderpass;
    };

    struct vk_framebuffer_t : framebuffer_t {
        VkFramebuffer framebuffer;
    };
}
