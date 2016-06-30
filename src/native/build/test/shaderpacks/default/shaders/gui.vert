#version 450

#extension GL_ARB_enhanced_layouts : enable

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec2 uv_in;

layout(location = 20, std140) uniform ScreenSize {
    float viewWidth;
    float viewHeight;
};

out vec2 uv;

vec4 ftransform() {
    // Just scale the vertex down
    vec3 scaled_pos = position_in * vec3(1.0 / viewWidth, 1.0 / viewHeight, 1) + vec3(-1, -1, 0);
    return vec4(scaled_pos, 1);
}

void main() {
    uv = uv_in.st;

	gl_Position = ftransform();
}
