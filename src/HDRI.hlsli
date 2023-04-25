
struct OutputVS
{
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION1;
};

static float4 positions[4] =
{
    { -1.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    { -1.0f, -1.0f, 1.0f, 1.0f },
    { 1.0f, -1.0f, 1.0f, 1.0f }
};

TextureCube CubeMap : register(t0);

cbuffer ConstBuffer : register(b0)
{
    float4x4 mMatrix;
    float4x4 vpMatrix;
}

SamplerState EnvSampler : register(s0);
