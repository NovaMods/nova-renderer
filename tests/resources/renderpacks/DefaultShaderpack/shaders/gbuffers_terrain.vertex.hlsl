struct VertexInput {
    float3 position;
    float2 uv;
    float2 lightmap_uv;
    float3 normal;
    float4 color;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    float2 lightmap_uv : LMCOORD;
    float3 normal : NORMAL;
};

struct PerModelData {
    float4x4 model_matrix;
};

[[vk::binding(0, 0)]]
ConstantBuffer<PerModelData> per_model_uniforms;

VertexOutput main(VertexIntput input) {
    VertexOutput output;

    output.position = mul(float4(input.position, 1), per_model_uniforms.model_matrix);
    output.uv = input.uv;
    output.color = input.color;
    output.lightmap_uv = (input.lightmap_uv + float2(0.5)) / float2(256.0);
    output.normal = input.normal;

    return output;
}
