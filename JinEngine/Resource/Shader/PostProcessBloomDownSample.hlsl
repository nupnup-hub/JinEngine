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
// The CS for downsampling 16x16 blocks of pixels down to 4x4 and 1x1 blocks.

#include "PostProcessingCommon.hlsl"

#ifdef  DOWN_SAMPLE_4
#define SAMPLE_COUNT 4
#else  
#define SAMPLE_COUNT 2
#endif

Texture2D<float3> bloom : register(t0); 
RWTexture2D<float3> result0 : register(u0);
RWTexture2D<float3> result1 : register(u1);
#ifdef DOWN_SAMPLE_4
RWTexture2D<float3> result2 : register(u2);
RWTexture2D<float3> result3 : register(u3);
#endif
SamplerState samLinearClmap : register(s0);

cbuffer cb0 : register(b0)
{
	float2 inverseDim;
};

#ifndef DIMX
#define DIMX 8
#endif
#ifndef DIMY
#define DIMY 8
#endif
groupshared float3 gTile[DIMX * DIMY]; // 8x8 input pixels
 
[numthreads(DIMX, DIMY, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // You can tell if both x and y are divisible by a power of two with this value
	uint parity = dispatchThreadID.x | dispatchThreadID.y;

    // Store the first downsampled quad per thread
	float2 centerUV = (float2(dispatchThreadID.xy) * 2.0f + 1.0f) * inverseDim;
	float3 avgPixel = bloom.SampleLevel(samLinearClmap, centerUV, 0.0f);
#ifdef DOWN_SAMPLE_4
	gTile[groupIndex] = avgPixel;
	result0[dispatchThreadID.xy] = avgPixel;

	GroupMemoryBarrierWithGroupSync();

    // Downsample and store the 4x4 block
	if ((parity & 1) == 0)
	{
		avgPixel = 0.25f * (avgPixel + gTile[groupIndex + 1] + gTile[groupIndex + 8] + gTile[groupIndex + 9]);
		gTile[groupIndex] = avgPixel;
		result1[dispatchThreadID.xy >> 1] = avgPixel;
	}

	GroupMemoryBarrierWithGroupSync();

    // Downsample and store the 2x2 block
	if ((parity & 3) == 0)
	{
		avgPixel = 0.25f * (avgPixel + gTile[groupIndex + 2] + gTile[groupIndex + 16] + gTile[groupIndex + 18]);
		gTile[groupIndex] = avgPixel;
		result2[dispatchThreadID.xy >> 2] = avgPixel;
	}

	GroupMemoryBarrierWithGroupSync();

    // Downsample and store the 1x1 block
	if ((parity & 7) == 0)
	{
		avgPixel = 0.25f * (avgPixel + gTile[groupIndex + 4] + gTile[groupIndex + 32] + gTile[groupIndex + 36]);
		result3[dispatchThreadID.xy >> 3] = avgPixel;
	}
#else  
	gTile[groupIndex] = avgPixel;
	GroupMemoryBarrierWithGroupSync();

	if ((parity & 1) == 0)
	{
		avgPixel = 0.25f * (avgPixel + gTile[groupIndex + 1] + gTile[groupIndex + 8] + gTile[groupIndex + 9]);
		gTile[groupIndex] = avgPixel;
		result0[dispatchThreadID.xy >> 1] = avgPixel;
	}

	GroupMemoryBarrierWithGroupSync();

	if ((parity & 3) == 0)
	{
		avgPixel = avgPixel + gTile[groupIndex + 2] + gTile[groupIndex + 16] + gTile[groupIndex + 18];
		gTile[groupIndex] = avgPixel;
	}

	GroupMemoryBarrierWithGroupSync();

	if ((parity & 7) == 0)
	{
		avgPixel = 0.0625f * (avgPixel + gTile[groupIndex + 4] + gTile[groupIndex + 32] + gTile[groupIndex + 36]);
		result1[dispatchThreadID.xy >> 3] = avgPixel;
	}
#endif
}
