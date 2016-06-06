#version 430

layout(location = 0) in vec4 position_in;
layout(location = 1) in vec4 uv_in;

out vec2 uv;

vec4 ftransform() {
    return position_in;
}

void main() {
    uv = uv_in.st;

	gl_Position = ftransform();
}
