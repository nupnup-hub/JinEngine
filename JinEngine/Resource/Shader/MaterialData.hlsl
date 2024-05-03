#pragma once
 
#ifndef MISSING_TEXTURE_INDEX
#define MISSING_TEXTURE_INDEX 1
#endif
struct MaterialData
{
    float4 albedoColor;
    float4x4 matTransform;
    float metallic;
    float roughness;
    float specularFactor;
    uint albedoMapIndex;
    uint normalMapIndex;
    uint heightMapIndex;
    uint metallicMapIndex;
    uint roughnessMapIndex;
    uint ambientMapIndex;
    uint specularMapIndex;
    uint materialPad00;
    uint materialPad01;
};

float ComputeDefaultSpecularFactor(float3 albedoColor, float metallic)
{ 
    //to ruminance
    return dot(albedoColor.xyz * metallic, float3(0.212671, 0.715160, 0.072169));  
}
