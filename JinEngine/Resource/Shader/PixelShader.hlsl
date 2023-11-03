#include "VertexLayout.hlsl"
#include "ShadowMapCal.hlsl"
 
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
	texC = saturate(ApplyParallaxOffset(texC, tangentSpaceViewDir, matData.normalMapIndex, matData.heightMapIndex)); 
#endif
	float4 normalMapSample = textureMaps[matData.normalMapIndex].Sample(samAnisotropicWrap, texC);
	// Uncompress each component from [0,1] to [-1,1].
    float3 normalT = 2.0f * normalMapSample.rgb - 1.0f;
	normalW =  normalize(mul(normalT, TBN));
#endif
	//float4 ambient = sceneAmbientLight * albedoColor;

#ifdef ROUGHNESS_MAP
#endif

#ifdef NORMAL_MAP
	float roughness = matData.roughness * normalMapSample.a;
#else
	float roughness = matData.roughness;
#endif
	 
	Material mat = { albedoColor, matData.metallic, roughness };
	float4 directLight = CalculateLight(mat, pin.posW, normalW, pin.tangentW, toEyeW);
	//float3 reflectLight = reflect(-toEyeW, normalW);
	//float4 reflectionColor = cubeMap[0].Sample(samLinearWrap, reflectLight);
	//float3 fresnelFactor = SchlickFresnel(reflectionColor.xyz, normalW, reflectLight, matData.metallic);
	float4 litColor = directLight;
	//litColor.rgb += roughness * fresnelFactor;
	litColor.a = albedoColor.a;
#ifdef AMBIENT_OCCLUSION_MAP
	float ambientFactor = textureMaps[matData.ambientMapIndex].Sample(samAnisotropicWrap, texC).x;
	return litColor * ambientFactor;
#else
	return litColor;
#endif 
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

