#pragma once 

#include"LightDefine.hlsl"
#include"LightClusterCommon.hlsl"
#include"Math.hlsl"
#include"CBCameraDefine.hlsl"

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

cbuffer cbPass : register(b1)
{
	uint litOffset;	
};
