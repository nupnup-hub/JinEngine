#pragma once
#ifndef CB_CAM_REIGSTER
#define CB_CAM_REIGSTER b2
#endif

//368
cbuffer cbCamera : register(CB_CAM_REIGSTER)
{
	float4x4 camView;
	float4x4 camInvView;
	float4x4 camProj; 
	float4x4 camInvProj;
	float4x4 camViewProj;
	//float4x4 camViewProjTex;
	//float4x4 camInvViewProj;
	float2 camRenderTargetSize;
	float2 camInvRenderTargetSize;
	float3 camEyePosW;
	float camNearZ; 
	float camFarZ;
	uint csmLocalIndex; //aligned by registered time
	uint cameraPad00;
	uint cameraPad01; 
};

float NdcToViewPZ(const float depth)
{  
	return camProj[3][2] / (depth - camProj[2][2]);
}