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
// The CS for extracting bright pixels and downsampling them to an unblurred bloom buffer.

#include "PostProcessingCommon.hlsl" 

Texture2D<float3> sourceTex : register(t0);
StructuredBuffer<float> exposure : register(t1);
RWTexture2D<float3> bloomResult : register(u0);
#ifdef EXTRACT_LUMA
RWTexture2D<uint> lumaResult : register(u1);
#endif
SamplerState samLinearClmap : register(s0);

cbuffer cb0 : register(b0)
{
	float2 inverseOutSize;
	float bloomThreshold;
};
 
#ifndef DIMX
#define DIMX 8
#endif
#ifndef DIMY
#define DIMY 8
#endif

[numthreads(DIMX, DIMY, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // We need the scale factor and the size of one pixel so that our four samples are right in the middle
    // of the quadrant they are covering.
	float2 uv = (dispatchThreadID.xy + 0.5) * inverseOutSize;
	float2 offset = inverseOutSize * 0.25;

    // Use 4 bilinear samples to guarantee we don't undersample when downsizing by more than 2x
	float3 color1 = sourceTex.SampleLevel(samLinearClmap, uv + float2(-offset.x, -offset.y), 0);
	float3 color2 = sourceTex.SampleLevel(samLinearClmap, uv + float2(offset.x, -offset.y), 0);
	float3 color3 = sourceTex.SampleLevel(samLinearClmap, uv + float2(-offset.x, offset.y), 0);
	float3 color4 = sourceTex.SampleLevel(samLinearClmap, uv + float2(offset.x, offset.y), 0);

	float luma1 = RGBToLuminance(color1);
	float luma2 = RGBToLuminance(color2);
	float luma3 = RGBToLuminance(color3);
	float luma4 = RGBToLuminance(color4);

	const float kSmallEpsilon = 0.0001f;

	float ScaledThreshold = bloomThreshold * exposure[1]; // BloomThreshold / exposure

    // We perform a brightness filter pass, where lone bright pixels will contribute less.
	color1 *= max(kSmallEpsilon, luma1 - ScaledThreshold) / (luma1 + kSmallEpsilon);
	color2 *= max(kSmallEpsilon, luma2 - ScaledThreshold) / (luma2 + kSmallEpsilon);
	color3 *= max(kSmallEpsilon, luma3 - ScaledThreshold) / (luma3 + kSmallEpsilon);
	color4 *= max(kSmallEpsilon, luma4 - ScaledThreshold) / (luma4 + kSmallEpsilon);

    // The shimmer filter helps remove stray bright pixels from the bloom buffer by inversely weighting
    // them by their luminance.  The overall effect is to shrink bright pixel regions around the border.
    // Lone pixels are likely to dissolve completely.  This effect can be tuned by adjusting the shimmer
    // filter inverse strength.  The bigger it is, the less a pixel's luminance will matter.
	const float kShimmerFilterInverseStrength = 1.0f;
	float weight1 = 1.0f / (luma1 + kShimmerFilterInverseStrength);
	float weight2 = 1.0f / (luma2 + kShimmerFilterInverseStrength);
	float weight3 = 1.0f / (luma3 + kShimmerFilterInverseStrength);
	float weight4 = 1.0f / (luma4 + kShimmerFilterInverseStrength);
	float weightSum = weight1 + weight2 + weight3 + weight4;

	bloomResult[dispatchThreadID.xy] = (color1 * weight1 + color2 * weight2 + color3 * weight3 + color4 * weight4) / weightSum;
#ifdef EXTRACT_LUMA
	float luma = (luma1 + luma2 + luma3 + luma4) * 0.25;

    // Prevent log(0) and put only pure black pixels in Histogram[0]
	if (luma == 0.0)
	{
		lumaResult[dispatchThreadID.xy] = 0;
	}
	else
	{
		const float MinLog = exposure[4];
		const float RcpLogRange = exposure[7];
		float logLuma = saturate((log2(luma) - MinLog) * RcpLogRange); // Rescale to [0.0, 1.0]
		lumaResult[dispatchThreadID.xy] = logLuma * 254.0 + 1.0; // Rescale to [1, 255]
	}
#endif
}
