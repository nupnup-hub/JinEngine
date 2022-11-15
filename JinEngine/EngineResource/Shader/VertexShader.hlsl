#include "VertexLayout.hlsl"
#include "Common.hlsl" 

#if defined(DEBUG)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f; 
	float4 posW = mul(float4(vin.PosL, 1.0f), objWorld);
	vout.PosW = posW.xyz;
	vout.PosH = mul(posW, camViewProj); 
	return vout;
}

#elif defined(SKY)
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosL = vin.PosL;
	float4 posW = mul(float4(vin.PosL, 1.0f), objWorld);
	posW.xyz += camEyePosW;
	vout.PosH = mul(posW, camViewProj).xyww;
	return vout;
}

#elif defined(ALBEDO_MAP_ONLY)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	float4 posW = mul(float4(vin.PosL, 1.0f), objWorld);
	vout.PosH = mul(posW, camViewProj);

	MaterialData matData = materialData[objMaterialIndex];
	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), objTexTransform);
	vout.TexC = mul(texC, matData.matTransform).xy;

	return vout;
}

#elif defined(WRITE_SHADOW_MAP)
#if defined(STATIC)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;  
	float4 posW = mul(float4(vin.PosL, 1.0f), objWorld);
	vout.PosH = mul(posW, lightViewProj);
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
	vout.PosH = mul(posW, lightViewProj);

	return vout;
}
#endif

#elif defined(BOUNDING_OBJECT_DEPTH_TEST)
#if defined(STATIC)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	float4 posW = mul(float4(vin.PosL, 1.0f), boundObjWorld);
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
	float4 posW = mul(float4(vin.PosL, 1.0f), boundObjWorld);
	vout.PosH = mul(posW, camViewProj);

	return vout;
}
#endif

#elif defined(STATIC)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	MaterialData matData = materialData[objMaterialIndex];

	float4 posW = mul(float4(vin.PosL, 1.0f), objWorld);
	vout.PosW = posW.xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)objWorld);
	vout.TangentW = mul(vin.TangentL, (float3x3)objWorld);
	vout.PosH = mul(posW, camViewProj);
	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), objTexTransform);
	vout.TexC = mul(texC, matData.matTransform).xy;
	
	return vout;
}
#elif defined(SKINNED)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	MaterialData matData = materialData[objMaterialIndex];
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.BoneWeights.x;
	weights[1] = vin.BoneWeights.y;
	weights[2] = vin.BoneWeights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; ++i)
	{
		posL += weights[i] * mul(float4(vin.PosL, 1.0f), objBoneTransforms[vin.BoneIndices[i]]).xyz;
		normalL += weights[i] * mul(vin.NormalL, (float3x3)objBoneTransforms[vin.BoneIndices[i]]);
		tangentL += weights[i] * mul(vin.TangentL.xyz, (float3x3)objBoneTransforms[vin.BoneIndices[i]]);
	}

	vin.PosL = posL;
	vin.NormalL = normalL;
	vin.TangentL.xyz = tangentL;
	float4 posW = mul(float4(vin.PosL, 1.0f), objWorld);
	vout.PosW = posW.xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)objWorld);
	vout.TangentW = mul(vin.TangentL, (float3x3)objWorld);
	vout.PosH = mul(posW, camViewProj);
	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), objTexTransform);
	vout.TexC = mul(texC, matData.matTransform).xy;

	return vout;
}
#else
#endif


/*#ifdef SHADOW_MAP 
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

	MaterialData matData = materialData[objMaterialIndex];

	// Transform to world space.
	float4 posW = mul(float4(vin.PosL, 1.0f), objWorld);

	// Transform to homogeneous clip space.
	vout.PosH = mul(posW, camViewProj);

	// Output vertex attributes for interpolation across triangle.
	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), objTexTransform);
	vout.TexC = mul(texC, matData.matTransform).xy;

	return vout;
}
#endif
 */

