Texture2D tex : register(t0);
SamplerState pointsampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 color = tex.Sample(pointsampler, input.uv);
    
    if (color.a == 0)
        discard;
    
    return color;
}