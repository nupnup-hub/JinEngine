#include "VertexLayout.hlsl"
#include "VertexCommon.hlsl" 

#if defined(DEBUG)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	float4 posW = mul(float4(vin.posL, 1.0f), objWorld);
	vout.posW = posW.xyz;
	vout.posH = mul(posW, camViewProj);
	return vout;
}

#elif defined(SKY)
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.posL = vin.posL;
	float4 posW = mul(float4(vin.posL, 1.0f), objWorld);
	posW.xyz += camEyePosW;
	vout.posH = mul(posW, camViewProj).xyww;
	return vout;
}

#elif defined(ALBEDO_MAP_ONLY)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	float4 posW = mul(float4(vin.posL, 1.0f), objWorld);
	vout.posH = mul(posW, camViewProj);

	MaterialData matData = materialData[objMaterialIndex];
	float4 texC = mul(float4(vin.texC, 0.0f, 1.0f), objTexTransform);
	vout.texC = mul(texC, matData.matTransform).xy;

	return vout;
}

#elif defined(STATIC)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	MaterialData matData = materialData[objMaterialIndex];

	float4 posW = mul(float4(vin.posL, 1.0f), objWorld);
	vout.posW = posW.xyz;
	vout.normalW = mul(vin.normalL, (float3x3)objWorld);
	vout.tangentW = mul(vin.tangentL, (float3x3)objWorld);
	vout.posH = mul(posW, camViewProj);
	float4 texC = mul(float4(vin.texC, 0.0f, 1.0f), objTexTransform);
	vout.texC = mul(texC, matData.matTransform).xy;
	return vout;
}
#elif defined(SKINNED)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	MaterialData matData = materialData[objMaterialIndex];
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.boneWeights.x;
	weights[1] = vin.boneWeights.y;
	weights[2] = vin.boneWeights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; ++i)
	{
		posL += weights[i] * mul(float4(vin.posL, 1.0f), objBoneTransforms[vin.boneIndices[i]]).xyz;
		normalL += weights[i] * mul(vin.normalL, (float3x3)objBoneTransforms[vin.boneIndices[i]]);
		tangentL += weights[i] * mul(vin.tangentL.xyz, (float3x3)objBoneTransforms[vin.boneIndices[i]]);
	}

	vin.posL = posL;
	vin.normalL = normalL;
	vin.tangentL.xyz = tangentL;
	float4 posW = mul(float4(vin.posL, 1.0f), objWorld);
	vout.posW = posW.xyz;
	vout.normalW = mul(vin.normalL, (float3x3)objWorld);
	vout.tangentW = mul(vin.tangentL, (float3x3)objWorld);
	vout.posH = mul(posW, camViewProj);
	float4 texC = mul(float4(vin.texC, 0.0f, 1.0f), objTexTransform);
	vout.texC = mul(texC, matData.matTransform).xy;
	return vout;
}
#else
#endif
 

