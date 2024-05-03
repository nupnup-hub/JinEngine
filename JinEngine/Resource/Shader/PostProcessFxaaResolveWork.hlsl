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
// Used with FXAA to resolve the lengths of the two work queues and to generate DispatchIndirect parameters.
// The work queues are also padded out to a multiple of 64 with dummy work items.
//
#include"PostProcessFxaaCommon.hlsl"
RWByteAddressBuffer indirectParams : register(u0);
RWByteAddressBuffer workQueue : register(u1);
RWByteAddressBuffer workCounts : register(u2);
 
#ifndef DIMX
#define DIMX 64
#endif

[numthreads(DIMX, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex, uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint2 pixelCounts = workCounts.Load2(0);

    // Write out padding to the buffer
	uint pixelCountH = pixelCounts.x;
	uint paddedCountH = (pixelCountH + 63) & ~63;
	if (groupIndex + pixelCountH < paddedCountH)
		workQueue.Store((pixelCountH + groupIndex) * 4, 0xffffffff);

    // Write out padding to the buffer
	uint pixelCountV = pixelCounts.y;
	uint paddedCountV = (pixelCountV + 63) & ~63;
	if (groupIndex + pixelCountV < paddedCountV)
		workQueue.Store((lastQueueIndex - pixelCountV - groupIndex) * 4, 0xffffffff);

	DeviceMemoryBarrierWithGroupSync();

	if (groupIndex == 0)
	{
		indirectParams.Store(0, paddedCountH >> 6);
		indirectParams.Store(12, paddedCountV >> 6);
		workCounts.Store2(0, 0);
	}
}
