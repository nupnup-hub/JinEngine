#include "DepthFunc.hlsl"

#define ARRAY_MAX_COUNT 8
#if !defined(ARRAY_COUNT)
#define ARRAY_COUNT ARRAY_MAX_COUNT
#endif
 
#define CUBE_MAP_FACE 6
  
#if defined(USE_VSM)
#define STORE_VARIANCE 1
#else
#define STORE_DEPTH_ONLY 1
#endif

//64
cbuffer cbObject : register(b0)
{
	float4x4 objWorld;
	//float4x4 objTexTransform;
	//uint objMaterialIndex;
	//uint objPad00;
	//uint objPad01;
	//uint objPad02;
};
//16384
cbuffer cbSkinned : register(b1)
{
	float4x4 objBoneTransforms[256];
};
//spot light and directional light(off csm)
//64
cbuffer cbShadowMap : register(b2)
{
	float4x4 shadowT; 
};
//512
cbuffer cbShadowMapArray : register(b3)
{
	float4x4 shadowArrayT[ARRAY_MAX_COUNT]; 
};
//384
cbuffer cbShadowMapCube : register(b4)
{
	float4x4 shadowCubeT[CUBE_MAP_FACE]; 
};


//Vsm 
float2 ComputeMoments(float depth)
{
    // Compute first few moments of depth
	float2 momments;
	momments.x = depth;
	momments.y = depth * depth;
    
    // Ajust the variance distribution to include the whole pixel if requested
    // NOTE: Disabled right now as a min variance clamp takes care of all problems
    // and doesn't risk screwy hardware derivatives.
	
	//float dx = ddx(depth);
	//float dy = ddy(depth); 
	//momments.y = depth * depth + 0.25f * (dx * dx + dy * dy);
    // Perhaps clamp maximum Delta here
	return momments;
}

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
	float3 normalL : NORMAL;
	float2 texC    : TEXCOORD;
	float3 tangentL : TANGENT;
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
	uint rtIndex	: SV_RenderTargetArrayIndex;
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

	vout.posW = mul(float4(posL, 1.0f), objWorld);
	return vout;
}
#endif
[maxvertexcount(ARRAY_COUNT * 3)]
void GS(triangle VertexOut input[3], inout TriangleStream<GsOut> output)
{
	[unroll]
	for (int i = 0; i < ARRAY_COUNT; ++i)
	{
		GsOut gsOut;
		gsOut.rtIndex = i;
		for (int j = 0; j < 3; j++)
		{
			//can add bias
			gsOut.posH = mul(input[j].posW, shadowArrayT[i]);
			output.Append(gsOut);
		}
		output.RestartStrip();
	}
}
#ifdef STORE_VARIANCE
float2 PS(GsOut pin) : SV_Target
{     
	return ComputeMoments(pin.posH.z / pin.posH.w);
}
#endif

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
	float3 normalL : NORMAL;
	float2 texC    : TEXCOORD;
	float3 tangentL : TANGENT;
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
	uint rtIndex	: SV_RenderTargetArrayIndex;
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

	vout.posW = mul(float4(posL, 1.0f), objWorld);
	return vout;
}
#endif
[maxvertexcount(CUBE_MAP_FACE * 3)]
void GS(triangle VertexOut input[3], inout TriangleStream<GsOut> output)
{
	[unroll]
	for (int i = 0; i < CUBE_MAP_FACE; ++i)
	{
		GsOut gsOut;
		gsOut.rtIndex = i;
		for (uint j = 0; j < 3; j++)
		{
			gsOut.posH = mul(input[j].posW, shadowCubeT[i]);
			output.Append(gsOut);
		}
		output.RestartStrip();
	}
} 
#ifdef STORE_VARIANCE
float2 PS(GsOut pin) : SV_Target
{     
	return ComputeMoments(pin.posH.z / pin.posH.w);
}
#endif

#endif
 
#if defined(NORMALSM)
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
	float3 normalL : NORMAL;
	float2 texC    : TEXCOORD;
	float3 tangentL : TANGENT;
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

	float4 posW = mul(float4(posL, 1.0f), objWorld);
	vout.posH = mul(posW, shadowT);
	return vout;
}
#endif
#ifdef STORE_VARIANCE
float2 PS(VertexOut pin) : SV_Target
{     
	return ComputeMoments(pin.posH.z / pin.posH.w);
}
#endif
#endif