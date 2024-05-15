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
// The CS for combining a lower resolution bloom buffer with a higher resolution buffer
// (via bilinear upsampling) and then guassian blurring the resultant buffer.
//
// For the intended bloom blurring algorithm, it is expected that this shader will be
// used repeatedly to upsample and blur successively higher resolutions until the final
// bloom buffer is the destination.
//

#include "Common.hlsl"

Texture2D<float3> higherResBuf : register(t0);
Texture2D<float3> lowerResBuf : register(t1);
RWTexture2D<float3> result : register(u0);

SamplerState samLinearBorder : register(s1);
cbuffer cb0 : register(b0)
{
	float2 inverseDimensions;
	float upsampleBlendFactor;
};

#define BlurPixels Blur9
#ifndef DIMX
#define DIMX 8
#endif
#ifndef DIMY
#define DIMY 8
#endif

// The guassian blur weights (derived from Pascal's triangle)
static const float gWeights5[3] = { 6.0f / 16.0f, 4.0f / 16.0f, 1.0f / 16.0f };
static const float gWeights7[4] = { 20.0f / 64.0f, 15.0f / 64.0f, 6.0f / 64.0f, 1.0f / 64.0f };
static const float gWeights9[5] = { 70.0f / 256.0f, 56.0f / 256.0f, 28.0f / 256.0f, 8.0f / 256.0f, 1.0f / 256.0f };

float3 Blur5(float3 a, float3 b, float3 c, float3 d, float3 e, float3 f, float3 g, float3 h, float3 i)
{
	return gWeights5[0] * e + gWeights5[1] * (d + f) + gWeights5[2] * (c + g);
}

float3 Blur7(float3 a, float3 b, float3 c, float3 d, float3 e, float3 f, float3 g, float3 h, float3 i)
{
	return gWeights7[0] * e + gWeights7[1] * (d + f) + gWeights7[2] * (c + g) + gWeights7[3] * (b + h);
}

float3 Blur9(float3 a, float3 b, float3 c, float3 d, float3 e, float3 f, float3 g, float3 h, float3 i)
{
	return gWeights9[0] * e + gWeights9[1] * (d + f) + gWeights9[2] * (c + g) + gWeights9[3] * (b + h) + gWeights9[4] * (a + i);
}
 
// 16x16 pixels with an 8x8 center that we will be blurring writing out.  Each uint is two color channels packed together
groupshared uint cacheR[128];
groupshared uint cacheG[128];
groupshared uint cacheB[128];

void Store2Pixels(uint index, float3 pixel1, float3 pixel2)
{
	cacheR[index] = f32tof16(pixel1.r) | f32tof16(pixel2.r) << 16;
	cacheG[index] = f32tof16(pixel1.g) | f32tof16(pixel2.g) << 16;
	cacheB[index] = f32tof16(pixel1.b) | f32tof16(pixel2.b) << 16;
}

void Load2Pixels(uint index, out float3 pixel1, out float3 pixel2)
{
	uint3 RGB = uint3(cacheR[index], cacheG[index], cacheB[index]);
	pixel1 = f16tof32(RGB);
	pixel2 = f16tof32(RGB >> 16);
}

void Store1Pixel(uint index, float3 pixel)
{
	cacheR[index] = asuint(pixel.r);
	cacheG[index] = asuint(pixel.g);
	cacheB[index] = asuint(pixel.b);
}

void Load1Pixel(uint index, out float3 pixel)
{
	pixel = asfloat(uint3(cacheR[index], cacheG[index], cacheB[index]));
}

// Blur two pixels horizontally.  This reduces LDS reads and pixel unpacking.
void BlurHorizontally(uint outIndex, uint leftMostIndex)
{
	float3 s0, s1, s2, s3, s4, s5, s6, s7, s8, s9;
	Load2Pixels(leftMostIndex + 0, s0, s1);
	Load2Pixels(leftMostIndex + 1, s2, s3);
	Load2Pixels(leftMostIndex + 2, s4, s5);
	Load2Pixels(leftMostIndex + 3, s6, s7);
	Load2Pixels(leftMostIndex + 4, s8, s9);
    
	Store1Pixel(outIndex, BlurPixels(s0, s1, s2, s3, s4, s5, s6, s7, s8));
	Store1Pixel(outIndex + 1, BlurPixels(s1, s2, s3, s4, s5, s6, s7, s8, s9));
}

void BlurVertically(uint2 pixelCoord, uint topMostIndex)
{
	float3 s0, s1, s2, s3, s4, s5, s6, s7, s8;
	Load1Pixel(topMostIndex, s0);
	Load1Pixel(topMostIndex + 8, s1);
	Load1Pixel(topMostIndex + 16, s2);
	Load1Pixel(topMostIndex + 24, s3);
	Load1Pixel(topMostIndex + 32, s4);
	Load1Pixel(topMostIndex + 40, s5);
	Load1Pixel(topMostIndex + 48, s6);
	Load1Pixel(topMostIndex + 56, s7);
	Load1Pixel(topMostIndex + 64, s8);

	result[pixelCoord] = BlurPixels(s0, s1, s2, s3, s4, s5, s6, s7, s8);
}
 
[numthreads(DIMX, DIMY, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    //
    // Load 4 pixels per thread into LDS
    //
	int2 GroupUL = (groupID.xy << 3) - 4; // Upper-left pixel coordinate of group read location
	int2 ThreadUL = (groupThreadID.xy << 1) + GroupUL; // Upper-left pixel coordinate of quad that this thread will read

    //
    // Store 4 blended-but-unblurred pixels in LDS
    //
	float2 uvUL = (float2(ThreadUL) + 0.5) * inverseDimensions;
	float2 uvLR = uvUL + inverseDimensions;
	float2 uvUR = float2(uvLR.x, uvUL.y);
	float2 uvLL = float2(uvUL.x, uvLR.y);
	int destIdx = groupThreadID.x + (groupThreadID.y << 4);

	float3 pixel1a = lerp(higherResBuf[ThreadUL + uint2(0, 0)], lowerResBuf.SampleLevel(samLinearBorder, uvUL, 0.0f), upsampleBlendFactor);
	float3 pixel1b = lerp(higherResBuf[ThreadUL + uint2(1, 0)], lowerResBuf.SampleLevel(samLinearBorder, uvUR, 0.0f), upsampleBlendFactor);
	Store2Pixels(destIdx + 0, pixel1a, pixel1b);

	float3 pixel2a = lerp(higherResBuf[ThreadUL + uint2(0, 1)], lowerResBuf.SampleLevel(samLinearBorder, uvLL, 0.0f), upsampleBlendFactor);
	float3 pixel2b = lerp(higherResBuf[ThreadUL + uint2(1, 1)], lowerResBuf.SampleLevel(samLinearBorder, uvLR, 0.0f), upsampleBlendFactor);
	Store2Pixels(destIdx + 8, pixel2a, pixel2b);

	GroupMemoryBarrierWithGroupSync();

    //
    // Horizontally blur the pixels in Cache
    //
	uint row = groupThreadID.y << 4;
	BlurHorizontally(row + (groupThreadID.x << 1), row + groupThreadID.x + (groupThreadID.x & 4));

	GroupMemoryBarrierWithGroupSync();

    //
    // Vertically blur the pixels and write the result to memory
    //
	BlurVertically(dispatchThreadID.xy, (groupThreadID.y << 3) + groupThreadID.x);
}
