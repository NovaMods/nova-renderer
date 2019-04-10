/*!
 * \author ddubois
 * \date 03-Apr-19.
 */

#pragma once

#include <glm/glm.hpp>

namespace nova::renderer::rhi {
    enum class queue_type {
        GRAPHICS,
        TRANSFER,
        ASYNC_COMPUTE,
    };

    struct resource_t {
        enum type_t {
            BUFFER,
            IMAGE,
        };

        type_t type;
        bool is_dynamic = false;
    };

    struct image_t : resource_t {
        bool is_depth_tex = false;
    };

    struct buffer_t : resource_t {};

    struct framebuffer_t {
        glm::uvec2 size;
    };

    struct renderpass_t {
        bool writes_to_backbuffer = false;
    };

    struct pipeline_t {};

    struct semaphore_t {};

    struct fence_t {};

    struct descriptor_t {};
} // namespace nova::renderer::rhi
