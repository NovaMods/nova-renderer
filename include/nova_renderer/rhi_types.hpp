/*!
 * \author ddubois
 * \date 03-Apr-19.
 */

#pragma once

#include <glm/glm.hpp>

namespace nova::renderer::rhi {
    enum class QueueType {
        Graphics,
        Transfer,
        AsyncCompute,
    };

    struct Resource {
        enum Type {
            Buffer,
            Image,
        };

        Type type;
        bool is_dynamic = false;
    };

    struct Image : Resource {
        bool is_depth_tex = false;
    };

    struct Buffer : Resource {};

    struct Framebuffer {
        glm::uvec2 size;
    };

    struct Renderpass {
        bool writes_to_backbuffer = false;
    };

    struct Pipeline {};

    struct Semaphore {};

    struct Fence {};

    struct Descriptor {};
} // namespace nova::renderer::rhi
