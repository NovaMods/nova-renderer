#version 430

layout(location = 0) uniform sampler2D albedo;

in vec2 uv;

layout(location = 0) out vec4 color;

void main() {
    color = texture2D(albedo, uv);
}
