struct VsInput {
    float2 position : POSITION;
};

struct VsOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VsOutput main(VsInput input) {
    VsOutput output;
    output.position = float4(input.position * 2.0 - 1.0, 0, 1);
    output.uv = input.position;
    return output;
}
