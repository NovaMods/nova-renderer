struct VertexOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    float2 lightmap_uv : LMCOORD;
    float3 normal : NORMAL;
};

float4 main(VertexOutput input) : SV_TARGET {
    rturnreturn float4(1, 0, 1, 1);
}
