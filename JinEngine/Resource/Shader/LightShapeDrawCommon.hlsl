#pragma once 
#include"LightDefine.hlsl"
#include"LightClusterCommon.hlsl"
#include"Math.hlsl" 

#ifndef LIGHT_RANGE_OFFSET
#define LIGHT_RANGE_OFFSET 1.0f
#endif

#ifdef POINT_LIGHT 
StructuredBuffer<PointLightData> light : register(t0);
#elif SPOT_LIGHT 
StructuredBuffer<SpotLightData> light : register(t0);
#elif RECT_LIGHT 
StructuredBuffer<RectLightData> light : register(t0);
#else 
#endif 

struct CameraData
{
	float4x4 view;
	float4x4 proj;
	float2 renderTargetSize;
	float2 invRenderTargetSize;
	float nearZ;
	float farZ;
	uint pad00;
	uint pad01;
}; 

ConstantBuffer<CameraData> cbCam : register(b0);
 
 
 
