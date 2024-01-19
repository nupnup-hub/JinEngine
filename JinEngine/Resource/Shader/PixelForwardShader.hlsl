#include "PixelLayout.hlsl"
#include "ShadowCompute.hlsl"

typedef float4 PixelOut;
#if defined(DEBUG)
PixelOut PS(PixelIn pin) : SV_Target
{
	return materialData[objMaterialIndex].albedoColor;
}
#elif defined(SKY)
PixelOut PS(PixelIn pin) : SV_Target
{ 
	const uint albedoTexIndex =  materialData[objMaterialIndex].albedoMapIndex; 
	return materialData[objMaterialIndex].albedoColor * cubeMap[albedoTexIndex].Sample(samLinearWrap, pin.posL); 
}
#elif defined(ALBEDO_MAP_ONLY)
PixelOut PS(PixelIn pin) : SV_Target
{ 
	const uint albedoTexIndex = materialData[objMaterialIndex].albedoMapIndex;
	return materialData[objMaterialIndex].albedoColor * textureMaps[albedoTexIndex].Sample(samAnisotropicWrap, pin.texC);
}
#else
PixelOut PS(PixelIn pin) : SV_Target
{
	MaterialData matData = materialData[objMaterialIndex];
	float4 albedoColor = matData.albedoColor;
	//float roughness = textureMaps[matData.roughnessMapIndex].Sample(gsamAnisotropicClamp, pin.TexC).r;
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
	// Uncompress each component from [0,1] to [-1,1].
    float3 normalT = 2.0f * normalMapSample.rgb - 1.0f;
	normalW =  normalize(mul(normalT, TBN));
#endif
	//float4 ambient = sceneAmbientLight * albedoColor;
		 
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
	
	Material mat = { albedoColor, metallic, roughness };
#ifdef LIGHT_CLUSTER
	float3 directLight = ComputeLight(mat, pin.posW, normalW, pin.tangentW, toEyeW, pin.posH.xy, pin.posH.z);
	//float3 directLight = ComputeLight(mat, pin.posW, normalW, pin.tangentW, toEyeW, pin.posH.xy, depth);
#else
	float3 directLight = ComputeLight(mat, pin.posW, normalW, pin.tangentW, toEyeW);
#endif  
	//float3 reflectLight = reflect(-toEyeW, normalW);
	//float4 reflectionColor = cubeMap[0].Sample(samLinearWrap, reflectLight);
	//float3 fresnelFactor = SchlickFresnel(reflectionColor.xyz, normalW, reflectLight, matData.metallic);
	float4 litColor = float4(directLight, albedoColor.a);
	//litColor.rgb += roughness * fresnelFactor; 
//#ifdef USE_SSAO
//	normalMap[pin.posH.xy] = normalW.xy;
//#endif
	
#ifdef AMBIENT_OCCLUSION_MAP
	float ambientFactor = textureMaps[matData.ambientMapIndex].Sample(samLinearWrap, texC).x;
	return litColor * ambientFactor;
#else
	return litColor;
	//return float4(normalW, 1.0f);
#endif 
}
#endif