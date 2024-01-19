#pragma once
#include"DebugCommon.hlsl"
 
/*
dim default value
group 1, 512, 1
thread 512, 1, 1
*/
[numthreads(DIMX, DIMY, DIMZ)]
void VisualizeAoMap(uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	if (width <= dispatchThreadID.x || height <= dispatchThreadID.y)
		return;

	float textureXFactor = dispatchThreadID.x;
	float textureYFactor = dispatchThreadID.y;
	uint maxPixelCount = width * height;

	while (maxPixelCount > (textureXFactor + (textureYFactor * width)))
	{ 
		const float ao = srcMap.Load(int3(textureXFactor, textureYFactor, 0)).r;
		result[int2(textureXFactor, textureYFactor)] = float4(ao, ao, ao, ao);
		textureXFactor += 512;
		if (textureXFactor >= width)
		{
			textureXFactor = dispatchThreadID.x;
			textureYFactor += 512;
		}
	}
} 