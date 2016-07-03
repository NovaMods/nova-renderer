#version 430

layout(location = 0) in vec3 position_in;

void main() {
	gl_Position.xyz = position_in;
	gl_Position.w = 1.0;
}
