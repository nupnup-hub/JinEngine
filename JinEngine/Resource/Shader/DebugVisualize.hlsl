#pragma once
#include"DepthFunc.hlsl" 
#include"GBufferCommon.hlsl"

#ifndef DIMX
#define DIMX 1
#endif
#ifndef DIMY
#define DIMY 1
#endif
#ifndef DIMZ
#define DIMZ 1
#endif 
 
#ifdef VELOCITY_MAP
Texture2D<uint> srcMap : register(t0); 
#else
Texture2D srcMap : register(t0);
#endif
RWTexture2D<float4> result : register(u0);
SamplerState samLinearWrap : register(s0);

cbuffer cbSettings : register(b0)
{
    uint2 resolution;
    float2 nearFar;
};
 

/*
dim default value 
thread 16, 16, 1
*/
#ifdef DEPTH_LINEAR_MAP
[numthreads(DIMX, DIMY, DIMZ)]
void VisualizeLinearMap(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	if (resolution.x <= dispatchThreadID.x || resolution.y <= dispatchThreadID.y)
		return;

	/*
	//float z = srcMap.SampleLevel(samLinearWrap, float2(textureXFactor / (float)resolution.x, textureYFactor / (float)resolution.y), 0).r;
	// (2.0 * n) / (f + n - z * (f - n));	 
	//float factor = 1 - (2.5f * camNearn) / (camFar + camNear - z * (camFar - camNear));
	*/
	//linear depth value
	const float z = 1 - srcMap.Load(int3(dispatchThreadID.xy, 0)).r;
	result[dispatchThreadID.xy] = float4(z, z, z, z); 
} 
#elif  DEPTH_NON_LINEAR_MAP
[numthreads(DIMX, DIMY, DIMZ)]
void VisualizeNonLinearMap(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	if (resolution.x <= dispatchThreadID.x || resolution.y <= dispatchThreadID.y)
		return;

#ifdef USE_PERSPECTIVE
	const float z = 1 - LinearDepth(srcMap.Load(int3(dispatchThreadID.xy, 0)).r, nearFar.x, nearFar.y);
#else
	const float z = 1 - srcMap.Load(int3(dispatchThreadID.xy, 0)).r;
#endif
	result[dispatchThreadID.xy] = float4(z, z, z, z);
}
#elif SPECULAR_MAP 
[numthreads(DIMX, DIMY, DIMZ)]
void VisualizeSpecularMap(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (resolution.x <= dispatchThreadID.x || resolution.y <= dispatchThreadID.y)
        return;
	
    float specularIntensity = srcMap.Load(int3(dispatchThreadID.xy, 0)).x; 
    result[dispatchThreadID.xy] = float4(specularIntensity, specularIntensity, specularIntensity, 1.0f);
}
#elif  NORMAL_MAP
[numthreads(DIMX, DIMY, DIMZ)]
void VisualizeNormalMap(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	if (resolution.x <= dispatchThreadID.x || resolution.y <= dispatchThreadID.y)
		return;
 
	result[dispatchThreadID.xy] = float4(UnpackNormal(srcMap.Load(int3(dispatchThreadID.xy, 0))), 1.0f);
}
#elif  TANGENT_MAP 
[numthreads(DIMX, DIMY, DIMZ)]
void VisualizeTangentMap(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (resolution.x <= dispatchThreadID.x || resolution.y <= dispatchThreadID.y)
        return;
	
    float4 encodeNormalAndTangent = srcMap.Load(int3(dispatchThreadID.xy, 0)); 
	float3 normal;
    float3 tangent;
	UnpackNormalAndTangentLayer(encodeNormalAndTangent, normal, tangent);
    result[dispatchThreadID.xy] = float4(tangent, 1.0f);
}
#elif  VELOCITY_MAP
[numthreads(DIMX, DIMY, DIMZ)]
void VisualizeVelocityMap(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (resolution.x <= dispatchThreadID.x || resolution.y <= dispatchThreadID.y)
        return;
	  
    uint encodeVelocity = srcMap.Load(int3(dispatchThreadID.xy, 0));
    result[dispatchThreadID.xy] = float4(abs(UnpackVelocity(encodeVelocity).xy), 0.0f, 1.0f);
}
#elif  SSAO_MAP
[numthreads(DIMX, DIMY, DIMZ)]
void VisualizeAoMap(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	if (resolution.x <= dispatchThreadID.x || resolution.y <= dispatchThreadID.y)
		return;
	
	const float ao = srcMap.Load(int3(dispatchThreadID.xy, 0)).r;
	result[dispatchThreadID.xy] = float4(ao, ao, ao, ao);
} 
#else
#endif 
 