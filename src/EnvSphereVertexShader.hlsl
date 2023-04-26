struct VSOut
{
	float4 pos : SV_Position;
	float4 color : Color;
	float3 texCoord : TEXCOORD0;
};

cbuffer VP_CBuf : register(b0)
{
	matrix vp_transform;
};

cbuffer M_CBuf : register(b1)
{
	matrix model_transform;
};

VSOut main(float3 pos : Position, float3 norm : Normal)
{
	VSOut vso;
	vso.pos = mul(mul(float4(pos, 1.0f), model_transform), vp_transform);
	vso.color = float4(1, 1, 1, 1);
	vso.texCoord = pos.xyz;
	return vso;
}