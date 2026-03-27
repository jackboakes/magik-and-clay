cbuffer Constants : register(b0)
{
    column_major float4x4 model;
    column_major float4x4 viewProjection;
};

struct VSInput
{
    float3 position: POSITION;
};

struct VSOutput
{
    float4 position: SV_Position;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output = (VSOutput)0;
    float4 position = float4(input.position, 1.0f);
    output.position = mul(viewProjection, mul(model, position));
    return output;
}