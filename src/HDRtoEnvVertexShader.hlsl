#include "HDRI.hlsli"

float4 main(vid : SV_VERTEXID) : SV_POSITION
{
	OutputVS output;
	output.worldPos = mul(positions[vid], mMatrix);
	output.pos = mul(output.worldPos, vpMatrix);

	return output;
}