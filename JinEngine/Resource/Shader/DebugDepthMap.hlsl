#pragma once
#include"DebugCommon.hlsl"
 
/*
dim default value
group 1, 512, 1
thread 512, 1, 1
*/
[numthreads(DIMX, DIMY, DIMZ)]
void LinearMap(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	if (width <= dispatchThreadID.x || height <= dispatchThreadID.y)
		return;

	float textureXFactor = dispatchThreadID.x;
	float textureYFactor = dispatchThreadID.y;
	uint maxPixelCount = width * height;

	while (maxPixelCount > (textureXFactor + (textureYFactor * width)))
	{
		//linear depth value
		const float z = 1 - srcMap.Load(int3(textureXFactor, textureYFactor, 0)).r;
		//float z = srcMap.SampleLevel(samLinearWrap, float2(textureXFactor / width, textureYFactor / height), 0).r;
		// (2.0 * n) / (f + n - z * (f - n));	 
		//float factor = 1 - (2.5f * camNearn) / (camFar + camNear - z * (camFar - camNear));
		result[int2(textureXFactor, textureYFactor)] = float4(z, z, z, z);
		textureXFactor += 512;
		if (textureXFactor >= width)
		{
			textureXFactor = dispatchThreadID.x;
			textureYFactor += 512;
		} 
	}
} 

[numthreads(DIMX, DIMY, DIMZ)]
void NonLinearMap(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	if (width <= dispatchThreadID.x || height <= dispatchThreadID.y)
		return;

	float textureXFactor = dispatchThreadID.x;
	float textureYFactor = dispatchThreadID.y;
	uint maxPixelCount = width * height;

	while (maxPixelCount > (textureXFactor + (textureYFactor * width)))
	{ 
#ifdef USE_PERSPECTIVE
		const float z = 1 - LinearDepth(srcMap.Load(int3(textureXFactor, textureYFactor, 0)).r, near, far);
#else
		const float z = 1 - srcMap.Load(int3(textureXFactor, textureYFactor, 0)).r;
#endif
		result[int2(textureXFactor, textureYFactor)] = float4(z, z, z, z);
		textureXFactor += 512;
		if (textureXFactor >= width)
		{
			textureXFactor = dispatchThreadID.x;
			textureYFactor += 512;
		}
	}
}