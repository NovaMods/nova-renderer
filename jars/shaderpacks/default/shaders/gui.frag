#version 450

layout(binding = 0) uniform sampler2D colortex;

layout(shared, std140) uniform gui_uniforms {
    float viewWidth;
    float viewHeight;
    mat4 gbufferModelView;
    mat4 gbufferProjection;
    float aspectRatio;
    float frameTimeCounter;
    int hideGUI;
};

in vec2 uv;

out vec3 color;

void main() {
    color = texture(colortex, uv).rgb;

    color = vec3(1, 0, 1);
}
