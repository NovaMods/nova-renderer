/*!
 * \brief
 *
 * \author ddubois 
 * \date 30-Sep-16.
 */

#ifndef RENDERER_RENDER_PASS_H
#define RENDERER_RENDER_PASS_H

#include <string>
#include <vector>
#include <view/objects/shaders/uniform_buffer_definitions.h>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "view/objects/gl_mesh.h"

namespace nova {
    namespace view {
        /*!
         * \brief Defines a single render pass
         *
         * There is a one-to-one corellation between render passes and shaders.
         *
         * I've separated the render passes from the shadow stage, gbuffer stage, and fullscreen stage because doing
         * this allows me to set the uniforms on a per-stage bases, rather than specifying them on a per-shader basis.
         * This will be faster when there's more than one render pass per stage, which is almost always
         */
        struct render_pass {
            std::string shader_program_name;

            std::vector<gl_mesh> meshes_to_draw;
        };
        struct shadow_stage {
            std::vector<render_pass> meshes_to_draw;

            shadow_pass_textures textures;
        };

        struct gbuffer_stage {
            std::vector<render_pass> meshes_to_draw;

            gbuffer_pass_textures textures;
        };

        struct fullscreen_stage {
            std::vector<render_pass> meshes_to_draw;

            fullscreen_pass_textures textures;
        };

        struct render_command {
            std::vector<shadow_stage> shadow_passes;
            std::vector<gbuffer_stage> gbuffer_passes;
            std::vector<fullscreen_stage> fullscreen_passes;

            per_frame_uniforms uniforms;
        };
    }
}

#endif //RENDERER_RENDER_PASS_H
