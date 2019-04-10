/*!
 * \brief Gl2 definitions of the structs forward-declared in render_engine.hpp
 *
 * \author ddubois 
 * \date 01-Apr-19.
 */

#pragma once

namespace nova::renderer::rhi {
    struct gl2_image_t : image_t {
        GLuint id;
    };

    struct gl2_buffer_t : buffer_t {
        GLuint id;
    };

    struct gl2_renderpass_t : renderpass_t {};

    struct gl2_framebuffer_t : framebuffer_t {
        GLuint framebuffer;
    };
}
