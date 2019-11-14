#version 450

layout(location = 0) in vec3 position_in;
layout(location = 2) in vec2 uv_in;
layout(location = 3) in vec2 lightmap_uv_in;
layout(location = 4) in vec3 normal_in;

layout(std140) uniform per_frame_uniforms {
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
    vec4 entityColor;
    vec3 fogColor;
    vec3 skyColor;
    vec3 sunPosition;
    vec3 moonPosition;
    vec3 shadowLightPosition;
    vec3 upPosition;
    vec3 cameraPosition;
    vec3 previousCameraPosition;
    ivec2 eyeBrightness;
    ivec2 eyeBrightnessSmooth;
    ivec2 terrainTextureSize;
    ivec2 atlasSize;
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
    float near;
    float far;
    float wetness;
    float eyeAltitude;
    float centerDepthSmooth;
};

layout(std140) readonly buffer per_model_uniforms {
    mat4 modelMatrices[];
};

layout(location = 0) out vec2 uv;
layout(location = 1) out vec4 color;

void main() {
	gl_Position = gbufferProjection * gbufferModelView * gbufferModel * vec4(position_in, 1.0f);

	uv = uv_in;
	color = vec4(1);
}