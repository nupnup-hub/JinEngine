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
// Description:  A Compute-optimized implementation of FXAA 3.11 (PC Quality).  The
// improvements take advantage of work queues (RWStructuredBuffer with atomic counters)
// for these benefits:
// 
// 1) Split horizontal and vertical edge searches into separate dispatches to reduce
// shader complexity and incoherent branching.
// 2) Delay writing new pixel colors until after the source buffer has been fully
// analyzed.  This avoids the write-after-scattered-read hazard.
// 3) Modify source buffer in-place rather than ping-ponging buffers, which reduces
// bandwidth and memory demands.
//
// In addition to the above-mentioned benefits of using UAVs, the first pass also
// takes advantage of groupshared memory for storing luma values, further reducing
// fetches and bandwidth.
//
// Another optimization is in the generation of perceived brightness (luma) of pixels.
// The original implementation used sRGB as a good approximation of log-luminance.  A
// more precise representation of log-luminance allows the algorithm to operate with a
// higher threshold value while still finding perceivable edges across the full range
// of brightness.  The approximation used here is (1 - 2^(-4L)) * 16/15, where L =
// dot( LinearRGB, float3(0.212671, 0.715160, 0.072169) ).  A threshold of 0.2 is
// recommended with log-luminance computed this way.
//

// Original Boilerplate:
//
/*============================================================================


                    NVIDIA FXAA 3.11 by TIMOTHY LOTTES


------------------------------------------------------------------------------
COPYRIGHT (C) 2010, 2011 NVIDIA CORPORATION. ALL RIGHTS RESERVED.
------------------------------------------------------------------------------
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR
LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION,
OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE
THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
DAMAGES.
*/


#include"PostProcessFxaaCommon.hlsl"

Texture2D<float3> color : register(t0);
RWByteAddressBuffer workCount : register(u0);
RWByteAddressBuffer workQueue : register(u1);
RWBuffer<float3> colorQueue : register(u2);
SamplerState samLinearClamp : register(s0);

#define BOUNDARY_SIZE 1
#define ROW_WIDTH (8 + BOUNDARY_SIZE * 2)
groupshared float gLumaCache[ROW_WIDTH * ROW_WIDTH];

// If pre-computed, source luminance as a texture, otherwise write it out for Pass2
#ifdef USE_LUMA_INPUT_BUFFER
Texture2D<float> luma : register(t1);
#else
RWTexture2D<float> luma : register(u3);
#endif

//
// Helper functions
//
float RGBToLogLuminance(float3 LinearRGB)
{
	float luma = dot(LinearRGB, float3(0.212671f, 0.715160f, 0.072169f));
	return log2(1 + luma * 15) / 4;
}
float3 FetchColor(int2 st)
{
	return color[st];
}
 
#ifndef DIMX
#define DIMX 8
#endif
#ifndef DIMY
#define DIMY 8
#endif

