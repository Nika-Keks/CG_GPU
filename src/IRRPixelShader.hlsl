#include "HDRI.hlsli"

float4 main(OutputVS vsout) : SV_TARGET
{
    float3 normal = normalize(vsout.worldPos.xyz);
    float3 tangent = normalize(normal.z < 10e-3 : float3(normal.y, -normal.x, 0) : float3(0, -normal.z, normal.y);
    float3 binormal = cross(normal, tangent);

    float4 irradiance = float4(0.0, 0.0, 0.0, 0.0);
    const uint N = 250;
    const uint K = 100;
    float PI = acos(-1);

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < K; ++j)
        {
            float phi = i * (2.0f * PI / N);
            float theta = j * (PI / 2.0f / K);
            float3 a = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 t_sample = a.x * tangent + a.y * binormal + a.z * normal;

            irradiance += Cubemap.Sample(EnvSampler, t_sample) * cos(theta) * sin(theta);
        }
    }

    irradiance = PI * irradiance / (N * K);

    return irradiance;
}