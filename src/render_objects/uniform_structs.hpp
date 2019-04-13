#pragma once
#include <glm/glm.hpp>

namespace nova::renderer {
    struct PerFrameUniforms {
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
        glm::vec3 skyColor;
        glm::vec3 sunPosition;
        glm::vec3 moonPosition;
        glm::vec3 shadowLightPosition;
        glm::vec3 upPosition;
        glm::vec3 cameraPosition;
        glm::vec3 previousCameraPosition;
        glm::ivec2 eyeBrightness;
        glm::ivec2 eyeBrightnessSmooth;
        glm::ivec2 terrainTextureSize;
        glm::ivec2 atlasSize;
        int heldItemId;
        int heldBlockLightValue;
        int heldItemId2;
        int heldBlockLightValue2;
        int fogMode;
        int worldTime;
        int moonPhase;
        int terrainIconSize;
        int isEyeInWater;
        int hideGUI;
        int entityId;
        int blockEntityId;
        float frameTimeCounter;
        float sunAngle;
        float shadowAngle;
        float rainStrength;
        float aspectRatio;
        float viewWidth;
        float viewHeight;
        float clip_distance_near;
        float clip_distance_far;
        float wetness;
        float eyeAltitude;
        float centerDepthSmooth;
    };
} // namespace nova::renderer
