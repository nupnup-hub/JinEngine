//***************************************************************************************
// LightingUtil.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Contains API for shader lighting.
//***************************************************************************************

#define MaxLights 16

struct ShadowContents
{
    float4x4 shadowTransform;
    uint shadowMapIndex;
    uint shadowPad00;
    uint shadowPad01;
    uint shadowPad02;
};

struct DirectionalLightContents
{
    float3 strength;
    uint dLightPad00;
    float3 direction;
    uint dLightPad01;
};
struct SMDirectionalLightContents
{
    DirectionalLightContents dLight;
    ShadowContents shadow;
};
struct PointLightContents
{
    float3 strength;
    float falloffStart;
    float3 position;
    float falloffEnd;
};
struct SMPointLightContents
{
    PointLightContents pLight;
    ShadowContents shadow;
};
struct SpotLightContents
{
    float3 strength;
    float falloffStart;
    float3 direction;
    float falloffEnd;
    float3 position;
    float spotPower;
};
struct SMSpotLightContents
{
    SpotLightContents sLight;
    ShadowContents shadow;
};

struct Material
{
    float4 diffuseAlbedo;
    float3 fresnelR0;
    float shininess;
};

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff.
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
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

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, Material mat)
{
    const float m = mat.shininess * 256.0f;
    float3 halfVec = normalize(toEye + lightVec);

    float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    float3 fresnelFactor = SchlickFresnel(mat.fresnelR0, halfVec, lightVec);

    float3 specAlbedo = fresnelFactor * roughnessFactor;

    // Our spec formula goes outside [0,1] range, but we are 
    // doing LDR rendering.  So scale it down a bit.
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (mat.diffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for directional lights.
//---------------------------------------------------------------------------------------
float3 ComputeDirectionalLight(DirectionalLightContents light, Material mat, float3 normal, float3 toEye)
{
    // The light vector aims opposite the direction the light rays travel.
    float3 lightVec = -light.direction;

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = light.strength * ndotl;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for point lights.
//---------------------------------------------------------------------------------------
float3 ComputePointLight(PointLightContents light, Material mat, float3 pos, float3 normal, float3 toEye)
{
    // The vector from the surface to the light.
    float3 lightVec = light.position - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test.
    if (d > light.falloffEnd)
        return 0.0f;

    // Normalize the light vector.
    lightVec /= d;

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = light.strength * ndotl;

    // Attenuate light by distance.
    float att = CalcAttenuation(d, light.falloffStart, light.falloffEnd);
    lightStrength *= att;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for spot lights.
//---------------------------------------------------------------------------------------
float3 ComputeSpotLight(SpotLightContents light, Material mat, float3 pos, float3 normal, float3 toEye)
{
    // The vector from the surface to the light.
    float3 lightVec = light.position - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test.
    if (d > light.falloffEnd)
        return 0.0f;

    // Normalize the light vector.
    lightVec /= d;

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = light.strength * ndotl;

    // Attenuate light by distance.
    float att = CalcAttenuation(d, light.falloffStart, light.falloffEnd);
    lightStrength *= att;

    // Scale by spotlight
    float spotFactor = pow(max(dot(-lightVec, light.direction), 0.0f), light.spotPower);
    lightStrength *= spotFactor;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float4 ComputeLighting(DirectionalLightContents dLights[MaxLights],
    PointLightContents pLights[MaxLights], 
    SpotLightContents sLights[MaxLights], 
    int dLightsMax,
    int pLightsMax,
    int sLightsMax,
    Material mat,
    float3 pos,
    float3 normal,
    float3 toEye, 
    float shadowFactor)
{
    float3 result = 0.0f;

    for (int i = 0; i < dLightsMax; ++i)
        result += shadowFactor * ComputeDirectionalLight(dLights[i], mat, normal, toEye);
    for (int i = 0; i < pLightsMax; ++i)
        result += ComputePointLight(pLights[i], mat, pos, normal, toEye);
    for (int i = 0; i < sLightsMax; ++i)
        result += ComputeSpotLight(sLights[i], mat, pos, normal, toEye);

    return float4(result, 0.0f);
}

float4 ComputeSLighting(SMDirectionalLightContents smDLight[MaxLights],
    SMPointLightContents smPLights[MaxLights],
    SMSpotLightContents smSLights[MaxLights],
    int dLightsMax,
    int pLightsMax,
    int sLightsMax,
    Material mat,
    float3 pos, 
    float3 normal, 
    float3 toEye,
    float shadowFactor[MaxLights * 3])
{
    float3 result = 0.0f;
    int shadowIndex = 0;
    for (int i = 0; i < dLightsMax; ++i)
    {
        result += shadowFactor[shadowIndex] * ComputeDirectionalLight(smDLight[i].dLight, mat, normal, toEye);
         ++shadowIndex;
    }
    for (int i = 0; i < pLightsMax; ++i)
    {
        result += shadowFactor[shadowIndex] * ComputePointLight(smPLights[i].pLight, mat, pos, normal, toEye);
        ++shadowIndex;
    }
    for (int i = 0; i < sLightsMax; ++i)
    {
        result += shadowFactor[shadowIndex] * ComputeSpotLight(smSLights[i].sLight, mat, pos, normal, toEye);
        ++shadowIndex;
    }

    return float4(result, 0.0f);
}


