/*!
 * \brief Defines a number of UBOs that are sent to shaders
 *
 * I've put them all in one header file so that there's a single place to see all the available uniforms. When this
 * project is a bit closer to release, I'll document this file so that the Doxygen generated documentation will serve
 * as a reference of what uniforms are available through Nova.
 *
 * \author David
 * \date 19-May-16.
 */

#ifndef RENDERER_UNIFORM_BUFFERS_H
#define RENDERER_UNIFORM_BUFFERS_H

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace nova {
    namespace view {
        /*!
         * \brief Holds all the uniform variables that are shared by all shader executions
         *
         * The data in the structure should only be uploaded once per frame
         */
        struct per_frame_uniforms {
            GLint heldItemId;
            GLint heldBlockLightValue;
            GLint heldItemId2;
            GLint heldBlockLightValue2;
            GLint fogMode;
            glm::vec3 fogColor;
            glm::vec3 skyColor;
            GLint worldTime;
            GLint moonPhase;
            GLfloat frameTimeCounter;
            GLfloat sunAngle;
            GLfloat shadowAngle;
            GLfloat rainStrength;
            GLfloat aspectRatio;
            GLfloat viewWidth;
            GLfloat viewHeight;
            GLfloat nearPlane;  // near in the shaders. Re-named because GCC was yelling about "This line does not declare anything", like it's some great authority on declaring things
            GLfloat farPlane;   // far in the shaders
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
            GLfloat wetness;
            GLfloat eyeAltitude;
            glm::ivec2 eyeBrightness;
            glm::ivec2 eyeBrightnessSmooth;
            glm::ivec2 terrainTextureSize;
            GLint terrainIconSize;
            GLint isEyeInWater;
            GLint hideGUI;
            GLfloat centerDepthSmooth;
            glm::ivec2 atlasSize;
            glm::vec4 entityColor;
            GLint entityId;
            GLint blockEntityId;
        };

        /*!
         * \brief Holds all the uniform variabled needed by the GUI
         */
        struct gui_uniforms {
            glm::mat4 gbufferModelView;
            glm::mat4 gbufferProjection;
        };

        /*!
         * \brief Holds all the uniform variables that are specific to shadow passes
         */
        struct shadow_pass_textures {
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
        struct gbuffer_pass_textures {
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
        struct fullscreen_pass_textures {
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
    }
}

#endif //RENDERER_UNIFORM_BUFFERS_H
