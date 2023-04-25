#include "HDRI.hlsli"

float4 main(OutputVS vsout) : SV_TARGET
{
    float PI = acos(-1);
    float3 wPos = normalize(input.worldPosition.xyz);

    float2 t_sample = {
        1.0f - atan2(wPos.z, wPos.x) / (2 * PI),
        1.0f - (0.5f + asin(wPos.y) / PI)
    };

    return EnvSampler.Sample(MinMagLinearSampler, float2(u, v));
}