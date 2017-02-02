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
    if(textureSize(colortex, 0).x > 0) {
        vec4 tex_sample = texture(colortex, uv);
        if(tex_sample.a < 0.5) {
            discard;
        }
        color = vec3(tex_sample.rgb);
    } else {
        color = vec3(1, 0, 1);
    }
}
