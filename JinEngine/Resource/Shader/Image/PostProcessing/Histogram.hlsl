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
// The group size is 16x16, but one group iterates over an entire 16-wide column of pixels (384 pixels tall)
// Assuming the total workspace is 640x384, there will be 40 thread groups computing the histogram in parallel.
// The histogram measures logarithmic luminance ranging from 2^-12 up to 2^4.  This should provide a nice window
// where the exposure would range from 2^-4 up to 2^4.

#pragma once
#include"Common.hlsl"
 
#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif

#ifndef HISTOGRAM_COUNT
#define HISTOGRAM_COUNT 256
#endif

Texture2D<uint> luma : register(t0);
RWByteAddressBuffer histogram : register(u0);
 
cbuffer cbPass : register(b0)
{
	uint kBufferHeight; 
};

groupshared uint groupTileHistogram[HISTOGRAM_COUNT];

[numthreads(DIMX, DIMY, 1)]
void CreateHistgram(uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{ 
	groupTileHistogram[groupIndex] = 0;
	GroupMemoryBarrierWithGroupSync();
	 
	for (uint2 st = dispatchThreadID.xy; st.y < kBufferHeight; st.y += DIMY)
	{ 
		uint quantizedLogLuma = luma[st];
		InterlockedAdd(groupTileHistogram[quantizedLogLuma], 1);
	}
	GroupMemoryBarrierWithGroupSync();
	uint temp; 
	histogram.InterlockedAdd(groupIndex * 4, groupTileHistogram[groupIndex], temp);
}
