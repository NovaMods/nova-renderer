#version 430

in vec2 uv;

out vec3 color;

void main() {
    color = vec3(uv, 0);
}
