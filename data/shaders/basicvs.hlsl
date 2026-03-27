cbuffer Constants : register(b0)
{
    column_major float4x4 model;
    column_major float4x4 viewProjection;
};

struct VSInput
{
    float3 position: POSITION;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 position: SV_Position;
    float2 uv : TEXCOORD;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output = (VSOutput)0;
    float4 position = float4(input.position, 1.0f);
    output.position = mul(viewProjection, mul(model, position));
    output.uv = input.uv;
    return output;
}