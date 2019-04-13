/*!
 * \brief Gl2 definitions of the structs forward-declared in render_engine.hpp
 *
 * \author ddubois 
 * \date 01-Apr-19.
 */

#pragma once

namespace nova::renderer::rhi {
    struct GL2Image : Image {
        GLuint id;
    };

    struct GL2Buffer : Buffer {
        GLuint id;
    };

    struct GL2Renderpass : Renderpass {};

    struct GL2Framebuffer : Framebuffer {
        GLuint framebuffer;
    };
}
