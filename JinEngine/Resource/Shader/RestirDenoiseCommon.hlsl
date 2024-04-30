#pragma once 
#include"GBufferCommon.hlsl"
#include"FilterCommon.hlsl"
#include"ColorSpaceUtility.hlsl"

#define MAX_FAST_FRAME_ACCMURATION 6
#define MAX_FRAME_ACCMURATION 30 
#define FIXED_FRAME_COUNT 4

struct ReserviorDenoiseConstants
{
    float4x4 camInvView;
    float4x4 camPreViewProj;
    float2 rtSize;
    float2 invRtSize;
    float2 uvToViewA;
    float2 uvToViewB;
    float2 camNearFar;
    float camNearMulFar;
    float denoiseRange; 
    float baseRadius;
    float radiusRange;
    uint sampleNumber;
    uint pad00; 
};
ConstantBuffer<ReserviorDenoiseConstants> cb : register(b0);
 

//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
 
uint GetIndexOfValueClosestToTheReference(in float refValue, in float2 vValues)
{
    float2 delta = abs(refValue - vValues);

    uint outIndex = delta[1] < delta[0] ? 1 : 0;

    return outIndex;
}
uint GetIndexOfValueClosestToTheReference(in float refValue, in float4 vValues)
{
    float4 delta = abs(refValue - vValues);

    uint outIndex = delta[1] < delta[0] ? 1 : 0;
    outIndex = delta[2] < delta[outIndex] ? 2 : outIndex;
    outIndex = delta[3] < delta[outIndex] ? 3 : outIndex;

    return outIndex;
}
// Remap partial depth derivatives at z0 from [1,1] pixel offset to a new pixel offset.
float2 RemapDdxy(in float z0, in float2 ddxy, in float2 pixelOffset)
{
    // Perspective correction for non-linear depth interpolation.
    // Ref: https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/visibility-problem-depth-buffer-depth-interpolation
    // Given a linear depth interpolation for finding z at offset q along z0 to z1
    //      z =  1 / (1 / z0 * (1 - q) + 1 / z1 * q)
    // and z1 = z0 + ddxy, where z1 is at a unit pixel offset [1, 1]
    // z can be calculated via ddxy as
    //
    //      z = (z0 + ddxy) / (1 + (1-q) / z0 * ddxy) 
    float2 z = (z0 + ddxy) / (1 + ((1 - pixelOffset) / z0) * ddxy);
    return sign(pixelOffset) * (z - z0);
}

uint SmallestPowerOf2GreaterThan(in uint x)
{
    // Set all the bits behind the most significant non-zero bit in x to 1.
    // Essentially giving us the largest value that is smaller than the
    // next power of 2 we're looking for.
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

    // Return the next power of two value.
    return x + 1;
}
// Returns float precision for a given float value.
// Values within (value -precision, value + precision) map to the same value. 
// Precision = exponentRange/MaxMantissaValue = (2^e+1 - 2^e) / (2^numMantissaBits)
// Ref: https://blog.demofox.org/2017/11/21/floating-point-precision/
float FloatPrecision(in float x, in uint numMantissaBits)
{
    // Find the exponent range the value is in.
    uint nextPowerOfTwo = SmallestPowerOf2GreaterThan(x);
    float exponentRange = nextPowerOfTwo - (nextPowerOfTwo >> 1);

    float MaxMantissaValue = (float) ((uint) 1 << numMantissaBits);

    return exponentRange / MaxMantissaValue;
}
float ComputeColorVariance(float3 color)
{
    float2 moments;
    moments.r = RGBToLuminance(color);
    moments.g = moments.r * moments.r;
    
    return max(0.0f, moments.g - moments.r * moments.r);
}
 
/*

 // N = surface normal for the current frame
 // Nprev = surface normal for the previous frame
 // prevViewZ = viewZ in the 2x2 footprint for the previous frame
float4 ComputeDisocclusion2x2(float threshold, float isInScreen, float3 Xprev, float3 normal, float3 preNormal, float invDistToPoint, float4 prevViewZ)
{
    threshold = lerp(-1.0, threshold, isInScreen);
    float NoXprev1 = abs(dot(Xprev, normal));
    float NoXprev2 = abs(dot(Xprev, preNormal));
    float NoXprev = max(NoXprev1, NoXprev2) * invDistToPoint;
    float Zprev = mul(mWorldToViewPrev, float4(Xprev, 1.0)).z;
    float NoVprev = NoXprev / abs(Zprev);
    float4 relativePlaneDist = abs(NoVprev * abs(prevViewZ) - NoXprev);
    return step(relativePlaneDist, threshold);
}
*/

