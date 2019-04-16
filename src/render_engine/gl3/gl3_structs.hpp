/*!
 * \brief Gl2 definitions of the structs forward-declared in render_engine.hpp
 *
 * \author ddubois 
 * \date 01-Apr-19.
 */

#pragma once
#include "glad/glad.h"
#include "nova_renderer/rhi_types.hpp"
#include <string>
#include <unordered_map>

namespace nova::renderer::rhi {
    struct Gl3Image : Image {
        GLuint id = 0;
    };

    struct Gl3Buffer : Buffer {
        GLuint id = 0;
    };

    struct Gl3Renderpass : Renderpass {};

    struct Gl3Framebuffer : Framebuffer {
        GLuint id;
    };

    struct Gl3Pipeline : Pipeline {
		GLuint id;

		std::unordered_map<ResourceBindingDescription, GLuint> uniform_cache;
    };
}
