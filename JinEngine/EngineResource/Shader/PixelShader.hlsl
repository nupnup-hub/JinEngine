#include "VertexLayout.hlsl"
#include "Common.hlsl" 

#if defined(DEBUG)
float4 PS(VertexOut pin) : SV_Target
{ 
	MaterialData matData = materialData[objMaterialIndex]; 
	return matData.diffuseAlbedo;
}
#elif defined(SKY)
float4 PS(VertexOut pin) : SV_Target
{ 
	return cubeMap.Sample(samLinearWrap, pin.PosL);
}
#elif defined(ALBEDO_MAP_ONLY)
float4 PS(VertexOut pin) : SV_Target
{
	MaterialData matData = materialData[objMaterialIndex];
	uint diffuseTexIndex = matData.diffuseMapIndex;
	return textureMaps[diffuseTexIndex].Sample(samAnisotropicWrap, pin.TexC);
}
#elif defined(WRITE_SHADOW_MAP)
void PS(VertexOut pin)
{ 
/*
	// Fetch the material data.
	MaterialData matData = materialData[objMaterialIndex];
	float4 diffuseAlbedo = matData.diffuseAlbedo;
	uint diffuseMapIndex = matData.diffuseMapIndex;

#ifdef ALBEDO_MAP
	// Dynamically look up the texture in the array.
	diffuseAlbedo *= textureMaps[diffuseMapIndex].Sample(samAnisotropicWrap, pin.TexC);
#endif

#ifdef ALPHA_CLIP
	// Discard pixel if texture alpha < 0.1.  We do this test as soon
	// as possible in the shader so that we can potentially exit the
	// shader early, thereby skipping the rest of the shader code.
	clip(diffuseAlbedo.a - 0.1f);
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
	float4 diffuseAlbedo = matData.diffuseAlbedo;
	float metalic = matData.metallic;
	float3 fresnelR0 = float3(metalic, metalic, metalic);
	float roughness = matData.roughness;
	uint diffuseTexIndex = matData.diffuseMapIndex;
	uint normalMapIndex = matData.normalMapIndex;
	uint roughnessMapIndex = matData.roughnessMapIndex;
	//float roughnessTex = textureMaps[roughnessMapIndex].Sample(gsamAnisotropicClamp, pin.TexC).r;
#ifdef ALBEDO_MAP
	diffuseAlbedo *= textureMaps[diffuseTexIndex].Sample(samAnisotropicWrap, pin.TexC);
#endif

	float3 normalW = normalize(pin.NormalW);
#ifdef NORMAL_MAP
	float4 normalMapSample = textureMaps[normalMapIndex].Sample(samAnisotropicWrap, pin.TexC);
	normalW = NormalSampleToWorldSpace(normalMapSample.rgb, normalW, pin.TangentW);
#endif

	float3 toEyeW = normalize(camEyePosW - pin.PosW);
	float4 ambient = sceneAmbientLight * diffuseAlbedo;

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

#ifdef SHADOW
	float shadowFactor[MaxLights * 3];
	int shadowIndex = 0;
	for (int i = 0; i < smDirectionalLightMax; ++i)
	{
		float4 shadowPosH = mul(float4(pin.PosW, 1.0f), smDirectionalLight[i].shadow.shadowTransform);
		shadowFactor[shadowIndex] = CalcShadowFactor(shadowPosH, smDirectionalLight[i].shadow.shadowMapIndex);
		++shadowIndex;
	}
	for (int i = 0; i < smPointLightMax; ++i)
	{
		float4 shadowPosH = mul(float4(pin.PosW, 1.0f), smPointLight[i].shadow.shadowTransform);
		shadowFactor[shadowIndex] = CalcShadowFactor(shadowPosH, smPointLight[i].shadow.shadowMapIndex);
		++shadowIndex;
	}
	for (int i = 0; i < smSpotLightMax; ++i)
	{
		float4 shadowPosH = mul(float4(pin.PosW, 1.0f), smSpotLight[i].shadow.shadowTransform);
		shadowFactor[shadowIndex] = CalcShadowFactor(shadowPosH, smSpotLight[i].shadow.shadowMapIndex);
		++shadowIndex;
	}
#endif

	Material mat = { diffuseAlbedo, fresnelR0, shininess };

#ifdef LIGHT
	float4 directLight = ComputeLighting(directionalLight, pointLight, spotLight,
		directionalLightMax, pointLightMax, spotLightMax,
		mat, pin.PosW,
		normalW, toEyeW, 1.0f);
#else
	float4 directLight = float4(0, 0, 0, 0);
#endif

#ifdef SHADOW
	directLight += ComputeSLighting(smDirectionalLight, smPointLight, smSpotLight,
		smDirectionalLightMax, smPointLightMax, smSpotLightMax,
		mat, pin.PosW, 
		normalW, toEyeW, shadowFactor);
#endif

	float4 litColor = diffuseAlbedo + directLight;
	//float4 litColor = diffuseAlbedo;

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

	litColor.a = diffuseAlbedo.a;
	return litColor; 
}
#endif
 



/*#ifdef SHADOW_MAP 
void PS(VertexOut pin)
{
	// Fetch the material data.
	MaterialData matData = materialData[objMaterialIndex];
	float4 diffuseAlbedo = matData.diffuseAlbedo;
	uint diffuseMapIndex = matData.diffuseMapIndex;

	// Dynamically look up the texture in the array.
	diffuseAlbedo *= textureMaps[diffuseMapIndex].Sample(samAnisotropicWrap, pin.TexC);

#ifdef ALPHA_CLIP
	// Discard pixel if texture alpha < 0.1.  We do this test as soon 
	// as possible in the shader so that we can potentially exit the
	// shader early, thereby skipping the rest of the shader code.
	clip(diffuseAlbedo.a - 0.1f);
#endif
}
#endif*/
 
