#include "VertexLayout.hlsl"
#include "Common.hlsl" 
 
float4 CalculateLight(Material mat, 
	float3 posW,
	float3 normal,
	float3 toEye)
{
	float3 directLight = float3(0, 0, 0);
#ifdef SHADOW
	for (int i = directionalLitSt; i < directionalLitEd; ++i)
	{
		float shadowFactor = 1;
		float4 shadowPosH = mul(float4(posW, 1.0f), directionalLight[i].shadowMapTransform);
		if (directionalLight[i].shadowMapType == DIRECTONAL_LIGHT_HAS_NORMAL_SHADOW_MAP)
			shadowFactor = CalShadowFactor(shadowPosH, directionalLight[i].shadowMapIndex);
		else if (directionalLight[i].shadowMapType == DIRECTONAL_LIGHT_HAS_CSM)
			shadowFactor = CalCascadeShadowFactor(shadowPosH, directionalLight[i].shadowMapIndex, directionalLight[i].csmDataIndex + csmLocalIndex);

		directLight += ComputeDirectionalLight(directionalLight[i], mat, normal, toEye) * shadowFactor;
	}
	for (int i = pointLitSt; i < pointLitEd; ++i)
	{
		float shadowFactor = CalCubeShadowFactor(posW, i, pointLight[i].shadowMapIndex);
		directLight += ComputePointLight(pointLight[i], mat, posW, normal, toEye) * shadowFactor;
	}
	for (int i = spotLitSt; i < spotLitEd; ++i)
	{
		float4 shadowPosH = mul(float4(posW, 1.0f), spotLight[i].shadowMapTransform);
		float shadowFactor = CalShadowFactor(shadowPosH, spotLight[i].shadowMapIndex);
		directLight += ComputeSpotLight(spotLight[i], mat, posW, normal, toEye) * shadowFactor;
	}
#else 
	for (int i = directionalLitSt; i < directionalLitEd; ++i)
		directLight += ComputeDirectionalLight(directionalLight[i], mat, normal, toEye);
	for (int i = pointLitSt; i < pointLitEd; ++i)
		directLight += ComputePointLight(pointLight[i], mat, posW, normal, toEye);
	for (int i = spotLitSt; i < spotLitEd; ++i)
		directLight += ComputeSpotLight(spotLight[i], mat, posW, normal, toEye);
#endif
	return float4(directLight, 0.0f);
}


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

