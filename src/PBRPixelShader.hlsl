#include "PBRPixelShader.cginc"

TextureCube DiffuseIrradianceMap : register(t0);
TextureCube EnvMap : register(t1);
Texture2D BRDFLut : register(t2);

SamplerState MinMagMipLinear : register(s0);
SamplerState MinMagLinearMipPointBorder : register(s1);


float normalDistribution(float3 wPos, float3 norm, int lightIdx)
{
	float3 v = toCamera(wPos);
	float3 l = toLight(lightIdx, wPos);

	float3 n = normalize(norm);
	float3 h = normalize(l + v);
	float alpha = min(max(roughness, 0.01f), 1);

	float alphaSqr = sqr(alpha);
	return alphaSqr / (PI * sqr(sqr(positiveDot(n, h)) * (alphaSqr - 1) + 1));
}

float geometry(float3 wPos, float3 norm, int lightIdx)
{
	float alpha = min(max(roughness, 0.01f), 1);
	float k = sqr(alpha + 1) / 8;

	float3 n = normalize(norm);
	float3 v = toCamera(wPos);
	float3 l = toLight(lightIdx, wPos);

	return SchlickGGX(n, v, k) * SchlickGGX(n, l, k);
}

float3 fresnel(float3 wPos, float3 norm, int lightIdx)
{
	float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metalness);
	float3 v = toCamera(wPos);
	float3 l = toLight(lightIdx, wPos);
	float3 h = normalize(l + v);

	return F0 + (1 - F0) * pow(1 - positiveDot(h, v), 5);
}

float3 FresnelSchlickRoughnessFunction(float3 wPos, float3 norm)
{
	float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metalness);
	float3 v = toCamera(wPos);
	float3 ir = float3(1.f, 1.f, 1.f) * max(1 - roughness, 10e-3);
	return F0 + (max(ir, F0) - F0) * pow(1 - positiveDot(norm, v), 5);
}

float4 main(PSInput input) : SV_TARGET
{
	float3 result = {0.f, 0.f, 0.f};
	for (uint i = 0; i < plbLenght.r; ++i)
	{
		float D = normalDistribution(input.wPos.xyz, input.norm.xyz, i);
		float G = geometry(input.wPos.xyz, input.norm.xyz, i);
		float3 F = fresnel(input.wPos.xyz, input.norm.xyz, i);
		float3 wo = toCamera(input.wPos);
		float3 l = toLight(i, input.wPos);
		float3 n = normalize(input.norm.xyz);
		float3 result_add = { 0.f, 0.f, 0.f };
		if (viewMode == pbrMode)
			result_add = (1 - F) * albedo / PI * (1 - metalness) + D * F * G / (0.001f + 4 * (positiveDot(l, n) * positiveDot(wo, n)));
		else if (viewMode == normalMode)
			result_add = D;
		else if (viewMode == geometryMode)
			result_add = G;
		else if (viewMode == fresnelMode)
			result_add = F;
		result += result_add * (dot(l, n) > 0);
	}

	float3 wPos = normalize(input.wPos.xyz);
	float3 v = toCamera(input.wPos);
	float3 n = normalize(input.norm.xyz);
	float3 r = normalize(2.0f * dot(v, n) * n - v);

	static const float MAX_REFLECTION_LOD = 4.0;
	float3 prefilteredColor = EnvMap.SampleLevel(MinMagMipLinear, r, roughness * MAX_REFLECTION_LOD);
	float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo.xyz, metalness);
	float3 F = FresnelSchlickRoughnessFunction(wPos, n);
	float2 envBRDF = BRDFLut.Sample(MinMagLinearMipPointBorder, float2(max(dot(n, v), 0.0), roughness));
	float3 specular = prefilteredColor * (F0 * envBRDF.x + envBRDF.y);
	
	float3 kS = F;
	float3 kD = float3(1.0, 1.0, 1.0) - kS;
	kD *= 1.0 - metalness;
	float3 irradiance = DiffuseIrradianceMap.Sample(MinMagMipLinear, n);
	float3 diffuse = irradiance * albedo;
	float3 ambient = (kD * diffuse + specular);

	return float4(result + ambient, 1);
}