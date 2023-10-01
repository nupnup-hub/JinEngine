
#if !defined(ARRAY_COUNT)
#define ARRAY_COUNT 8
#endif
#define CUBE_MAP_FACE 6

cbuffer cbObject : register(b0)
{
	float4x4 objWorld;
	//float4x4 objTexTransform;
	//uint objMaterialIndex;
	//uint objPad00;
	//uint objPad01;
	//uint objPad02;
};
cbuffer cbSkinned : register(b1)
{
	float4x4 objBoneTransforms[256];
};
cbuffer cbNormalLightShadowMap : register(b2)
{
	float4x4 shadowT;
};
cbuffer cbCsm : register (b3)
{
	float4x4 csmT[ARRAY_COUNT];
};
cbuffer cbPointLightShadowMap : register (b4)
{
	float4x4 pointLightShadowT[CUBE_MAP_FACE];
};

//
// Csm Contents
//
#if defined(ARRAY)
#if defined(STATIC)
struct VertexIn
{
	float3 posL    : POSITION;
};
struct VertexOut
{
	float4 posW    : POSITION;
};
#elif defined(SKINNED)
struct VertexIn
{
	float3 posL    : POSITION;
	float3 boneWeights : WEIGHTS;
	uint4 boneIndices  : BONEINDICES;
};
struct VertexOut
{
	float4 posW    : POSITION;
};
#endif
struct GsOut
{
	float4 posH		: SV_POSITION;
	uint rTIndex	: SV_RenderTargetArrayIndex;
};
#if defined(STATIC)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	vout.posW = mul(float4(vin.posL, 1.0f), objWorld);
	return vout;
}
#elif defined(SKINNED)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.boneWeights.x;
	weights[1] = vin.boneWeights.y;
	weights[2] = vin.boneWeights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; ++i)
		posL += weights[i] * mul(float4(vin.posL, 1.0f), objBoneTransforms[vin.boneIndices[i]]).xyz;

	vout.posW = mul(float4(vin.posL, 1.0f), objWorld);
	return vout;
}
#endif
[maxvertexcount(ARRAY_COUNT * 3)]
void GS(triangle VertexOut input[3], inout TriangleStream<GsOut> output)
{
	for (int i = 0; i < ARRAY_COUNT; ++i)
	{
		GsOut gsOut;
		gsOut.rTIndex = i;
		for (int j = 0; j < 3; j++)
		{
			//can add bias
			gsOut.posH = mul(input[j].posW, csmT[i]);
			output.Append(gsOut);
		}
		output.RestartStrip();
	}
}
#endif

#if defined(CUBE)
#if defined(STATIC)
struct VertexIn
{
	float3 posL    : POSITION;
};
struct VertexOut
{
	float4 posW    : POSITION;
};
#elif defined(SKINNED)
struct VertexIn
{
	float3 posL    : POSITION;
	float3 boneWeights : WEIGHTS;
	uint4 boneIndices  : BONEINDICES;
};
struct VertexOut
{
	float4 posW    : POSITION;
};
#endif
struct GsOut
{
	float4 posH		: SV_POSITION;
	uint rTIndex	: SV_RenderTargetArrayIndex;
};
#if defined(STATIC)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	vout.posW = mul(float4(vin.posL, 1.0f), objWorld);
	return vout;
}
#elif defined(SKINNED)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.boneWeights.x;
	weights[1] = vin.boneWeights.y;
	weights[2] = vin.boneWeights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; ++i)
		posL += weights[i] * mul(float4(vin.posL, 1.0f), objBoneTransforms[vin.boneIndices[i]]).xyz;

	vout.posW = mul(float4(vin.posL, 1.0f), objWorld);
	return vout;
}
#endif
[maxvertexcount(CUBE_MAP_FACE * 3)]
void GS(triangle VertexOut input[3], inout TriangleStream<GsOut> output)
{
	for (int i = 0; i < CUBE_MAP_FACE; ++i)
	{
		GsOut gsOut;
		gsOut.rTIndex = i;
		for (uint j = 0; j < 3; j++)
		{
			gsOut.posH = mul(input[j].posW, pointLightShadowT[i]);
			output.Append(gsOut);
		}
		output.RestartStrip();
	}
} 
#endif

#if defined(NORMAL)
#if defined(STATIC)
struct VertexIn
{
	float3 posL    : POSITION;
};
struct VertexOut
{
	float4 posH    : SV_POSITION;
};
#elif defined(SKINNED)
struct VertexIn
{
	float3 posL    : POSITION;
	float3 boneWeights : WEIGHTS;
	uint4 boneIndices  : BONEINDICES;
};
struct VertexOut
{
	float4 posH    : SV_POSITION;
};
#endif
#if defined(STATIC)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	float4 posW = mul(float4(vin.posL, 1.0f), objWorld);
	vout.posH = mul(posW, shadowT);
	return vout;
}
#elif defined(SKINNED)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.boneWeights.x;
	weights[1] = vin.boneWeights.y;
	weights[2] = vin.boneWeights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; ++i)
		posL += weights[i] * mul(float4(vin.posL, 1.0f), objBoneTransforms[vin.boneIndices[i]]).xyz;

	float4 posW = mul(float4(vin.posL, 1.0f), objWorld);
	vout.posH = mul(posW, shadowT);
	return vout;
}
#endif
#endif