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

#pragma once
#include"FxaaCommon.hlsl"

Texture2D<float> luma : register(t0);
ByteAddressBuffer workQueue : register(t1);
Buffer<float3> colorQueue : register(t2);
RWTexture2D<float3> dstColor : register(u0);
SamplerState samLinearClamp : register(s0);


// Note that the number of samples in each direction is one less than the number of sample distances.  The last
// is the maximum distance that should be used, but whether that sample is "good" or "bad" doesn't affect the result,
// so we don't need to load it.

#define FXAA_EXTREME_QUALITY
#ifdef FXAA_EXTREME_QUALITY
#define NUM_SAMPLES 11
    static const float s_SampleDistances[12] =	// FXAA_QUALITY__PRESET == 39
    {
        1.0, 2.0, 3.0, 4.0, 5.0, 6.5, 8.5, 10.5, 12.5, 14.5, 18.5, 36.5, 
    };
#else
#define NUM_SAMPLES 7
static const float s_SampleDistances[8] = // FXAA_QUALITY__PRESET == 25
{
	1.0, 2.5, 4.5, 6.5, 8.5, 10.5, 14.5, 22.5
};
#endif

#ifndef DIMX
#define DIMX 64
#endif

[numthreads(DIMX, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex, uint3 groupThreadIndex : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
#ifdef VERTICAL_ORIENTATION
    uint itemIdx = lastQueueIndex - dispatchThreadID.x;
#else
	uint itemIdx = dispatchThreadID.x;
#endif
	uint workHeader = workQueue.Load(itemIdx * 4);
	uint2 st = startPixel + (uint2(workHeader >> 8, workHeader >> 20) & 0xFFF);
	uint gradientDir = workHeader & 1; // Determines which side of the pixel has the highest contrast
	float subpix = (workHeader & 0xFE) / 254.0f * 0.5f; // 7-bits to encode [0, 0.5]

#ifdef VERTICAL_ORIENTATION
    float nextLuma = luma[st + int2(gradientDir * 2 - 1, 0)];
    float2 startUV = (st + float2(gradientDir, 0.5f)) * rcpTextureSize;
#else
	float nextLuma = luma[st + int2(0, gradientDir * 2 - 1)];
	float2 startUV = (st + float2(0.5f, gradientDir)) * rcpTextureSize;
#endif
	float thisLuma = luma[st];
	float centerLuma = (nextLuma + thisLuma) * 0.5f; // Halfway between this and next; center of the contrasting edge
	float gradientSgn = sign(nextLuma - thisLuma); // Going down in brightness or up?
	float gradientMag = abs(nextLuma - thisLuma) * 0.25f; // How much contrast?  When can we stop looking?

	float negDist = s_SampleDistances[NUM_SAMPLES];
	float posDist = s_SampleDistances[NUM_SAMPLES];
	bool negGood = false;
	bool posGood = false;

	for (uint iter = 0; iter < NUM_SAMPLES; ++iter)
	{
		const float distance = s_SampleDistances[iter];

#ifdef VERTICAL_ORIENTATION
        float2 negUV = startUV - float2(0, rcpTextureSize.y) * distance;
        float2 posUV = startUV + float2(0, rcpTextureSize.y) * distance;
#else
		float2 negUV = startUV - float2(rcpTextureSize.x, 0) * distance;
		float2 posUV = startUV + float2(rcpTextureSize.x, 0) * distance;
#endif

        // Check for a negative endpoint
		float negGrad = luma.SampleLevel(samLinearClamp, negUV, 0) - centerLuma;
		if (abs(negGrad) >= gradientMag && distance < negDist)
		{
			negDist = distance;
			negGood = sign(negGrad) == gradientSgn;
		}

        // Check for a positive endpoint
		float posGrad = luma.SampleLevel(samLinearClamp, posUV, 0) - centerLuma;
		if (abs(posGrad) >= gradientMag && distance < posDist)
		{
			posDist = distance;
			posGood = sign(posGrad) == gradientSgn;
		}
	}

    // Ranges from 0.0 to 0.5
	float pixelShift = 0.5f - min(negDist, posDist) / (posDist + negDist);
	bool goodSpan = negDist < posDist ? negGood : posGood;
	pixelShift = max(subpix, goodSpan ? pixelShift : 0.0f);

	if (pixelShift > 0.01f)
	{
#ifdef DEBUG_OUTPUT
		dstColor[st] = float3(2.0 * pixelShift, 1.0 - 2.0 * pixelShift, 0);
#else
		dstColor[st] = lerp(dstColor[st], colorQueue[itemIdx], pixelShift);
#endif
	}
#ifdef DEBUG_OUTPUT
    else
    {
		dstColor[st] = float3(0, 0, 0.25f);
    }
#endif
}
