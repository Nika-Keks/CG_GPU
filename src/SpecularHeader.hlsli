static const float PI = acos(-1);

float sqr(float x)
{
	return x * x;
}

float positiveDot(float3 a, float3 b)
{
	return max(dot(a, b), 0);
}

float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
	return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 norm, float roughness)
{
	float a = roughness * roughness;
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	float3 H;
	H.x = cos(phi) * sinTheta;
	H.z = sin(phi) * sinTheta;
	H.y = cosTheta;

	float3 up = abs(norm.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
	float3 tangent = normalize(cross(up, norm));
	float3 bitangent = cross(norm, tangent);
	float3 sampleVec = tangent * H.x + bitangent * H.z + norm * H.y;
	return normalize(sampleVec);
}

float normalDistribution(float3 halfV, float3 norm, float roughness)
{
	float3 n = normalize(norm);
	float alpha = min(max(roughness, 0.01f), 1);

	float alphaSqr = sqr(alpha);
	return alphaSqr / (PI * sqr(sqr(positiveDot(norm, halfV)) * (alphaSqr - 1) + 1));
}