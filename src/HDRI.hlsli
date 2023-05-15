
struct OutputVS
{
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION1;
};


struct InputVS
{
    uint vertexId   : SV_VERTEXID;
};