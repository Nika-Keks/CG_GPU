#include "HDRI.hlsli"

TextureCube CubeMap : register(t0);

float4 main(OutputVS vsout) : SV_TARGET
{
    float3 normal = normalize(vsout.worldPos.xyz);
    float3 dir = abs(normal.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(dir, normal));
    float3 binormal = cross(normal, tangent);

    float4 irradiance = float4(0.0, 0.0, 0.0, 0.0);
    const int N = 200;
    const int K = 50;
    float PI = acos(-1);

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < K; ++j)
        {
            float phi = i * (2.0f * PI / N);
            float theta = j * (PI / 2.0f / K);
            float3 a = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 t_sample = a.x * tangent + a.y * binormal + a.z * normal;

            irradiance += CubeMap.Sample(EnvSampler, t_sample) * cos(theta) * sin(theta);
        }
    }

    irradiance = PI * irradiance / (N * K);

    return irradiance;
}