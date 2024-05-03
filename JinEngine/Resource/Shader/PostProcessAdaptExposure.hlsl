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
 
#include "PostProcessingCommon.hlsl"
ByteAddressBuffer histogram : register(t0);
RWStructuredBuffer<float> exposure : register(u0);

cbuffer cbAdaptPass : register(b0)
{
	float4 dataPack; //x = targetLuminance, y = adaptationRate, z = minExposure, w = maxExposure
	uint pixelCount;  
};

#ifndef DIMX
#define DIMX 256
#endif

#define HISTOGRAM_COUNT DIMX

groupshared float gs_Accum[DIMX];
[numthreads(DIMX, 1, 1)]
void AdaptExposure(uint groupIndex : SV_GroupIndex)
{
	float targetLuminance = dataPack.x;
	float adaptationRate = dataPack.y;
	float minExposure = dataPack.z;
	float maxExposure = dataPack.w;
	float weightedSum = (float)groupIndex * (float)histogram.Load(groupIndex * 4);

    [unroll]
	for (uint i = 1; i < HISTOGRAM_COUNT; i *= 2)
	{
		gs_Accum[groupIndex] = weightedSum; // Write
		GroupMemoryBarrierWithGroupSync(); // Sync
		weightedSum += gs_Accum[(groupIndex + i) % HISTOGRAM_COUNT]; // Read
		GroupMemoryBarrierWithGroupSync(); // Sync
	}

    // If the entire image is black, don't adjust exposure
	if (weightedSum == 0.0)
		return;

	float minLog = exposure[4];
	float maxLog = exposure[5];
	float logRange = exposure[6];
	float rcpLogRange = exposure[7];

    // Average histogram value is the weighted sum of all pixels divided by the total number of pixels
    // minus those pixels which provided no weight (i.e. black pixels.)
	float weightedHistAvg = weightedSum / (max(1, pixelCount - histogram.Load(0))) - 1.0f;
	float logAvgLuminance = exp2(weightedHistAvg / 254.0f * logRange + minLog);
	float targetExposure = targetLuminance / logAvgLuminance;
    //float targetExposure = -log2(1 - targetLuminance) / logAvgLuminance;

	float ex = exposure[0];
	ex = lerp(ex, targetExposure, adaptationRate);
	ex = clamp(ex, minExposure, maxExposure);

	if (groupIndex == 0)
	{
		exposure[0] = ex;
		exposure[1] = 1.0f / ex;
		exposure[2] = ex;
		exposure[3] = weightedHistAvg;

        // First attempt to recenter our histogram around the log-average.
		float biasToCenter = (floor(weightedHistAvg) - 128.0f) / 255.0f;
		if (abs(biasToCenter) > 0.1f)
		{
			minLog += biasToCenter * rcpLogRange;
			maxLog += biasToCenter * rcpLogRange;
		}

        // TODO:  Increase or decrease the log range to better fit the range of values.
        // (Idea) Look at intermediate log-weighted sums for under- or over-represented
        // extreme bounds.  I.e. break the for loop into two pieces to compute the sum of
        // groups of 16, check the groups on each end, then finish the recursive summation.

		exposure[4] = minLog;
		exposure[5] = maxLog;
		exposure[6] = logRange;
		exposure[7] = 1.0 / logRange;
	}
}
