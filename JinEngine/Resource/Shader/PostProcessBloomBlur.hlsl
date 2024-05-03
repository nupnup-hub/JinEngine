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
// The CS for guassian blurring a single RGB buffer.
//
// For the intended bloom blurring algorithm, this shader is expected to be used only on
// the lowest resolution bloom buffer before starting the series of upsample-and-blur
// passes.

#include "PostProcessingCommon.hlsl"

Texture2D<float3> input : register(t0);
RWTexture2D<float3> output : register(u0);

cbuffer cb0 : register(b0)
{
	float2 inverseDimensions;
};

#ifndef DIMX
#define DIMX 8
#endif
#ifndef DIMY
#define DIMY 8
#endif

// The guassian blur weights (derived from Pascal's triangle)
static const float gWeights[5] = { 70.0f / 256.0f, 56.0f / 256.0f, 28.0f / 256.0f, 8.0f / 256.0f, 1.0f / 256.0f };
float3 BlurPixels(float3 a, float3 b, float3 c, float3 d, float3 e, float3 f, float3 g, float3 h, float3 i)
{
	return gWeights[0] * e + gWeights[1] * (d + f) + gWeights[2] * (c + g) + gWeights[3] * (b + h) + gWeights[4] * (a + i);
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
	uint rr = cacheR[index];
	uint gg = cacheG[index];
	uint bb = cacheB[index];
	pixel1 = float3(f16tof32(rr), f16tof32(gg), f16tof32(bb));
	pixel2 = float3(f16tof32(rr >> 16), f16tof32(gg >> 16), f16tof32(bb >> 16));
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

	output[pixelCoord] = BlurPixels(s0, s1, s2, s3, s4, s5, s6, s7, s8);
}
 
[numthreads(DIMX, DIMY, 1)]
void main(uint3 groupID : SV_GroupID, uint3 grpupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    //
    // Load 4 pixels per thread into LDS
    //
	int2 GroupUL = (groupID.xy << 3) - 4; // Upper-left pixel coordinate of group read location
	int2 ThreadUL = (grpupThreadID.xy << 1) + GroupUL; // Upper-left pixel coordinate of quad that this thread will read

    //
    // Store 4 unblurred pixels in LDS
    //
	int destIdx = grpupThreadID.x + (grpupThreadID.y << 4);
	Store2Pixels(destIdx + 0, input[ThreadUL + uint2(0, 0)], input[ThreadUL + uint2(1, 0)]);
	Store2Pixels(destIdx + 8, input[ThreadUL + uint2(0, 1)], input[ThreadUL + uint2(1, 1)]);

	GroupMemoryBarrierWithGroupSync();

    //
    // Horizontally blur the pixels in Cache
    //
	uint row = grpupThreadID.y << 4;
	BlurHorizontally(row + (grpupThreadID.x << 1), row + grpupThreadID.x + (grpupThreadID.x & 4));

	GroupMemoryBarrierWithGroupSync();

    //
    // Vertically blur the pixels and write the result to memory
    //
	BlurVertically(dispatchThreadID.xy, (grpupThreadID.y << 3) + grpupThreadID.x);
}
