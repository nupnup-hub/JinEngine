/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"VertexShaderCommon.hlsl"  
#include"../Common/DepthFunc.hlsl"
#include"../Common/Packing.hlsl" 
#include"../Common/GBufferCommon.hlsl"
#include"../Culling/Light/ClusterCommon.hlsl"

#ifndef DEFERRED_GEOMETRY
#include"../Common/LightDefine.hlsl"
#endif

#define PARALLAX_STEP 4 
#define PARALLAX_SCALE	0.03125f //	1 / 16 .. 1/ 32

#ifndef TEXTURE_2D_COUNT
#define TEXTURE_2D_COUNT 0
#endif
#ifndef CUBE_MAP_COUNT
#define CUBE_MAP_COUNT 0
#endif
#ifndef SHADOW_MAP_COUNT
#define SHADOW_MAP_COUNT 0
#endif
#ifndef SHADOW_MAP_ARRAY_COUNT
#define SHADOW_MAP_ARRAY_COUNT 0
#endif
#ifndef SHADOW_MAP_CUBE_COUNT
#define SHADOW_MAP_CUBE_COUNT 0
#endif
  
#ifdef DEFERRED_GEOMETRY 
Texture2D textureMaps[TEXTURE_2D_COUNT] : register(t2, space0);
TextureCube cubeMap[CUBE_MAP_COUNT] : register(t2, space1);
#else
StructuredBuffer<DirectionalLightData> directionalLight : register(t0, space0);
StructuredBuffer<PointLightData> pointLight : register(t0, space1);
StructuredBuffer<SpotLightData> spotLight : register(t0, space2);
StructuredBuffer<RectLightData> rectLight : register(t0, space3);
StructuredBuffer<CsmData> csmData : register(t0, space4);

Texture2D textureMaps[TEXTURE_2D_COUNT] : register(t2, space0);
TextureCube cubeMap[CUBE_MAP_COUNT] : register(t2, space1);
Texture2D shadowMaps[SHADOW_MAP_COUNT] : register(t2, space2);
Texture2DArray shadowArray[SHADOW_MAP_ARRAY_COUNT] : register(t2, space3);
TextureCube shadowCubeMap[SHADOW_MAP_CUBE_COUNT] : register(t2, space4);
#endif

Texture2D ambientOcclusionMap : register(t2, space5);

#ifdef DEFERRED_SHADING
Texture2D gBuffer[G_BUFFER_LAYER_COUNT] : register(t2, space6);
Texture2D depthMap : register(t2, space7); //this depth map is pre frame result if forward
#endif
 
#if !defined (DEFERRED_GEOMETRY) && defined (LIGHT_CLUSTER)
ByteAddressBuffer startOffsetBuffer : register(t2, space8);
StructuredBuffer<LinkedLightID> linkedLightList : register(t2, space9);
#endif
 
#ifdef GLOBAL_ILLUMINATION
Texture2D giMap : register(t2, space10);
#endif

SamplerState samPointClamp : register(s0);
SamplerState samPointWrap : register(s1);
SamplerState samLinearWrap : register(s2);
SamplerState samAnisotropicWrap : register(s3);
#ifndef DEFERRED_GEOMETRY
SamplerState samPcssBloker : register(s4);
SamplerState samLTC : register(s5);
SamplerState samLTCSample : register(s6);
SamplerComparisonState samCmpLinearPointShadow : register(s7);
#endif

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
float2 PallaxMapping(Texture2D heightMap, const float3 viewDir, const float2 texC)
{
    float height = heightMap.Sample(samAnisotropicWrap, texC).r * 0.1f;
    return texC + (height * viewDir.xy);
}
// Shifting UV by using Parallax Mapping
float2 ApplyParallaxOffset(Texture2D normalMap, Texture2D heightMap, float2 uv, float3 vDir)
{ 
	//float2 scale = (PARALLAX_SCALE * width) / (2.0f * PARALLAX_STEP * width);
    float2 scale = PARALLAX_SCALE / (2.0f * PARALLAX_STEP);
    float2 pdir = vDir.xy * scale;
    [unroll]
    for (int i = 0; i < PARALLAX_STEP; ++i)
    {
    // This code can be replaced with fetching parallax map for parallax variable(h * nz)
        float nz = (normalMap.Sample(samAnisotropicWrap, uv) * 2.0 - 1.0).z;
        float h = heightMap.Sample(samAnisotropicWrap, uv).r;
        uv += pdir * (nz * h);
    }
    return uv;
} 

float LinearDepth(const float v)
{
    return LinearDepth(v, cbCam.nearZ, cbCam.FarZ);
}
float NonLinearDepth(const float v)
{
    return NonLinearDepth(v, cbCam.nearZ, cbCam.FarZ);
}
float NdcToViewPZ(const float v)
{ 
    return NdcToViewPZ(v, cbCam.nearZ, cbCam.FarZ);
}
float ViewToNdcPZ(const float v)
{
    return ViewToNdcPZ(v, cbCam.nearZ, cbCam.FarZ);
}
float NdcToViewOZ(const float v)
{
    return NdcToViewOZ(v, cbCam.nearZ, cbCam.FarZ);
}
float ViewToNdcOZ(const float v)
{
    return ViewToNdcOZ(v, cbCam.nearZ, cbCam.FarZ);
}

 #define GI_APP_DIRECT_LIGHT_COLOR_FACTOR 0.9f
#define GI_APP_MATERIAL_COLOR_FACTOR 0.3f
#define GI_APP_LIGHT_COLOR_FACTOR 0.25f

#define GI_DIRECT_LIGHT_COLOR_FACTOR 0.9f
#define GI_MATERIAL_COLOR_FACTOR 0.4f
#define GI_LIGHT_COLOR_FACTOR 0.25f

float3 CombineGlobalLight(float3 directLight, float3 albedoColor, float3 giColor, float aoFactor)
{  
    if (aoFactor == 0)
        return directLight;
  
    float3 ambientLight = giColor * aoFactor;
    return directLight * GI_DIRECT_LIGHT_COLOR_FACTOR + directLight * ambientLight * GI_LIGHT_COLOR_FACTOR + albedoColor * ambientLight * GI_MATERIAL_COLOR_FACTOR;
}
float3 CombineApproxGlobalLight(float3 directLight, float3 albedoColor, float aoFactor)
{ 
    return directLight * GI_APP_DIRECT_LIGHT_COLOR_FACTOR + directLight * aoFactor * GI_APP_LIGHT_COLOR_FACTOR + albedoColor * aoFactor * GI_APP_MATERIAL_COLOR_FACTOR;
}
 