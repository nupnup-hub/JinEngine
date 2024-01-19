#pragma once
#include"DebugCommon.hlsl"
#include"Math.hlsl"

/*
dim default value
group 1, 512, 1
thread 512, 1, 1
*/
[numthreads(DIMX, DIMY, DIMZ)]
void VisualizeNormalMap(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	if (width <= dispatchThreadID.x || height <= dispatchThreadID.y)
		return;

	float textureXFactor = dispatchThreadID.x;
	float textureYFactor = dispatchThreadID.y;
	uint maxPixelCount = width * height;

	while (maxPixelCount > (textureXFactor + (textureYFactor * width)))
	{
		const float3 normal = DecodeOct(srcMap.Load(int3(textureXFactor, textureYFactor, 0)).xy);
		result[int2(textureXFactor, textureYFactor)] = float4(normal, 1.0f);
		textureXFactor += 512;
		if (textureXFactor >= width)
		{
			textureXFactor = dispatchThreadID.x;
			textureYFactor += 512;
		}
	}
}