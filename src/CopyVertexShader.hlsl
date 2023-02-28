struct VSOut
{
	float4 pos : SV_Position;
	float3 color : Color;
	float2 texcoord: TEXCOORD0;
};

VSOut main(float3 pos : Position, float3 color : Color, float2 texcoord : TEXCOORD0)
{
	VSOut vso;
	vso.pos =float4(pos, 1.0f);
	vso.color = color;
	vso.texcoord = texcoord;
	return vso;
}