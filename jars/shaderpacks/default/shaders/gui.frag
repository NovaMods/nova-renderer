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
in vec3 color;

out vec3 color_out;

void main() {
    if(textureSize(colortex, 0).x > 0) {
        vec4 tex_sample = texture(colortex, uv);
        if(tex_sample.a < 0.5) {
            discard;
        }
        color_out = vec3(tex_sample.rgb) * color;
    } else {
        color_out = vec3(1, 0, 1);
    }
}
