/*!
 * \brief Gl2 definitions of the structs forward-declared in render_engine.hpp
 *
 * \author ddubois 
 * \date 01-Apr-19.
 */

#pragma once

namespace nova::renderer {
    struct gl_resource_t : resource_t {
        GLuint id;
    };

    struct gl_framebuffer_t : framebuffer_t {
        GLuint framebuffer;
    };
}
