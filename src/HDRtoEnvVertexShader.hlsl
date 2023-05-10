#include "HDRI.hlsli"

OutputVS main(InputVS i)
{
	OutputVS output;
	output.worldPos = mul(positions[i.vertexId], mMatrix);
	output.pos = mul(output.worldPos, vpMatrix);

	return output;
}