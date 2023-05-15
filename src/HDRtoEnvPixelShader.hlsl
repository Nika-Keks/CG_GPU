#include "HDRI.hlsli"

Texture2D HDRTexture : register(t0);
SamplerState EnvSampler : register(s0);

float4 main(OutputVS vsout) : SV_TARGET
{
    float PI = acos(-1);
    float3 wPos = normalize(vsout.worldPos.xyz);

    float2 t_sample = {
        1.0f - atan2(wPos.z, wPos.x) / (2 * PI),
        1.0f - (0.5f + asin(wPos.y) / PI)
    };

    return HDRTexture.Sample(EnvSampler, float3(t_sample, 0));
}