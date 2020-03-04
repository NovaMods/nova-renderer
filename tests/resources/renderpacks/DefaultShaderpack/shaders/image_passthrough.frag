#version 450

//layout(set = 2, location = 0, binding = 0) uniform sampler2D tex;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 color;

void main() {
    color = vec4(1, 0, 1, 1); // texture(tex, uv);
}
