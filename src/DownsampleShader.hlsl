sampler TextureSampler;

static const float3 ChannelsWeight = float3(0.2126f, 0.7151f, 0.0722f);

struct PS_INPUT
{
	float2 coord0				: TEXCOORD0;
	float2 coord1				: TEXCOORD1;
	float2 coord2				: TEXCOORD2;
	float2 coord3				: TEXCOORD3;
};

float4 GrayscaleDownsample(PS_INPUT i) : SV_TARGET
{
	float4 s0, s1, s2, s3;
	s0 = tex2D(TextureSampler, i.coord0);
	s1 = tex2D(TextureSampler, i.coord1);
	s2 = tex2D(TextureSampler, i.coord2);
	s3 = tex2D(TextureSampler, i.coord3);

	float4 vResult = { 0, 0, 0, 0 };

	vResult.a = (
		log(dot(s0.rgb, ChannelsWeight) + 1) +
		log(dot(s1.rgb, ChannelsWeight) + 1) +
		log(dot(s2.rgb, ChannelsWeight) + 1) +
		log(dot(s3.rgb, ChannelsWeight) + 1)
		) * 0.25f;

	return vResult;
}

float4 Downsample(PS_INPUT i) : SV_TARGET
{
	float4 s0, s1, s2, s3;
	s0 = tex2D(TextureSampler, i.coord0);
	s1 = tex2D(TextureSampler, i.coord1);
	s2 = tex2D(TextureSampler, i.coord2);
	s3 = tex2D(TextureSampler, i.coord3);

	float4 vResult = (s0 + s1 + s2 + s3) * 0.25f;
	return vResult;
}