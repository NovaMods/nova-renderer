#version 450

layout(binding = 0, set = 2, location = 0) uniform sampler2D tex;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 color;

void main() {
    color = texture(tex, uv);
}
