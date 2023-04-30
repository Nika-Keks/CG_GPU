#include "HDRI.hlsli"
#include "SpecularHeader.hlsli"

float SchlickGGX(float3 n, float3 v, float k)
{
    float nv = positiveDot(n, v);
    return nv / (nv * (1 - k) + k);
}

float geometry(float3 n, float3 v, float3 l, float roughness)
{
    float alpha = min(max(roughness, 0.01f), 1);
    float k = sqr(alpha) / 2;

    return SchlickGGX(n, v, k) * SchlickGGX(n, l, k);
}

float2 IntegrateBRDF(float NdotV, float roughness)
{
    float3 V;
    V.x = sqrt(1.0 - NdotV);
    V.z = 0.0;
    V.y = NdotV;
    float A = 0.0;
    float B = 0.0;
    float3 N = float3(0.0, 1.0, 0.0);
    static const uint SAMPLE_COUNT = 1024u;

    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V);
        float NdotL = max(L.y, 0.0);
        float NdotH = max(H.y, 0.0);
        float VdotH = max(dot(V, H), 0.0);
        if (NdotL > 0.0)
        {
            float G = geometry(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);
            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return float2(A, B);
}

float2 main(OutputVS input) : SV_TARGET
{
    float2 outpos = (float2(input.worldPos.x, -input.worldPos.y) + 1) / 2;
    float NdotV = outpos.x;
    float r = outpos.y;
	
    return IntegrateBRDF(NdotV, r);
}