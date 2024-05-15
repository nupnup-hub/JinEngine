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

#if !defined(CSM_MAX_COUNT)
#define CSM_MAX_COUNT 8
#endif
 
#if !defined(CUBE_MAP_FACE_COUNT)
#define CUBE_MAP_FACE_COUNT 6
#endif

#define DIRECTONAL_LIGHT_NON_SHADOW_MAP -1
#define DIRECTONAL_LIGHT_HAS_NORMAL_SHADOW_MAP 0
#define DIRECTONAL_LIGHT_HAS_CSM 1

//272byte
struct DirectionalLightData
{
	float4x4 view;
	float4x4 viewProj;
	float4x4 shadowMapTransform;
	float3 color;
	float power; 
	float3 direction;
	float frustumNear;
	float2 frustumSize;
	float frustumFar;
	float penumbraScale;
	float penumbraBlockerScale;
	int csmDataIndex; //data start index
	int shadowMapIndex;
	int shadowMapType; //-1 not, 0 normal, 1 csm, 2 cube
	float shadowMapSize;
	float shadowMapInvSize;
	float tanAngle;
	float bias;  
};
//336byte
struct CsmData
{
	float4 scale[CSM_MAX_COUNT]; // (projM * textureM) scale
	float4 posOffset[CSM_MAX_COUNT]; // (projM * textureM) translate
	//float2 frustumSize[CSM_MAX_COUNT];
	float frustumNear[CSM_MAX_COUNT];
	float frustumFar[CSM_MAX_COUNT];
	float mapMinBorder; //default value is 0
	float mapMaxBorder; //default value is 1
	float levelBlendRate;
	uint count;
};
//464byte
struct PointLightData
{
	float4x4 shadowMapTransform[CUBE_MAP_FACE_COUNT];
	float3 midPosition;
	float power;
	float3 sidePosition[2];
	float radius;
	float frustumNear;
	float3 color;
	float frustumFar;
	float penumbraScale;
	float penumbraBlockerScale;
	int shadowMapIndex;
	uint hasShadowMap;
	float shadowMapSize;
	float shadowMapInvSize;
	float bias;
	uint pLightPad00;
};
//144 + 32byte
struct SpotLightData
{
	float4x4 shadowMapTransform;
	float3 color;
	float power;
	float3 position;
	float frustumNear;
	float3 direction;
	float frustumFar;
	float innerConeCosAngle;
	float outerConeCosAngle;
	float outerConeAngle;
	float penumbraScale;
	float penumbraBlockerScale;
	int shadowMapIndex;
	uint hasShadowMap;
	float shadowMapSize;
	float shadowMapInvSize;
	float bias;
	uint sLightPad00;
	uint sLightPad01; 
};

struct RectLightData
{
	float4x4 shadowMapTransform;
	float3 origin;
	float power;
	float3 axis[3];
    float3 direction;
	float3 color;
    float frustumNear;	
	float2 extents;
    float frustumFar;
    float barndoorLength;
    float barndoorCosAngle;
    int shadowMapIndex;
    uint hasShadowMap;
	int sourceTextureIndex;
    int ltcMatTextureIndex;
    int ltcAmpTextureIndex;
	int rLightPad00;
    int rLightPad01;
};