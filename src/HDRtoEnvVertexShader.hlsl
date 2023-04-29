#include "HDRI.hlsli"

static float4 positions[4] =
{
    { -1.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    { -1.0f, -1.0f, 1.0f, 1.0f },
    { 1.0f, -1.0f, 1.0f, 1.0f }
};

cbuffer ConstBuffer : register(b0)
{
    float4x4 mMatrix;
    float4x4 vpMatrix;
}

OutputVS main(InputVS i)
{
	OutputVS output;
	output.worldPos = mul(positions[i.vertexId], mMatrix);
	output.pos = mul(output.worldPos, vpMatrix);

	return output;
}