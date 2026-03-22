struct PSInput
{
    float4 position : SV_Position;
    float3 colour : COLOR;
};

struct PSOutput
{
    float4 colour : SV_Target0;
};

PSOutput Main(PSInput input)
{
    PSOutput output = (PSOutput)0;
    output.colour = float4(input.colour, 1.0);
    return output;
}