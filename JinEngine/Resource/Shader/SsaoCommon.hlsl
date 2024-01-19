#pragma once 
#include"FilterCommon.hlsl" 

#ifndef SSAO_SAMPLE_COUNT 
#define SSAO_SAMPLE_COUNT 16
#endif

#ifndef SSAO_RANDOM_MAP_SIZE 
#define SSAO_RANDOM_MAP_SIZE 4.0f
#endif
 
#ifndef SSAO_DIR_COUNT
#define SSAO_DIR_COUNT 8
#endif

cbuffer cbSsaoPass : register(b0)
{
	float4x4 camView;
	float4x4 camProj;
	float radius;
	float radius2;
	float bias;
	float sharpness; 
	float2 camNearFar; 
	float2 camRtSize;
	float2 camInvRtSize;
	float2 aoRtSize;
	float2 aoInvRtSize; 
	float2 aoInvQuaterRtSize;
	float2 uvToViewA;
	float2 uvToViewB;
	float2 viewPortTopLeft;
	float radiusToScreen;
	float negInvR2;
	float tanBias;
	float smallScaleAOAmount;
	float largeScaleAOAmount;
	float camNearMulFar;
};
#ifdef USE_HBAO
cbuffer cbSsaoSlice: register(b1)
{
	float4 jitter;
	float2 posOffset;
	float sliceIndex;
	unsigned int uSliceIndex;
};   
#elif USE_SSAO
cbuffer sampleCB : register(b1)
{
	float4 sample[SSAO_SAMPLE_COUNT];
}; 
#endif
#include"FullScreenTriangleVs.hlsl" 

