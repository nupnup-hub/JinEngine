
#if defined(STATIC)
struct VertexIn
{
	float3 PosL    : POSITION;
};
struct VertexOut
{
	float4 PosH    : SV_POSITION;
};
#elif defined(SKINNED)
struct VertexIn
{
	float3 PosL    : POSITION;
	float3 BoneWeights : WEIGHTS;
	uint4 BoneIndices  : BONEINDICES;
};
struct VertexOut
{
	float4 PosH    : SV_POSITION;
};
#endif

cbuffer cbObject : register(b0)
{
	float4x4 objWorld;
};
cbuffer cbSkinned : register(b1)
{
	float4x4 objBoneTransforms[256];
};
cbuffer cbCamera: register(b2)
{
	float4x4 camViewProj;
};

#if defined(STATIC)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	float4 posW = mul(float4(vin.PosL, 1.0f), objWorld);
	vout.PosH = mul(posW, camViewProj);
	return vout;
}
#elif defined(SKINNED)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.BoneWeights.x;
	weights[1] = vin.BoneWeights.y;
	weights[2] = vin.BoneWeights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; ++i)
		posL += weights[i] * mul(float4(vin.PosL, 1.0f), objBoneTransforms[vin.BoneIndices[i]]).xyz;

	vin.PosL = posL;
	float4 posW = mul(float4(vin.PosL, 1.0f), objWorld);
	vout.PosH = mul(posW, camViewProj);

	return vout;
}
#endif