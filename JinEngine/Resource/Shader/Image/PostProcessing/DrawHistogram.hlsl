//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//
#ifndef DIMX
#define DIMX 256
#endif

#define HISTOGRAM_COUNT DIMX

ByteAddressBuffer histogram : register(t0);
StructuredBuffer<float> exposure : register(t1);
RWTexture2D<float4> colorBuffer : register(u0);

groupshared uint gs_hist[DIMX];
 
[numthreads(DIMX, 1, 1)]
void DrawHistogram(uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint histValue = histogram.Load(groupIndex * 4);

    // Compute the maximum histogram value, but don't include the black pixel
	gs_hist[groupIndex] = groupIndex == 0 ? 0 : histValue;
	GroupMemoryBarrierWithGroupSync();
	gs_hist[groupIndex] = max(gs_hist[groupIndex], gs_hist[(groupIndex + 128) % HISTOGRAM_COUNT]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[groupIndex] = max(gs_hist[groupIndex], gs_hist[(groupIndex + 64) % HISTOGRAM_COUNT]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[groupIndex] = max(gs_hist[groupIndex], gs_hist[(groupIndex + 32) % HISTOGRAM_COUNT]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[groupIndex] = max(gs_hist[groupIndex], gs_hist[(groupIndex + 16) % HISTOGRAM_COUNT]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[groupIndex] = max(gs_hist[groupIndex], gs_hist[(groupIndex + 8) % HISTOGRAM_COUNT]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[groupIndex] = max(gs_hist[groupIndex], gs_hist[(groupIndex + 4) % HISTOGRAM_COUNT]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[groupIndex] = max(gs_hist[groupIndex], gs_hist[(groupIndex + 2) % HISTOGRAM_COUNT]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[groupIndex] = max(gs_hist[groupIndex], gs_hist[(groupIndex + 1) % HISTOGRAM_COUNT]);
	GroupMemoryBarrierWithGroupSync();

	uint maxHistValue = gs_hist[groupIndex];

	uint2 bufferDim;
	colorBuffer.GetDimensions(bufferDim.x, bufferDim.y);

	const uint2 rectCorner = uint2(bufferDim.x / 2 - 512, bufferDim.y - 256);
	const uint2 groupCorner = rectCorner + dispatchThreadID.xy * 4;

	uint height = 127 - dispatchThreadID.y * 4;
	uint threshold = histValue * 128 / max(1, maxHistValue);

	float4 outColor = (groupIndex == (uint) exposure[3]) ? float4(1.0, 1.0, 0.0, 1.0f) : float4(0.5, 0.5, 0.5, 1.0f);

	for (uint i = 0; i < 4; ++i)
	{
		float4 maskedColor = (height - i) < threshold ? outColor : float4(0, 0, 0, 1.0f);

        // 4-wide column with 2 pixels for the histogram bar and 2 for black spacing
		colorBuffer[groupCorner + uint2(0, i)] = maskedColor;
		colorBuffer[groupCorner + uint2(1, i)] = maskedColor;
		colorBuffer[groupCorner + uint2(2, i)] = float4(0, 0, 0, 1.0f);
		colorBuffer[groupCorner + uint2(3, i)] = float4(0, 0, 0, 1.0f);
	}
}
