#include "VertexLayout.hlsl"
#include "ShadowMapCal.hlsl"

#ifndef JINENGINE_COMMON
#include "Common.hlsl" 
#endif

#if defined(DEBUG)
float4 PS(VertexOut pin) : SV_Target
{
	MaterialData matData = materialData[objMaterialIndex];
	return matData.albedoColor;
}
#elif defined(SKY)
float4 PS(VertexOut pin) : SV_Target
{
	MaterialData matData = materialData[objMaterialIndex];
	uint albedoTexIndex = matData.albedoMapIndex; 
	return cubeMap[albedoTexIndex].Sample(samLinearWrap, pin.posL);
	//MaterialData matData = materialData[objMaterialIndex];
	//uint albedoTexIndex = matData.albedoMapIndex;
	//return cubeMap[albedoTexIndex].Sample(samLinearWrap, pin.PosL);
}
#elif defined(ALBEDO_MAP_ONLY)
float4 PS(VertexOut pin) : SV_Target
{
	MaterialData matData = materialData[objMaterialIndex];
	uint albedoTexIndex = matData.albedoMapIndex;
	return textureMaps[albedoTexIndex].Sample(samAnisotropicWrap, pin.texC);
}
#else
float4 PS(VertexOut pin) : SV_Target
{
	MaterialData matData = materialData[objMaterialIndex];
	float4 albedoColor = matData.albedoColor;
	float metalic = matData.metallic;
	float3 fresnelR0 = float3(metalic, metalic, metalic);
	float roughness = matData.roughness;
	uint albedoTexIndex = matData.albedoMapIndex;
	uint normalMapIndex = matData.normalMapIndex;
	uint roughnessMapIndex = matData.roughnessMapIndex;
	//float roughnessTex = textureMaps[roughnessMapIndex].Sample(gsamAnisotropicClamp, pin.TexC).r;
#ifdef ALBEDO_MAP
	albedoColor *= textureMaps[albedoTexIndex].Sample(samAnisotropicWrap, pin.texC);
#endif

	float3 normalW = normalize(pin.normalW);
#ifdef NORMAL_MAP
	float4 normalMapSample = textureMaps[normalMapIndex].Sample(samAnisotropicWrap, pin.texC);
	normalW = NormalSampleToWorldSpace(normalMapSample.rgb, normalW, pin.tangentW);
#endif
	 
	float3 toEyeW = normalize(camEyePosW - pin.posW);
	//float4 ambient = sceneAmbientLight * albedoColor;

#ifdef HEIGHT_MAP
#endif

#ifdef ROUGHNESS_MAP
#endif

#ifdef AMBIENT_OCCLUSION_MAP
#endif 

#ifdef NORMAL_MAP
	float shininess = (1.001f - roughness) * normalMapSample.a;
#else
	float shininess = (1.001f - roughness);
#endif

	Material mat = { albedoColor, fresnelR0, shininess };
	float4 directLight = CalculateLight(mat, pin.posW, normalW, toEyeW);
	float3 reflectLight = reflect(-toEyeW, normalW);
	//float4 reflectionColor = cubeMap[0].Sample(samLinearWrap, reflectLight);
	float3 fresnelFactor = SchlickFresnel(fresnelR0, normalW, reflectLight);
	float4 litColor = albedoColor + directLight;
	litColor.rgb += shininess * fresnelFactor;

	litColor.a = albedoColor.a; 
	return litColor;
}
#endif




/*#ifdef SHADOW_MAP
void PS(VertexOut pin)
{
	// Fetch the material data.
	MaterialData matData = materialData[objMaterialIndex];
	float4 albedoColor = matData.albedoColor;
	uint albedoMapIndex = matData.albedoMapIndex;

	// Dynamically look up the texture in the array.
	albedoColor *= textureMaps[albedoMapIndex].Sample(samAnisotropicWrap, pin.TexC);

#ifdef ALPHA_CLIP
	// Discard pixel if texture alpha < 0.1.  We do this test as soon
	// as possible in the shader so that we can potentially exit the
	// shader early, thereby skipping the rest of the shader code.
	clip(albedoColor.a - 0.1f);
#endif
}
#endif*/

