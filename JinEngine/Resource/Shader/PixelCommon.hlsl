#pragma once
#include"VertexCommon.hlsl"
#include"Material.hlsl"
#include"LightDefine.hlsl"
#include"DepthFunc.hlsl"
 
#define PARALLAX_STEP 4 
#define PARALLAX_SCALE	0.03125f //	1 / 16 .. 1/ 32

//cbuffer && StructuredBuffer aligned 16byte
//dx12 cbuffer aligned 256byte

StructuredBuffer<DirectionalLightData> directionalLight : register(t0, space0);
StructuredBuffer<PointLightData> pointLight : register(t0, space1);
StructuredBuffer<SpotLightData> spotLight : register(t0, space2);
StructuredBuffer<RectLightData> rectLight : register(t0, space3);
StructuredBuffer<CsmData> csmData : register(t0, space4);
 
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
//#ifdef USE_SSAO
//RWTexture2D<float2> normalMap : register(u0);
//#endif
// Put in space1, so the texture array does not overlap with these resources.  
// The texture array will occupy registers t0, t1, ..., t3 in space0. 
//RWBuffer<float> shadowFactor : register(u0);
 
SamplerState samPointClamp       : register(s0);
SamplerState samLinearWrap       : register(s1); 
SamplerState samAnisotropicWrap  : register(s2);  
SamplerState samPcssBloker : register(s3); 
SamplerState samLTC : register(s4);
SamplerState samLTCSample : register(s5);
SamplerComparisonState samCmpLinearPointShadow : register(s6);

//48
cbuffer cbEnginePass : register(b2)
{ 
    float appTotalTime;
    float aapDeltaTime;
	int missingTextureIndex;
	int bluseNoiseTextureIndex;
	float2 bluseNoiseTextureSize;
	float2 invBluseNoiseTextureSize;
	int ltcMatTextureIndex;
	int ltcAmpTextureIndex;
	int passPad00;
	int paddPad01;
};

//48
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
	uint rectLitSt;
	uint rectLitEd;
	uint scenePassPad00;
	uint scenePassPad01;
};

//b0 + b2 + b3 => 256 * 3
//b2 = 16384
//b4 = 512
//sum = 17664byte
//register 크기는 는 multi processor당 고정되있으며
//하나의 thread가 많은 register 사용시 동시에 실행될수있는 thread에 수가 줄어들며 이는 block에 감소로 이어진다.
//block단위로 감소하기때문에 긴지연시간을 가진 연산이 발생하면 프로세서가 유용한 일들을 찾을수있는 능력을 감소시킨다.
//ex latency hiding을 통해 다른 워프 작업하기

float4x4 Identity()
{
    return float4x4(1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}
 
float3x3 CalTBN(float3 unitNormalW, float3 tangentW)
{
    // Build orthonormal basis.
	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N) * N);
	float3 B = cross(N, T);

	return float3x3(T, B, N);
}
float2 PallaxMapping(const float3 viewDir, const float2 texC, const int heightMapIndex)
{
	float height = textureMaps[heightMapIndex].Sample(samAnisotropicWrap, texC).r * 0.1f;
	return texC + (height * viewDir.xy); 
}
// Shifting UV by using Parallax Mapping
float2 ApplyParallaxOffset(float2 uv, float3 vDir, int normalMapIndex, int heightMapIndex)
{
	//float2 scale = (PARALLAX_SCALE * width) / (2.0f * PARALLAX_STEP * width);
	float2 scale = PARALLAX_SCALE / (2.0f * PARALLAX_STEP);
	float2 pdir = vDir.xy * scale;
    [unroll]
	for (int i = 0; i < PARALLAX_STEP; ++i)
	{
    // This code can be replaced with fetching parallax map for parallax variable(h * nz)
		float nz = (textureMaps[normalMapIndex].Sample(samAnisotropicWrap, uv) * 2.0 - 1.0).z;
		float h = textureMaps[heightMapIndex].Sample(samAnisotropicWrap, uv).r;
		uv += pdir * (nz * h);
	}
	return uv;
}
