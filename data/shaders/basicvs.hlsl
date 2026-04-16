cbuffer Constants : register(b0)
{
    column_major float4x4 model;
    column_major float4x4 viewProjection;
    float2 sourcePosition;
    float2 textureSize;
    float2 spriteSize;
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
    float2 sourcePixels = (input.uv * spriteSize) + sourcePosition;
    
    VSOutput output = (VSOutput)0;
    
    float4 position = float4(input.position, 1.0f);
    output.position = mul(viewProjection, mul(model, position));
    output.uv = sourcePixels / textureSize;
    
    return output;
}