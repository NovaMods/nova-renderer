#version 450

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec2 uv_in;

layout(binding = 20, std140) uniform gui_uniforms {
    mat4 gbufferModelView;
    mat4 gbufferProjection;
};

out vec2 uv;

void main() {
	gl_Position = gbufferModelView * vec4(position_in, 1.0f);

	uv = uv_in;
}