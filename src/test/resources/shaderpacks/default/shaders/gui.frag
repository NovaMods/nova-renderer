#version 450

layout(binding = 0) uniform sampler2D colortex;

in vec2 uv;

out vec3 color;

void main() {
    color = texture(colortex, uv).rgb;

    color = vec3(1, 0, 1);
}
