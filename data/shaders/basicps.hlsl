struct PSInput
{
    float4 position : SV_Position;
};

struct PSOutput
{
    float4 colour : SV_Target0;
};

PSOutput PSMain(PSInput input)
{
    PSOutput output = (PSOutput)0;
    output.colour = float4(1.0f, 0.5f, 0.0f, 1.0f);
    return output;
}