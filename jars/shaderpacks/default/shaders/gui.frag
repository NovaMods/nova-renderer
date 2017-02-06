#version 450

layout(binding = 0) uniform sampler2D colortex;

layout(std140) uniform gui_uniforms {
    mat4 gbufferModelView;
    mat4 gbufferProjection;
	float viewWidth;
    float viewHeight;
    float aspectRatio;
    float frameTimeCounter;
    int hideGUI;
};

in vec2 uv;
in vec4 color;

out vec4 color_out;

void main() {
    if(textureSize(colortex, 0).x > 0) {
        vec4 tex_sample = texture(colortex, uv);
        if(tex_sample.a < 0.01) {
            discard;
        }
        color_out = tex_sample * color;
    } else {
        color_out = vec4(1, 0, 1, 1);
    }
}
