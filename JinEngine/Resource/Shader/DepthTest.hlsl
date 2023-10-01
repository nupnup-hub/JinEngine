#include"DepthFunc.hlsl"

cbuffer cbObject : register(b0)
{
	float4x4 objWorld;
};
cbuffer cbPass: register(b1)
{
	float4x4 viewProj;
};

struct VertexIn
{
	float3 posL    : POSITION;
};
struct VertexOut
{
	float4 posH    : SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	float4 posW = mul(float4(vin.posL, 1.0f), objWorld);
	vout.posH = mul(posW, viewProj);
#if defined(OCCLUSION_QUERY)
	vout.posH.z *= 0.95f;
#endif
	return vout;
}