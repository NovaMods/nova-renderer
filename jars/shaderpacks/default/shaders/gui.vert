#version 450

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec2 uv_in;
layout(location = 2) in vec4 color_in;

layout(binding = 0,std140) uniform gui_uniforms {
	mat4 gbufferModelView;
    mat4 gbufferProjection;
	float viewWidth;
    float viewHeight;
    float aspectRatio;
    float frameTimeCounter;
    int hideGUI;
};

out vec2 uv;
out vec4 color;

void main() {
	gl_Position = gbufferModelView * vec4(position_in, 1.0f);

	uv = uv_in;
	color = color_in;
}