#version 450

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec2 uv_in;

layout(std140) uniform per_frame_uniforms {
    int heldItemId;
    int heldBlockLightValue;
    int heldItemId2;
    int heldBlockLightValue2;

    int forMode;
    vec3 fogColor;

    vec3 skyColor;

    int worldTime;
    float frameTimeCounter;

    int moonPhase;
    float sunAngle;
    float shadowAngle;

    float rainStrength;
    float aspectRatio;

    float viewWidth;
    float viewHeight;
    float near;
    float far;

    vec3 sunPosition;
    vec3 moonPosition;
    vec3 shadowLightPosition;

    vec3 upPosition;
    vec3 cameraPosition;
    vec3 previousCameraPosition;

    mat4 gbufferModelView;
    mat4 gbufferModelViewInverse;
    mat4 gbufferPreviousModelView;
    mat4 gbufferProjection;
    mat4 gbufferProjectionInverse;
    mat4 gbufferPreviousProjection;

    mat4 shadowProjection;
    mat4 shadowProjectionInverse;
    mat4 shadowModelView;
    mat4 shadowModelViewInverse;

    float wetness;
    float eyeAltitude;

    ivec2 eyeBrightness;
    ivec2 eyeBrightnessSmooth;
    ivec3 terrainTextureSize;
    int terrainIconSize;
    int isEyeInWater;
    int hideGUI;
    float centerDepthSmooth;
    ivec2 atlasSize;
    vec4 entityColor;
    int entityId;
    int blockEntityId;
};

out vec2 uv;

void main() {
	gl_Position = gbufferModelView * vec4(position_in, 1.0f);

	uv = uv_in;
}