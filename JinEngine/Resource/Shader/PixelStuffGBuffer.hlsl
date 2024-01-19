#include "PixelLayout.hlsl" 
#include"PixelCommon.hlsl"

struct PixelOut
{
	float4 albedoColor : SV_TARGET0;
	float4 normal : SV_TARGET1;					//10,10,10, 2 rgb, empty
	float4 tangent : SV_TARGET2;				//10,10,10, 2 rgb, empty
	float4 lightingProp : SV_TARGET3;		//x metalic, y roughness, z ambient, w is emtpy
};
 
#if defined(DEBUG)
PixelOut PS(PixelIn pin) : SV_Target
{
	PixelOut pOut;
	pOut.albedoColor = materialData[objMaterialIndex].albedoColor;
	pOut.normal = float4(0, 0, 0, G_BUFFER_ALBEDO_ONLY_TYPE);
	return pOut;
}
#elif defined(SKY)
PixelOut PS(PixelIn pin) : SV_Target
{ 
	PixelOut pOut;
	pOut.albedoColor = materialData[objMaterialIndex].albedoColor;
	pOut.albedoColor *=	cubeMap[materialData[objMaterialIndex].albedoMapIndex].Sample(samLinearWrap, pin.posL);
	pOut.normal = float4(0, 0, 0, G_BUFFER_ALBEDO_ONLY_TYPE);
	return pOut; 
}
#elif defined(ALBEDO_MAP_ONLY)
PixelOut PS(PixelIn pin) : SV_Target
{ 
	PixelOut pOut;
	pOut.albedoColor = materialData[objMaterialIndex].albedoColor;
	pOut.albedoColor *= textureMaps[materialData[objMaterialIndex].albedoMapIndex].Sample(samAnisotropicWrap, pin.texC); 
	pOut.normal = float4(0, 0, 0, G_BUFFER_ALBEDO_ONLY_TYPE);
	return pOut;
}
#else
PixelOut PS(PixelIn pin) : SV_Target
{
	PixelOut pOut;
	MaterialData matData = materialData[objMaterialIndex];
	float4 albedoColor = matData.albedoColor; 
#ifdef ALBEDO_MAP
	albedoColor *= textureMaps[matData.albedoMapIndex].Sample(samAnisotropicWrap, pin.texC);
#endif
	float3 normalW = normalize(pin.normalW);
	float3 toEyeW = normalize(camEyePosW - pin.posW);
	float2 texC = pin.texC;
 
#ifdef NORMAL_MAP
	float3x3 TBN = CalTBN(normalW, pin.tangentW);
#ifdef HEIGHT_MAP
	float3 tangentSpaceViewDir = normalize(mul(TBN, toEyeW));
	texC = saturate(ApplyParallaxOffset(textureMaps[ matData.normalMapIndex], textureMaps[matData.heightMapIndex], texC, tangentSpaceViewDir)); 
#endif
	float4 normalMapSample = textureMaps[matData.normalMapIndex].Sample(samAnisotropicWrap, texC);
    float3 normalT = 2.0f * normalMapSample.rgb - 1.0f;
	normalW =  normalize(mul(normalT, TBN));
#endif 
		 
#ifdef METALLIC_MAP
	float metallic = textureMaps[matData.metallicMapIndex].Sample(samLinearWrap, texC).x;
#else
	float metallic = matData.metallic;
#endif
#ifdef ROUGHNESS_MAP
	float roughness = textureMaps[matData.roughnessMapIndex].Sample(samLinearWrap, texC).x;
#else
#ifdef NORMAL_MAP
	float roughness = matData.roughness * normalMapSample.a;
#else
	float roughness = matData.roughness;
#endif
#endif
 
	//deferred update후 추가필요.
#ifdef AMBIENT_OCCLUSION_MAP
	float ambientFactor = textureMaps[matData.ambientMapIndex].Sample(samLinearWrap, texC).x;	 
#else
	float ambientFactor = 1.0f;
#endif 
	pOut.albedoColor = albedoColor;
	pOut.normal = float4(EncodeOct(normalW), 0, G_BUFFER_STANDARD_TYPE);
	//pOut.normal = float4(SignedOctEncode(normalW), G_BUFFER_STANDARD_TYPE);
	pOut.tangent = float4(EncodeOct(normalize(pin.tangentW)),0,  0);
	pOut.lightingProp = float4(metallic, roughness, ambientFactor, 0.0f);
	 
	return pOut;
}
#endif