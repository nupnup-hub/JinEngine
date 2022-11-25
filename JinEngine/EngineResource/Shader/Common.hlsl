#include "LightingUtil.hlsl"

struct MaterialData
{
	float4 albedoColor; 
	float4x4 matTransform;
    float metallic;
    float roughness;
	uint albedoMapIndex;
    uint normalMapIndex;
	uint heightMapIndex;
	uint roughnessMapIndex;  
    uint ambientMapIndex;
    uint materialObjPad;
};

StructuredBuffer<LightData> light : register(t0);
StructuredBuffer<ShadowMapLightData> smLight : register(t0, space1);
StructuredBuffer<MaterialData> materialData : register(t1);

#ifdef CUBE_MAP_COUNT
TextureCube cubeMap[CUBE_MAP_COUNT]: register(t2, space1);
#endif
#ifdef TEXTURE_2D_COUNT
Texture2D textureMaps[TEXTURE_2D_COUNT] : register(t2, space2);
#endif
#ifdef SHADOW_MAP_COUNT
Texture2D shadowMaps[SHADOW_MAP_COUNT] : register(t2, space3);
#endif

// Put in space1, so the texture array does not overlap with these resources.  
// The texture array will occupy registers t0, t1, ..., t3 in space0. 
//RWBuffer<float> shadowFactor : register(u0);

SamplerState samPointWrap        : register(s0);
SamplerState samPointClamp       : register(s1);
SamplerState samLinearWrap       : register(s2);
SamplerState samLinearClamp      : register(s3);
SamplerState samAnisotropicWrap  : register(s4);
SamplerState samAnisotropicClamp : register(s5);
SamplerComparisonState samShadow : register(s6);

cbuffer cbObject : register(b0)
{
    float4x4 objWorld;
    float4x4 objTexTransform;
    uint objMaterialIndex;
    uint objPad00;
    uint objPad01;
    uint objPad02;
};

cbuffer cbSkinned : register(b1)
{
    float4x4 objBoneTransforms[256];
};
 
cbuffer cbPass : register(b2)
{
    float4 sceneAmbientLight;
    float sceneTotalTime;
    float sceneDeltaTime;
    uint passPad01;
    uint passPad02;
};

cbuffer cbCamera: register(b3)
{
    float4x4 camView;
    float4x4 camInvView;
    float4x4 camProj;
    float4x4 camInvProj;
    float4x4 camViewProj;
    float4x4 camInvViewProj;
    float2 camRenderTargetSize;
    float2 camInvRenderTargetSize;
    float3 camEyePosW;
    uint cameraPad00;
    float camNearZ;
    float camFarZ;
    uint cameraPad01;
    uint cameraPad02;
};

cbuffer cbLightIndex : register(b4)
{
    uint litStIndex;
    uint litEdIndex;
    uint shadwMapStIndex;
    uint shadowMapEdIndex;
};

cbuffer cbShadowMapCalculate: register(b5)
{
    float4x4 lightView;
    float4x4 lightInvView;
    float4x4 lightProj;
    float4x4 lightInvProj;
    float4x4 lightViewProj;
    float4x4 lightInvViewProj;
    float2 lightRenderTargetSize;
    float2 lightInvRenderTargetSize;
    float3 lightEyePosW;
    uint shadowCalPad00;
    float lightNearZ;
    float lightFarZ;
    uint shadowCalPad01;
    uint shadowCalPad02;
};

cbuffer cbBoundingObject : register(b6)
{
    float4x4 boundObjWorld;
};

float4x4 Identity()
{
    return float4x4(1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
    // Uncompress each component from [0,1] to [-1,1].
    float3 normalT = 2.0f * normalMapSample - 1.0f;

    // Build orthonormal basis.
    float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);

    // Transform from tangent space to world space.
    float3 bumpedNormalW = mul(normalT, TBN);

    return bumpedNormalW;
}

float CalcShadowFactor(float4 shadowPosH, int shadowMapIndex)
{
   // Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;

    // Depth in NDC space.
    float depth = shadowPosH.z;

    uint width, height, numMips;
    shadowMaps[shadowMapIndex].GetDimensions(0, width, height, numMips);
    
    // Texel size.
    float dx = 1.0f / (float)width;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += shadowMaps[shadowMapIndex].SampleCmpLevelZero(samShadow, shadowPosH.xy + offsets[i], depth).r;
    }

    return percentLit / 9.0f;
    // return   gShadowMap.SampleCmpLevelZero(gsamShadow, shadowPosH.xy, depth).r;
}