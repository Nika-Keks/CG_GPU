#include "HDRI.hlsli"
#include "SpecularHeader.hlsli"

cbuffer Constantbuffer : register(b0)
{
	float roughness;
};

TextureCube EnvironmentMap : register(t0);
SamplerState MinMagMipLinear : register(s0);

float4 main(OutputVS input) : SV_TARGET
{
	float3 norm = normalize(input.worldPos);
	float3 view = norm;
	float totalWeight = 0.0;
	float3 prefilteredColor = float3(0, 0, 0);
	static const uint SAMPLE_COUNT = 1024u;
	for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
		float2 Xi = Hammersley(i, SAMPLE_COUNT);
		float3 H = ImportanceSampleGGX(Xi, norm, roughness);
		float3 L = normalize(2.0 * dot(view, H) * H - view); 
		float ndotl = max(dot(norm, L), 0.0);
		float ndoth = max(dot(norm, H), 0.0);
		float hdotv = max(dot(H, view), 0.0);
		
		float D = normalDistribution(H, norm, roughness);
		float pdf = (D * ndoth / (4.0 * hdotv)) + 0.0001;
		float resolution = 512.0;
		float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
		float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
		float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

		if (ndotl > 0.0) {
			prefilteredColor += EnvironmentMap.SampleLevel(MinMagMipLinear, L, mipLevel) * ndotl;
			totalWeight += ndotl;
		}
	}
	prefilteredColor = prefilteredColor / totalWeight;
	
	return float4(prefilteredColor, 1.0f);
}