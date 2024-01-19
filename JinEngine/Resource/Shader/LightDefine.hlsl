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
	float3 color;  
	float2 extents;
	int sourceTextureIndex;
	float frustumNear;
	float frustumFar;
	float barndoorLength;
	float barndoorCosAngle;
	int shadowMapIndex;
	uint hasShadowMap;
	int rLightPad00;
	int rLightPad01;
	int rLightPad02;
};