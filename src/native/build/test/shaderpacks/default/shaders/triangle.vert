#version 450

layout(location = 0) in vec3 position_in;
//layout(location = 1)

void main() {
    gl_Position = vec4(position_in, 1.0);
}
