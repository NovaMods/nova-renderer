#version 430

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec2 uv_in;

layout(location = 20, std140) uniform CameraData {
    mat4 gbufferModelView;
    mat4 gbufferProjection;
    mat4 gbufferModelviewInverse;
    mat4 gbufferprojectionInverse;
    vec3 cameraPosition;
    float viewWidth;
    float viewHeight;
};

out vec2 uv;

void main() {
	gl_Position.xyz = position_in;// * vec3(1.0 / viewWidth, 1.0 / viewHeight, 1) + vec3(-1, -1, 0);
	gl_Position.w = 1.0;

	uv = uv_in;
	uv = vec2(1000 / viewWidth, 1000 / viewHeight);
}
