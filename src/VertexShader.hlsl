struct VSOut
{
	float3 color : Color;
	float4 pos : SV_Position;
};

cbuffer VP_CBuf : register(b0)
{
	matrix vp_transform;
};

cbuffer M_CBuf : register(b1)
{
	matrix model_transform;
};




VSOut main( float3 pos : Position, float4 color : Color )
{
	VSOut vso;
	//vso.pos = mul(mul( float4(pos,1.0f),model_transform ), vp_transform);
	vso.pos = mul( float4(pos,1.0f), vp_transform);
	vso.color = color;
	return vso;
}