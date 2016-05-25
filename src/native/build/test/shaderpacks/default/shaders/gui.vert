#version 430

layout(location = 0) in vec4 position_in;
layout(location = 1) in vec4 uv_in;

layout(std140, binding = 0) uniform cameraData {
    mat4 gbufferModelView;
    mat4 gbufferProjection;
    mat4 gbufferModelviewInverse;
    mat4 gbufferProjectionInverse;
    vec3 cameraPosition;
};

out vec2 uv;

vec4 ftransform() {
    return gbufferProjection * gbufferModelView * position_in;
}

void main() {
    uv = uv_in.st;

	gl_Position = ftransform();
}
