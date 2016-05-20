#version 430

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec2 lmcoord;
layout(location = 3) in vec3 normal_in;

uniform mat4 gbufferModelviewMatrix;
uniform mat4 gbufferProjectionMatrix;

out vec3 normal;

void main() {
	gl_Position = gbufferModelviewMatrix * gbufferProjectionMatrix * position;

	normal = normal_in;
}
