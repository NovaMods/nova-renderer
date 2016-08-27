#version 450

in vec2 uv;

out vec3 color;

void main() {
    color = vec3(uv, 1);
}
