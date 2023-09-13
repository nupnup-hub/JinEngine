#include "LightingUtil.hlsl"
#include "DepthFunc.hlsl"

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

#ifdef CUBE_MAP_COUNT
TextureCube cubeMap[CUBE_MAP_COUNT]: register(t2, space0);
#endif
#ifdef TEXTURE_2D_COUNT
Texture2D textureMaps[TEXTURE_2D_COUNT] : register(t2, space1);
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

cbuffer cbEnginePass : register(b2)
{ 
    float appTotalTime;
    float aapDeltaTime;
    uint enginePassPad00;
    uint enginePassPad01;
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

cbuffer cbBoundingObject : register(b5)
{
    float4x4 boundObjWorld;
};

cbuffer cbCullingCommon: register(b6)
{
    float4x4 cullViewProj;
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

float CalShadowFactor(float4 shadowPosH, int shadowMapIndex)
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

uint CalCsmIndex(uint dataIndex, float4 posV, out float4 textureCoord)
{
    textureCoord = 0.0f;
    const uint validCount = csmData[dataIndex].count;
    for (uint i = 0; i < validCount; ++i)
    {
        textureCoord = posV * csmData[dataIndex].scale[i];
        textureCoord += csmData[dataIndex].posOffset[i];
        if (min(textureCoord.x, textureCoord.y) > csmData[dataIndex].mapMinBorder &&
            max(textureCoord.x, textureCoord.y) < csmData[dataIndex].mapMaxBorder)
        {
            return i;
        }
    }
    return validCount - 1;
}
float CalCascadeShadowFactor(float4 posV, int shadowMapIndex, int dataIndex)
{  
    float4 textureCoord = 0.0f;
    uint csmIndex = CalCsmIndex(dataIndex, posV, textureCoord);

    textureCoord.xyz /= textureCoord.w;
    float depth = textureCoord.z;

    uint width, height, count, numMips;
    shadowArray[shadowMapIndex].GetDimensions(csmIndex, width, height, count, numMips);

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
        percentLit += shadowArray[shadowMapIndex].SampleCmpLevelZero(samShadow,
            float3(textureCoord.xy + offsets[i], csmIndex),
            depth).r;
    }

    return percentLit / 9.0f;
}
float CalCubeShadowFactor(float3 posW, int lightIndex, int shadowMapIndex)
{
    float3 lightToPos = posW - pointLight[lightIndex].position;
 
    float depth = ToNoLinearZValue(lightToPos.z, pointLight[lightIndex].nearPlane, pointLight[lightIndex].farPlane);
    float3 direction = normalize(lightToPos);

    uint width, height, numMips;
    shadowCubeMap[shadowMapIndex].GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float)width;

    float percentLit = 0.0f;
    const float3 offsets[9] =
    {
        float3(-dx,  -dx, 0.0f), float3(0.0f,  -dx, 0.0f), float3(dx,  -dx, 0.0f),
        float3(-dx, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(dx, 0.0f, 0.0f),
        float3(-dx,  +dx, 0.0f), float3(0.0f,  +dx, 0.0f), float3(dx,  +dx, 0.0f)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        //offsets[i]
        percentLit += shadowCubeMap[shadowMapIndex].SampleCmpLevelZero(samShadow,
            direction + offsets[i],
            depth).r;
    }
    return percentLit / 9.0f;
}