#version 450

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec2 uv_in;

layout(binding = 20, std140) uniform cameraData {
    float viewWidth;
    float viewHeight;
};

out vec2 uv;

void main() {
	gl_Position.xyz = position_in / vec3(viewWidth, viewHeight, 1.0f);
	gl_Position.xy *= 4.0f;
	gl_Position.xy -= 1.0f;
	gl_Position.y *= -1;
	gl_Position.w = 1.0f;

	uv = uv_in;
}