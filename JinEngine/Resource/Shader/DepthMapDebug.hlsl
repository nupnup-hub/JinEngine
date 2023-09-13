#include"DepthFunc.hlsl"

Texture2D depthMap	: register(t0);
RWTexture2D<float4> result	: register(u0);
SamplerState samLinearWrap	: register(s0);

cbuffer cbSettings : register(b0)
{
	uint width;
	uint height;
	float camNear;
	float camFar;
};

/*
dim info
group 1, 512, 1
thread 512, 1, 1
*/
[numthreads(512, 1, 1)]
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
		const float z = 1 - depthMap.Load(int3(textureXFactor, textureYFactor, 0)).r;
		//float z = depthMap.SampleLevel(samLinearWrap, float2(textureXFactor / width, textureYFactor / height), 0).r;
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

[numthreads(512, 1, 1)]
void NonLinearMap(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	if (width <= dispatchThreadID.x || height <= dispatchThreadID.y)
		return;

	float textureXFactor = dispatchThreadID.x;
	float textureYFactor = dispatchThreadID.y;
	uint maxPixelCount = width * height;

	while (maxPixelCount > (textureXFactor + (textureYFactor * width)))
	{  
		const float z = 1 - ToLinearZValue(depthMap.Load(int3(textureXFactor, textureYFactor, 0)).r, camNear, camFar);
		result[int2(textureXFactor, textureYFactor)] = float4(z, z, z, z);
		textureXFactor += 512;
		if (textureXFactor >= width)
		{
			textureXFactor = dispatchThreadID.x;
			textureYFactor += 512;
		}
	}
}