[numthreads(DIMX, DIMY, 1)]
void main(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex, uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint2 pixelCoord = dispatchThreadID.xy + startPixel;

#ifdef USE_LUMA_INPUT_BUFFER
    // Load 4 lumas per thread into LDS (but only those needed to fill our pixel cache)
    if (max(groupThreadID.x, groupThreadID.y) < ROW_WIDTH / 2)
    {
        int2 threadUL = pixelCoord + groupThreadID.xy - (BOUNDARY_SIZE - 1);
        float4 luma4 = luma.Gather(samLinearClamp, threadUL * rcpTextureSize);
        uint loadIndex = (groupThreadID.x + groupThreadID.y * ROW_WIDTH) * 2;
        gLumaCache[loadIndex                ] = luma4.w;
        gLumaCache[loadIndex + 1            ] = luma4.z;
        gLumaCache[loadIndex + ROW_WIDTH    ] = luma4.x;
        gLumaCache[loadIndex + ROW_WIDTH + 1] = luma4.y;
    }
#else
    // Because we can't use Gather() on RGB, we make each thread read two pixels (but only those needed).
	if (groupIndex < ROW_WIDTH * ROW_WIDTH / 2)
	{
		uint loadIndex = groupIndex;
		int2 uavCoord = startPixel + uint2(groupIndex % ROW_WIDTH, groupIndex / ROW_WIDTH) + groupID.xy * 8 - BOUNDARY_SIZE;
		float luma1 = RGBToLogLuminance(FetchColor(uavCoord));
		luma[uavCoord] = luma1;
		gLumaCache[loadIndex] = luma1;

		loadIndex += ROW_WIDTH * ROW_WIDTH / 2;
		uavCoord += int2(0, ROW_WIDTH / 2);
		float Luma2 = RGBToLogLuminance(FetchColor(uavCoord));
		luma[uavCoord] = Luma2;
		gLumaCache[loadIndex] = Luma2;
	}
#endif

	GroupMemoryBarrierWithGroupSync();

	uint CenterIdx = (groupThreadID.x + BOUNDARY_SIZE) + (groupThreadID.y + BOUNDARY_SIZE) * ROW_WIDTH;

    // Load the ordinal and center luminances
	float lumaN = gLumaCache[CenterIdx - ROW_WIDTH];
	float lumaW = gLumaCache[CenterIdx - 1];
	float lumaM = gLumaCache[CenterIdx];
	float lumaE = gLumaCache[CenterIdx + 1];
	float lumaS = gLumaCache[CenterIdx + ROW_WIDTH];

    // Contrast threshold test
	float rangeMax = max(max(lumaN, lumaW), max(lumaE, max(lumaS, lumaM)));
	float rangeMin = min(min(lumaN, lumaW), min(lumaE, min(lumaS, lumaM)));
	float range = rangeMax - rangeMin;
	if (range < contrastThreshold)
		return;

    // Load the corner luminances
	float lumaNW = gLumaCache[CenterIdx - ROW_WIDTH - 1];
	float lumaNE = gLumaCache[CenterIdx - ROW_WIDTH + 1];
	float lumaSW = gLumaCache[CenterIdx + ROW_WIDTH - 1];
	float lumaSE = gLumaCache[CenterIdx + ROW_WIDTH + 1];

    // Pre-sum a few terms so the results can be reused
	float lumaNS = lumaN + lumaS;
	float lumaWE = lumaW + lumaE;
	float lumaNWSW = lumaNW + lumaSW;
	float lumaNESE = lumaNE + lumaSE;
	float lumaSWSE = lumaSW + lumaSE;
	float lumaNWNE = lumaNW + lumaNE;

    // Compute horizontal and vertical contrast; see which is bigger
	float edgeHorz = abs(lumaNWSW - 2.0 * lumaW) + abs(lumaNS - 2.0 * lumaM) * 2.0 + abs(lumaNESE - 2.0 * lumaE);
	float edgeVert = abs(lumaSWSE - 2.0 * lumaS) + abs(lumaWE - 2.0 * lumaM) * 2.0 + abs(lumaNWNE - 2.0 * lumaN);

    // Also compute local contrast in the 3x3 region.  This can identify standalone pixels that alias.
	float avgNeighborLuma = ((lumaNS + lumaWE) * 2.0 + lumaNWSW + lumaNESE) / 12.0;
	float subpixelShift = saturate(pow(smoothstep(0, 1, abs(avgNeighborLuma - lumaM) / range), 2) * subpixelRemoval * 2);

	float negGrad = (edgeHorz >= edgeVert ? lumaN : lumaW) - lumaM;
	float posGrad = (edgeHorz >= edgeVert ? lumaS : lumaE) - lumaM;
	uint gradientDir = abs(posGrad) >= abs(negGrad) ? 1 : 0;
	uint subpix = uint(subpixelShift * 254.0) & 0xFE;

    // Packet header: [ 12 bits Y | 12 bits X | 7 bit Subpix | 1 bit dir(Grad) ]
	uint workHeader = dispatchThreadID.y << 20 | dispatchThreadID.x << 8 | subpix | gradientDir;

	if (edgeHorz >= edgeVert)
	{
		uint workIdx;
		workCount.InterlockedAdd(0, 1, workIdx);
		workQueue.Store(workIdx * 4, workHeader);
		colorQueue[workIdx] = FetchColor(pixelCoord + uint2(0, 2 * gradientDir - 1));
	}
	else
	{
		uint workIdx;
		workCount.InterlockedAdd(4, 1, workIdx);
		workIdx = lastQueueIndex - workIdx;
		workQueue.Store(workIdx * 4, workHeader);
		colorQueue[workIdx] = FetchColor(pixelCoord + uint2(2 * gradientDir - 1, 0));
	}
}
