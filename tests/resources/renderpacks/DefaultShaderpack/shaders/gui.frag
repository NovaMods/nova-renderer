#version 460

// layout(set = 2, binding = 0) uniform sampler2D colortex;

/*
layout(set = 1, binding = 0) uniform per_frame_uniforms {
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
*/

layout(location = 0) in vec2 uv;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 color_out;

void main() {
    /*
    if(textureSize(colortex, 0).x > 0) {
        vec4 tex_sample = texture(colortex, mod(uv,1.0));
        if(tex_sample.a < 0.01) {
            discard;
        }
        color_out = tex_sample * color;
    } else {
        color_out = vec4(1, 0, 1, 1);
    }
    */
    color_out = vec4(1, 0, 1, 1);
}
