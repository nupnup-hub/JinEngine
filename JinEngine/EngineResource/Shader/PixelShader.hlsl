#include "VertexLayout.hlsl"
#include "Common.hlsl" 

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
	cubeMap[albedoTexIndex].Sample(samLinearWrap, pin.PosL);
	return cubeMap[albedoTexIndex].Sample(samLinearWrap, pin.PosL);
	//MaterialData matData = materialData[objMaterialIndex];
	//uint albedoTexIndex = matData.albedoMapIndex;
	//return cubeMap[albedoTexIndex].Sample(samLinearWrap, pin.PosL);
}
#elif defined(ALBEDO_MAP_ONLY)
float4 PS(VertexOut pin) : SV_Target
{
	MaterialData matData = materialData[objMaterialIndex];
	uint albedoTexIndex = matData.albedoMapIndex;
	return textureMaps[albedoTexIndex].Sample(samAnisotropicWrap, pin.TexC);
}
#elif defined(WRITE_SHADOW_MAP)
void PS(VertexOut pin)
{ 
/*
	// Fetch the material data.
	MaterialData matData = materialData[objMaterialIndex];
	float4 albedoColor = matData.albedoColor;
	uint albedoMapIndex = matData.albedoMapIndex;

#ifdef ALBEDO_MAP
	// Dynamically look up the texture in the array.
	albedoColor *= textureMaps[albedoMapIndex].Sample(samAnisotropicWrap, pin.TexC);
#endif

#ifdef ALPHA_CLIP
	// Discard pixel if texture alpha < 0.1.  We do this test as soon
	// as possible in the shader so that we can potentially exit the
	// shader early, thereby skipping the rest of the shader code.
	clip(albedoColor.a - 0.1f);
#endif
*/
}
#elif defined(BOUNDING_OBJECT_DEPTH_TEST)
void PS(VertexOut pin)
{}
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
	albedoColor *= textureMaps[albedoTexIndex].Sample(samAnisotropicWrap, pin.TexC);
#endif

	float3 normalW = normalize(pin.NormalW);
#ifdef NORMAL_MAP
	float4 normalMapSample = textureMaps[normalMapIndex].Sample(samAnisotropicWrap, pin.TexC);
	normalW = NormalSampleToWorldSpace(normalMapSample.rgb, normalW, pin.TangentW);
#endif

	float3 toEyeW = normalize(camEyePosW - pin.PosW);
	float4 ambient = sceneAmbientLight * albedoColor;

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
	float4 directLight = float4(0, 0, 0, 0);

#ifdef LIGHT
	for (int i = litStIndex; i < litEdIndex; ++i)
		directLight += ComputeLighting(light[i], mat, pin.PosW, normalW, toEyeW, 1.0f);
#endif

#ifdef SHADOW
	float shadowFactor = 0;
	int shadowIndex = 0;
	for (int i = shadwMapStIndex; i < shadowMapEdIndex; ++i)
	{
		float4 shadowPosH = mul(float4(pin.PosW, 1.0f), smLight[i].shadowTransform);
		shadowFactor = CalcShadowFactor(shadowPosH, smLight[i].shadowMapIndex);
		directLight += ComputeSLighting(smLight[i], mat, pin.PosW, normalW, toEyeW, shadowFactor);
	}
#endif

	float4 litColor = albedoColor + directLight;
	//float4 litColor = albedoColor;

#ifdef NORMAL_MAP
	float3 reflectLight = reflect(-toEyeW, normalW);
	float4 reflectionColor = cubeMap.Sample(samLinearWrap, reflectLight);
	float3 fresnelFactor = SchlickFresnel(fresnelR0, normalW, reflectLight);
	litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
#else
	float3 reflectLight = reflect(-toEyeW, normalW);
	float3 fresnelFactor = SchlickFresnel(fresnelR0, normalW, reflectLight);
	litColor.rgb += shininess * fresnelFactor;
#endif

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
 
