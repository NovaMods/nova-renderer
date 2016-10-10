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
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "model/shaders/uniform_buffer_definitions.h"

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

            std::vector<mesh> meshes_to_draw;
        };

        struct shadow_stage {
            std::vector<render_pass> meshes_to_draw;

            model::shadow_pass_uniforms uniforms;
        };

        struct gbuffer_stage {
            std::vector<render_pass> meshes_to_draw;

            model::gbuffer_pass_uniforms uniforms;
        };

        struct fullscreen_stage {
            std::vector<render_pass> meshes_to_draw;

            model::fullscreen_pass_uniforms uniforms;
        };

        struct render_command {
            std::vector<shadow_stage> shadow_passes;
            std::vector<gbuffer_stage> gbuffer_passes;
            std::vector<fullscreen_stage> fullscreen_passes;

            model::per_frame_uniforms uniforms;
        };
    }
}

#endif //RENDERER_RENDER_PASS_H
