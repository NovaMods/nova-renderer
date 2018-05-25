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

#include <glm/glm.hpp>
#include <easylogging++.h>

namespace nova {
    /*!
     * \brief Holds all the uniform variables that are shared by all shader executions
     *
     * The data in the structure should only be uploaded once per frame
     */
    struct per_frame_uniforms {
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
        glm::vec4 entityColor;
        glm::vec3 fogColor;
        float padding1; // needed to fit the GLSL std140 alignment rules. A future version will pack the data here more efficiently
        glm::vec3 skyColor;
        float padding2;
        glm::vec3 sunPosition;
        float padding3;
        glm::vec3 moonPosition;
        float padding4;
        glm::vec3 shadowLightPosition;
        float padding5;
        glm::vec3 upPosition;
        float padding6;
        glm::vec3 cameraPosition;
        float padding7;
        glm::vec3 previousCameraPosition;
        float padding8;
        glm::ivec2 eyeBrightness;
        glm::ivec2 eyeBrightnessSmooth;
        glm::ivec2 terrainTextureSize;
        glm::ivec2 atlasSize;
        uint32_t heldItemId;
        uint32_t heldBlockLightValue;
        uint32_t heldItemId2;
        uint32_t heldBlockLightValue2;
        uint32_t fogMode;
        uint32_t worldTime;
        uint32_t moonPhase;
        uint32_t terrainIconSize;
        uint32_t isEyeInWater;
        uint32_t hideGUI;
        uint32_t entityId;
        uint32_t blockEntityId;
        float frameTimeCounter;
        float sunAngle;
        float shadowAngle;
        float rainStrength;
        float aspectRatio;
        float viewWidth;
        float viewHeight;
        float nearPlane;  // near in the shaders. Re-named because GCC was yelling about "This line does not declare anything", like it's some great authority on declaring things
        float farPlane;   // far in the shaders
        float wetness;
        float eyeAltitude;
        float centerDepthSmooth;
    };

    /*!
     * \brief Holds all the uniform variables that are specific to shadow passes
     */
    struct shadow_pass_textures {
        uint32_t tex;
        uint32_t texture;
        uint32_t lightmap;
        uint32_t normals;
        uint32_t specular;
        uint32_t shadow;
        uint32_t watershadow;
        uint32_t shadowtex0;
        uint32_t shadowtex1;
        uint32_t shadowcolor;
        uint32_t shadowcolor0;
        uint32_t shadowcolor1;
        uint32_t noisetex;
    };

    /*!
     * \brief Holds all the uniform variables that are specific to gbuffer passes
     */
    struct gbuffer_pass_textures {
        uint32_t texture;
        uint32_t lightmap;
        uint32_t normals;
        uint32_t specular;
        uint32_t shadow;
        uint32_t watershadow;
        uint32_t shadowtex0;
        uint32_t shadowtex1;
        uint32_t depthtex0;
        uint32_t depthtex1;
        uint32_t shadowcolor;
        uint32_t shadowcolor0;
        uint32_t shadowcolor1;
        uint32_t noisetex;
    };

    /*!
     * \brief Holds all the uniform variables that are specific to the fullscreen passes
     */
    struct fullscreen_pass_textures {
        uint32_t gcolor;
        uint32_t gdepth;
        uint32_t gnormal;
        uint32_t composite;
        uint32_t gaux1;
        uint32_t gaux2;
        uint32_t gaux3;
        uint32_t gaux4;
        uint32_t colortex0;
        uint32_t colortex1;
        uint32_t colortex2;
        uint32_t colortex3;
        uint32_t colortex4;
        uint32_t colortex5;
        uint32_t colortex6;
        uint32_t colortex7;
        uint32_t shadow;
        uint32_t watershadow;
        uint32_t shadowtex0;
        uint32_t shadowtex1;
        uint32_t gdepthtex;
        uint32_t depthtex0;
        uint32_t depthtex1;
        uint32_t depthtex2;
        uint32_t shadowcolor;
        uint32_t shadowcolor0;
        uint32_t shadowcolor1;
        uint32_t noisetex;
    };

    el::base::Writer& operator<<(el::base::Writer& out, const glm::mat4& mat);
}

#endif //RENDERER_UNIFORM_BUFFERS_H
