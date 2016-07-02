#version 450

uniform sampler2D albedo;

in vec2 uv;

layout(location = 0) out vec4 color;

void main() {
    color = vec4(1, 0, 0, 1);
}
