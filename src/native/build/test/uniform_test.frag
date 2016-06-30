#version 450

uniform mat4 gbufferModelviewInverse;
uniform mat4 gbufferProjectionInverse;
uniform vec3 sunPosition;
uniform int worldTime;

in vec3 normal;

layout(location = 0) out vec4 color;

void main() {
	color = vec4(1.0, 0.0, 0.0, 1.0);
}
