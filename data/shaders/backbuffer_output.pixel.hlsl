[[vk::binding(0, 0)]]
Texture2D ui_output : register(t0);

[[vk::binding(1, 0)]]
Texture2D scene_output : register(t1);

[[vk::binding(2, 0)]]
SamplerState tex_sampler : register(s0);

struct VsOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float3 main(VsOutput input) : SV_Target {
    float4 ui_color = ui_output.Sample(tex_sampler, input.uv);
    float4 scene_color = scene_output.Sample(tex_sampler, input.uv);

    float3 combined_color = lerp(scene_color.rgb, ui_color.rgb, ui_color.a);
    
    return combined_color;
}
