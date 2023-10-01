#include "LightingUtil.hlsl"
#include "DepthFunc.hlsl"

#define JINENGINE_COMMON 1

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

StructuredBuffer<DirectionalLightData> directionalLight : register(t0, space0);
StructuredBuffer<PointLightData> pointLight : register(t0, space1);
StructuredBuffer<SpotLightData> spotLight : register(t0, space2);
StructuredBuffer<CsmData> csmData : register(t0, space3);
StructuredBuffer<MaterialData> materialData : register(t1);

#ifdef TEXTURE_2D_COUNT
Texture2D textureMaps[TEXTURE_2D_COUNT] : register(t2, space0);
#endif
#ifdef CUBE_MAP_COUNT
TextureCube cubeMap[CUBE_MAP_COUNT]: register(t2, space1);
#endif
#ifdef SHADOW_MAP_COUNT
Texture2D shadowMaps[SHADOW_MAP_COUNT] : register(t2, space2);
#endif
#ifdef SHADOW_MAP_ARRAY_COUNT
Texture2DArray shadowArray[SHADOW_MAP_ARRAY_COUNT] : register(t2, space3);
#endif
#ifdef SHADOW_MAP_CUBE_COUNT
TextureCube shadowCubeMap[SHADOW_MAP_CUBE_COUNT] : register(t2, space4);
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
SamplerState samCubeShadow : register(s6);
SamplerState samPcssBloker : register(s7);
SamplerComparisonState samCmpPcssFilter : register(s8);
SamplerComparisonState samCmpLinearPointShadow : register(s9);

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

cbuffer cbEnginePass : register(b2)
{ 
    float appTotalTime;
    float aapDeltaTime;
	int missingTextureIndex;
	int bluseNoiseTextureIndex;
	float2 bluseNoiseTextureSize;
	float2 invBluseNoiseTextureSize;
};

cbuffer cbScenePass : register(b3)
{
    float sceneTotalTime;
    float sceneDeltaTime;
    uint directionalLitSt;   
    uint directionalLitEd;
    uint pointLitSt;  
    uint pointLitEd;
    uint spotLitSt;    
    uint spotLitEd;
};

cbuffer cbCamera: register(b4)
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
    float camNearZ;
    float camFarZ;
    uint csmLocalIndex;   //aligned by registered time
    uint cameraPad00;
    uint cameraPad01; 
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