/*!
 * \brief
 *
 * \author ddubois 
 * \date 20-Sep-16.
 */

#ifndef RENDERER_RENDER_COMMAND_H
#define RENDERER_RENDER_COMMAND_H

#include <vector>
#include "glad/glad.h"
#include "glm/glm.hpp"

namespace nova {
    namespace adapter {
        /*!
         * \brief Holds all the uniform variables that are shared by all shader executions
         *
         * The data in the structure should only be uploaded once per frame
         */
        struct per_pass_uniforms {
            int heldItemId;
            int heldBlockLightValue;
            int heldItemId2;
            int heldBlockLightValue2;
            int fogMode;
            glm::vec3 fogColor;
            glm::vec3 skyColor;
            int worldTime;
            int moonPhase;
            float frameTimeCounter;
            float sunAngle;
            float shadowAngle;
            float rainStrength;
            float aspectRatio;
            float viewWidth;
            float viewHeight;
            float near; // No clue why this is a userless type name in empty declaration. Google gives no help
            float far;
            glm::vec3 sunPosition;
            glm::vec3 moonPosition;
            glm::vec3 shadowLightPosition;
            glm::vec3 upPosition;
            glm::vec3 cameraPosition;
            glm::vec3 previousCameraPosition;
            glm::mat4 gbufferModelView;
            glm::mat4 gbufferModelViewInverse;
            glm::mat4 gbufferPreviousModelView;
            glm::mat4 gbufferProjection;
            glm::mat4 gbufferProjectionInverse;
            glm::mat4 gbufferPreviousProjection;
            glm::mat4 shadowProjection;
            glm::mat4 shadowProjectionInverse;
            glm::mat4 shadowModelView;
            glm::mat4 shadowModelViewInverse;
            float wetness;
            float eyeAltitude;
            glm::ivec2 eyeBrightness;
            glm::ivec2 eyeBrightnessSmooth;
            glm::ivec2 terrainTextureSize;
            int terrainIconSize;
            int isEyeInWater;
            int hideGUI;
            float centerDepthSmooth;
            glm::ivec2 atlasSize;
            glm::vec4 entityColor;
            int entityId;
            int blockEntityId;
        };

        /*!
         * \brief Holds all the uniform variables that are specific to shadow passes
         */
        struct shadow_pass_uniforms {
            GLuint tex;
            GLuint texture;
            GLuint lightmap;
            GLuint normals;
            GLuint specular;
            GLuint shadow;
            GLuint watershadow;
            GLuint shadowtex0;
            GLuint shadowtex1;
            GLuint shadowcolor;
            GLuint shadowcolor0;
            GLuint shadowcolor1;
            GLuint noisetex;
        };

        /*!
         * \brief Holds all the uniform variables that are specific to gbuffer passes
         */
        struct gbuffer_pass_uniforms {
            GLuint texture;
            GLuint lightmap;
            GLuint normals;
            GLuint specular;
            GLuint shadow;
            GLuint watershadow;
            GLuint shadowtex0;
            GLuint shadowtex1;
            GLuint depthtex0;
            GLuint depthtex1;
            GLuint shadowcolor;
            GLuint shadowcolor0;
            GLuint shadowcolor1;
            GLuint noisetex;
        };

        /*!
         * \brief Holds all the uniform variables that are specific to the fullscreen passes
         */
        struct fullscreen_pass_uniforms {
            GLuint gcolor;
            GLuint gdepth;
            GLuint gnormal;
            GLuint composite;
            GLuint gaux1;
            GLuint gaux2;
            GLuint gaux3;
            GLuint gaux4;
            GLuint colortex0;
            GLuint colortex1;
            GLuint colortex2;
            GLuint colortex3;
            GLuint colortex4;
            GLuint colortex5;
            GLuint colortex6;
            GLuint colortex7;
            GLuint shadow;
            GLuint watershadow;
            GLuint shadowtex0;
            GLuint shadowtex1;
            GLuint gdepthtex;
            GLuint depthtex0;
            GLuint depthtex1;
            GLuint depthtex2;
            GLuint shadowcolor;
            GLuint shadowcolor0;
            GLuint shadowcolor1;
            GLuint noisetex;
        };

        /*!
         * \brief Defines a single mesh to draw
         */
        struct mesh {
            GLuint vao;

            // We need the bounds of the mesh for visibility culling
            float min_x;
            float min_y;
            float max_x;
            float max_y;
        };

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
            GLuint shader_program;

            std::vector<mesh> meshes_to_draw;
        };

        struct shadow_stage {
            std::vector<render_pass> meshes_to_draw;

            shadow_pass_uniforms uniforms;
        };

        struct gbuffer_stage {
            std::vector<render_pass> meshes_to_draw;

            gbuffer_pass_uniforms uniforms;
        };

        struct fullscreen_stage {
            std::vector<render_pass> meshes_to_draw;

            fullscreen_pass_uniforms uniforms;
        };

        struct render_command {
            std::vector<shadow_stage> shadow_passes;
            std::vector<gbuffer_stage> gbuffer_passes;
            std::vector<fullscreen_stage> fullscreen_passes;

            per_pass_uniforms uniforms;
        };
    }
}


#endif //RENDERER_RENDER_COMMAND_H
