static const float WhiteLumen = 11.2f;
static const float lumMin = 0.0f;
static const float lumMax = 1.0f;

float3 Uncharted2Tonemap(float3 x)
{
	static const float A = 0.15;
	static const float B = 0.50;
	static const float C = 0.10;
	static const float D = 0.20;
	static const float E = 0.02;
	static const float F = 0.30;

	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float getExposition(float lum)
{
	float keyValue = 1.03f - 2.0f / (2.0f + log(lum + 1));
	return keyValue / (max(min(lum, lumMax), lumMin));
}

float4 main(float3 c : Color) : SV_Target
{
	float lum = exp(tex2D(inLum, float2(0.5f, 0.5f)).a) - 1;

	float E = getExposition(lum);
	float4 toneMappedCol = Uncharted2Tonemap(c * E);
	float3 whiteScale = 1.0f / Uncharted2Tonemap(WhiteLumen);

	return float4(toneMappedCol * whiteScale, 1.0f);
}