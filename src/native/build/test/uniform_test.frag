#version 450

uniform mat4 gbufferModelviewInverse;
uniform mat4 gbufferProjectionInverse;
uniform vec3 sunPosition;
uniform int worldTime;

void main() {
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
