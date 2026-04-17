cbuffer Constants : register(b0)
{
    column_major float4x4 viewProjection;
};

struct VSInput
{
    float3 position: POSITION;
    float2 uv : TEXCOORD;
};

struct InstanceInput
{
    column_major float4x4 model : INST_MODEL;
    float2 sourcePos : INST_SRCPOS;
    float2 textureSize : INST_TEXSIZE;
    float2 spriteSize : INST_SPRSIZE;
};

struct VSOutput
{
    float4 position: SV_Position;
    float2 uv : TEXCOORD;
};

VSOutput VSMain(VSInput input, InstanceInput instance)
{
    float2 sourcePixels = (input.uv * instance.spriteSize) + instance.sourcePos;

    VSOutput output = (VSOutput) 0;

    float4 position = float4(input.position, 1.0f);
    output.position = mul(viewProjection, mul(instance.model, position));
    output.uv = sourcePixels / instance.textureSize;

    return output;
}