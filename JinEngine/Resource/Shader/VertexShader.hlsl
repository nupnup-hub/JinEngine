#include "VertexLayout.hlsl"
#include "VertexShaderCommon.hlsl" 
 
#if defined(FULL_SCREEN_QUAD)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut) 0.0f;	 
	//Practical Rendering & Computation dx11 chapter 11
	//float3 posV = mul(float4(vin.posL, 1.0f), camInvProj).xyz;
	//vout.dir = float3(posV.xy / posV.z, 1.0f);
	vout.texC = vin.texC;
	vout.posH = float4(vin.posL, 1.0f); 
	return vout;
}
#elif defined(FULL_SCREEN_TRIANGLE)
VertexOut VS(uint VertexId : SV_VertexID)
{
	VertexOut vout = (VertexOut) 0.0f;
	vout.texC = float2((VertexId << 1) & 2, VertexId & 2);
	vout.posH = float4(vout.texC * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	//float3 posV = mul(vout.posH, camInvProj).xyz;
	//vout.dir = float3(posV.xy / posV.z, 1.0f);
	return vout;
} 
#elif defined(DEBUG)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	float4 posW = mul(float4(vin.posL, 1.0f), cbObject.world);
	vout.posW = posW.xyz;
	vout.posH = mul(posW, cbCam.viewProj);
	return vout;
}
#elif defined(SKY)
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.posL = vin.posL;
	float4 posW = mul(float4(vin.posL, 1.0f), cbObject.world);
	posW.xyz += cbCam.eyePosW;
	vout.posH = mul(posW, cbCam.viewProj).xyww;
	return vout;
}
#elif defined(ALBEDO_MAP_ONLY)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	float4 posW = mul(float4(vin.posL, 1.0f), cbObject.world);
	vout.posH = mul(posW, cbCam.viewProj);

	MaterialData matData = materialData[cbObject.materialIndex];
	float4 texC = mul(float4(vin.texC, 0.0f, 1.0f), cbObject.texTransform);
	vout.texC = mul(texC, matData.matTransform).xy;

	return vout;
} 
#elif defined(STATIC)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	MaterialData matData = materialData[cbObject.materialIndex];

	//hlsl은 column major를 사용하며
	//d3d에 경우 shader에 보낼 matrix를 transpose하면 raw major형식으로 계산가능.
	
	float4 posW = mul(float4(vin.posL, 1.0f), cbObject.world);
	vout.posW = posW.xyz;
	vout.normalW = mul(vin.normalL, (float3x3)cbObject.world);
	vout.tangentW = mul(vin.tangentL, (float3x3)cbObject.world);
	vout.posH = mul(posW, cbCam.viewProj);
	float4 texC = mul(float4(vin.texC, 0.0f, 1.0f), cbObject.texTransform);
	vout.texC = mul(texC, matData.matTransform).xy;
	return vout;
}
#elif defined(SKINNED)
VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	MaterialData matData = materialData[cbObject.materialIndex];
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
		posL += weights[i] * mul(float4(vin.posL, 1.0f), cbSkinned.boneTransforms[vin.boneIndices[i]]).xyz;
		normalL += weights[i] * mul(vin.normalL, (float3x3)cbSkinned.boneTransforms[vin.boneIndices[i]]);
		tangentL += weights[i] * mul(vin.tangentL.xyz, (float3x3)cbSkinned.boneTransforms[vin.boneIndices[i]]);
	}

	vin.posL = posL;
	vin.normalL = normalL;
	vin.tangentL.xyz = tangentL;
	float4 posW = mul(float4(vin.posL, 1.0f), cbObject.world);
	vout.posW = posW.xyz;
	vout.normalW = mul(vin.normalL, (float3x3)cbObject.world);
	vout.tangentW = mul(vin.tangentL, (float3x3)cbObject.world);
	vout.posH = mul(posW, cbCam.viewProj);
	float4 texC = mul(float4(vin.texC, 0.0f, 1.0f), cbObject.texTransform);
	vout.texC = mul(texC, matData.matTransform).xy;
	return vout;
}
#else
#endif
 

