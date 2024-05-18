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
#include"../Common/DepthFunc.hlsl" 
#include"../Common/GBufferCommon.hlsl"

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
#elif ALBEDO_MAP 
[numthreads(DIMX, DIMY, DIMZ)]
void VisualizeAlbedoMap(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (resolution.x <= dispatchThreadID.x || resolution.y <= dispatchThreadID.y)
        return;
	
    float3 albedoColor = UnpackAlbedoColor(srcMap.Load(int3(dispatchThreadID.xy, 0)));
    result[dispatchThreadID.xy] = float4(albedoColor, 1.0f);
}
#elif SPECULAR_MAP 
[numthreads(DIMX, DIMY, DIMZ)]
void VisualizeSpecularMap(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (resolution.x <= dispatchThreadID.x || resolution.y <= dispatchThreadID.y)
        return;
	
    float specularIntensity = UnpackSpecularFactor(srcMap.Load(int3(dispatchThreadID.xy, 0)));
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
 