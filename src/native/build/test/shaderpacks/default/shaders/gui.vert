#version 430

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec2 uv_in;

out vec2 uv;

vec4 ftransform() {
    return vec4(position_in, 1);
}

void main() {
    uv = uv_in.st;

	gl_Position = ftransform();
}
