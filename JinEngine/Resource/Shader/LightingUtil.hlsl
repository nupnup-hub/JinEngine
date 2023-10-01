//***************************************************************************************
// LightingUtil.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Contains API for shader lighting.
//***************************************************************************************
 
#if !defined(CSM_MAX_COUNT)
#define CSM_MAX_COUNT 8
#endif
 
#define DIRECTONAL_LIGHT_NON_SHADOW_MAP -1
#define DIRECTONAL_LIGHT_HAS_NORMAL_SHADOW_MAP 0
#define DIRECTONAL_LIGHT_HAS_CSM 1

struct DirectionalLightData
{ 
	float4x4 view;
	float4x4 viewProj;
    float4x4 shadowMapTransform;    //if cascade shadow this matrix transform camera viewspace else transform textureSpace
    float3 color; 
    int shadowMapIndex;
    float3 direction;  
    int shadowMapType;  //-1 not, 0 normal, 1 csm, 2 cube
	float2 frustumSize;
	float frustumNear;
	float frustumFar;
	float punumbraScale;
	float penumbraNearPlane;
    int csmDataIndex;  //data start index
    uint lightPad00; 
};

struct CsmData
{ 
	float4 scale[CSM_MAX_COUNT]; // (projM * textureM) scale
	float4 posOffset[CSM_MAX_COUNT]; // (projM * textureM) translate
	float2 frustumSize[CSM_MAX_COUNT];
	float frustumNear[CSM_MAX_COUNT];
	float frustumFar[CSM_MAX_COUNT];
    float mapMinBorder;     //default value is 0
    float mapMaxBorder;     //default value is 1
	float levelBlendRate;
    uint count; 
};
struct PointLightData
{ 
    float3 color;
    float power; 
    float3 position;
    float range;
    float radius;         
	float shadowMapIndex;
	int hasShadowMap;
	uint pLightPad00;
};

struct SpotLightData
{
    float4x4 shadowMapTransform;
    float3 color;
	float power;
    float3 position;
	float range;
    float3 direction;
	float innerConeAngle;
	float outerConeAngle;  
    int shadowMapIndex;
    uint hasShadowMap;     
	uint sLightPad00;
};
struct Material
{
    float4 albedoColor;
    float3 fresnelR0;
    float shininess;
};

float CalLinearAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff.
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}
float CalInverseAttenuation(float d, float range)
{
	return saturate(1 - pow(d / range, 2.0f));
}
float CalFactorAttenuation(float d, float constantsF, float linearF, float quadraticF)
{
	return saturate(1.0f / (constantsF + linearF * d + quadraticF * (d * d)));
}

// Schlick gives an approximation to Fresnel reflectance (see pg. 233 "Real-Time Rendering 3rd Ed.").
// R0 = ( (n-1)/(n+1) )^2, where n is the index of refraction.
float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    float cosIncidentAngle = saturate(dot(normal, lightVec));

    float f0 = 1.0f - cosIncidentAngle;
    float3 reflectPercent = R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);

    return reflectPercent;
}
float3 BlinnPhong(float3 lightColor, float3 lightVec, float3 normal, float3 toEye, Material mat)
{
    const float m = mat.shininess * 256.0f;
    float3 halfVec = normalize(toEye + lightVec);

    float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    float3 fresnelFactor = SchlickFresnel(mat.fresnelR0, halfVec, lightVec);

    float3 specAlbedo = fresnelFactor * roughnessFactor;

    // Our spec formula goes outside [0,1] range, but we are 
    // doing LDR rendering.  So scale it down a bit.
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (mat.albedoColor.rgb + specAlbedo) * lightColor;
} 
float3 ComputeDirectionalLight(DirectionalLightData light, Material mat, float3 normal, float3 toEye)
{
    // The light vector aims opposite the direction the light rays travel.
    float3 lightVec = -light.direction;

    // Scale light down by Lambert's cosine law.
    float nDotL = max(dot(lightVec, normal), 0.0f);
    float3 lightColor = light.color * nDotL;

    return BlinnPhong(lightColor, lightVec, normal, toEye, mat);
}  
float3 ComputePointLight(PointLightData light, Material mat, float3 pos, float3 normal, float3 toEye)
{
    // The vector from the surface to the light.
    float3 lightVec = light.position - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test.
    if (d > light.range)
        return 0.0f;

    // Normalize the light vector.
    lightVec /= d;

    // Scale light down by Lambert's cosine law.
    float nDotL = max(dot(lightVec, normal), 0.0f);
    float3 lightColor = light.color * nDotL;

    // Attenuate light by distance.
	float attFactor = CalInverseAttenuation(d, light.range);
	lightColor *= attFactor;

    return BlinnPhong(lightColor, lightVec, normal, toEye, mat);
}
//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for spot lights.
//---------------------------------------------------------------------------------------
float3 ComputeSpotLight(SpotLightData light, Material mat, float3 pos, float3 normal, float3 toEye)
{
    // The vector from the surface to the light.
    float3 lightVec = light.position - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test.
    if (d > light.range)
        return 0.0f;

    // Normalize the light vector.
    lightVec /= d;

    // Scale light down by Lambert's cosine law.
    float nDotL = max(dot(lightVec, normal), 0.0f);
	float dDotL = dot(-lightVec, light.direction);
    
    float3 lightColor = light.color * nDotL;
      
	float attFactor = CalInverseAttenuation(d, light.range); 
	float theta = dDotL;
	float epsilon = light.innerConeAngle - light.outerConeAngle;
	float smoothFactor = clamp((theta - light.outerConeAngle) / epsilon, 0.0, 1.0);
    
	lightColor *= light.power * attFactor * (smoothFactor * smoothFactor);
    return BlinnPhong(lightColor, lightVec, normal, toEye, mat);
}


