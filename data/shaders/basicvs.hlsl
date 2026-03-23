struct VSInput
{
    float3 position: POSITION;
    float3 colour: COLOR;
};

struct VSOutput
{
    float4 position: SV_Position;
    float3 colour : COLOR;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.position = float4(input.position, 1.0);
    output.colour = input.colour;
    return output;
}