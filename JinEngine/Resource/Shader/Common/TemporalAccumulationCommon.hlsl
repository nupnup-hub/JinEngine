/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"ColorSpaceUtility.hlsl"
#include"GBufferCommon.hlsl"
#include"FilterCommon.hlsl"
#include"PixelCommon.hlsl"   
#include"Sampling.hlsl"

#ifndef MAX_FAST_FRAME_ACCMURATION
#define MAX_FAST_FRAME_ACCMURATION 6
#endif

#ifndef MAX_FRAME_ACCMURATION
#define MAX_FRAME_ACCMURATION 60
#endif 

#ifndef FIXED_FRAME_COUNT
#define FIXED_FRAME_COUNT 4
#endif 
 
uint GetIndexOfValueClosestToTheReference(const float refValue, const float2 vValues)
{
    float2 delta = abs(refValue - vValues);
    uint outIndex = delta[1] < delta[0] ? 1 : 0;
    return outIndex;
}
uint GetIndexOfValueClosestToTheReference(const float refValue, const float4 vValues)
{
    float4 delta = abs(refValue - vValues);

    uint outIndex = delta[1] < delta[0] ? 1 : 0;
    outIndex = delta[2] < delta[outIndex] ? 2 : outIndex;
    outIndex = delta[3] < delta[outIndex] ? 3 : outIndex;

    return outIndex;
}
float ComputeColorVariance(const float3 color)
{
    float2 moments;
    moments.r = RGBToLuminance(color);
    moments.g = moments.r * moments.r;
    
    return max(0.0f, moments.g - moments.r * moments.r);
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

float AccumSpeed(const float currHistoryLength)
{
#if 1
    return 1.0f / min(currHistoryLength, MAX_FRAME_ACCMURATION);
#else 
    return 1.0f / min(currHistoryLength + 1.0f, MAX_FRAME_ACCMURATION);
#endif
}
float4 AccumSpeed4(const float4 currHistoryLength)
{
    return float4(AccumSpeed(currHistoryLength.x), AccumSpeed(currHistoryLength.y), AccumSpeed(currHistoryLength.z), AccumSpeed(currHistoryLength.w));
}
float FastAccumSpeed(const float currHistoryLength)
{
#if 1
    return 1.0f / min(currHistoryLength, MAX_FAST_FRAME_ACCMURATION);
#else 
    return 1.0f / min(currHistoryLength + 1.0f, MAX_FAST_FRAME_ACCMURATION);
#endif
}
 
namespace TA
{
    struct Result
    {
        //bilinear weight
        CustomSampling::BilinearParameter bilinearParameter;
        float4 customWeight;
        int passCount;
        bool canUseCubic;
        bool canUseBilinear;
    }; 
    class Actor
    {
        float2 preUv;
        float3 preCenterPosW;
        float3 centerNormal;
        float3 preCenterNormal;
        float preCenterViewZ;
        
        float2 rtSize;
        float2 invRtSize;
        
        float invDistToPoint;      
        float disocclusionThreshold;
        float normalThresHold;
         
        Texture2D<float> preViewZMap;
        Texture2D preNormalMap;
        SamplerState samPointClamp;
        SamplerState samLinearClmap;
         
        bool IsValidNormal(const float3 normal, const float3 preNormal)
        {
            return dot(normal, preNormal) > normalThresHold;
        }
        
        void Initialze(float2 _preUv,
            float3 _preCenterPosW,
            float3 _centerNormal,
            float3 _preCenterNormal,
            float _preCenterViewZ,
            float2 _rtSize,
            float2 _invRtSize,
            float _invDistToPoint,
            float _disocclusionThreshold,
            float _normalThresHold,
            Texture2D<float> _preViewZMap,
            Texture2D _preNormalMap,
            SamplerState _samPointClamp,
            SamplerState _samLinearClmap)
        {
            preUv = _preUv;
            preCenterPosW = _preCenterPosW;
            preCenterNormal = _preCenterNormal;
            centerNormal = _centerNormal;
            preCenterViewZ = _preCenterViewZ;
            
            rtSize = _rtSize;
            invRtSize = _invRtSize;
            invDistToPoint = _invDistToPoint;
            disocclusionThreshold = _disocclusionThreshold;
            normalThresHold = _normalThresHold;
            
            preViewZMap = _preViewZMap;
            preNormalMap = _preNormalMap;
            samPointClamp = _samPointClamp;
            samLinearClmap = _samLinearClmap;          
        } 
        void Compute(inout Result result)
        {
            // bc - bicubic tap,
            // bl - bicubic & bilinear tap
            //
            // -- bc bc --      Gather 
            // bc bl bl bc      w  z 
            // bc bl bl bc      x  y
            // -- bc bc --
 
            //  CatRom12     => Bilinear
            //    0y 1x
            // 0z 0w 1z 1w       0z 1y
            // 2x 2y 3x 3y       2y 3x
            //    2w 3z
            // Gather sample ordering: (-,+),(+,+),(+,-),(-,-),
            
            Catmul::Parameter catmulParam;
            catmulParam.Initialize(preUv, rtSize, invRtSize);
 
            /*
                Since the catmulParam origin is located at the center,
                it is necessary to subtract 0.5f in order to perform the GatherRed operation in Cubic 
            */
            float2 origin = catmulParam.origin - 0.5f;
            float4 preViewZ00 = preViewZMap.GatherRed(samPointClamp, origin, float2(0.0f, 0.0f)).wzxy;
            float4 preViewZ10 = preViewZMap.GatherRed(samPointClamp, origin, float2(2.0f, 0.0f)).wzxy;
            float4 preViewZ01 = preViewZMap.GatherRed(samPointClamp, origin, float2(0.0f, 2.0f)).wzxy;
            float4 preViewZ11 = preViewZMap.GatherRed(samPointClamp, origin, float2(2.0f, 2.0f)).wzxy;
     
            float distanceA = abs(dot(preCenterPosW, centerNormal));
            float distanceB = abs(dot(preCenterPosW, preCenterNormal));
            float distanceRate = max(distanceA, distanceB) * invDistToPoint;
            
            float centerDistanceRate = distanceRate / preCenterViewZ;
             
            float3 relativePlaneDist00 = abs(centerDistanceRate * preViewZ00.yzw - distanceRate);
            float3 relativePlaneDist10 = abs(centerDistanceRate * preViewZ10.xzw - distanceRate);
            float3 relativePlaneDist01 = abs(centerDistanceRate * preViewZ01.xyw - distanceRate);
            float3 relativePlaneDist11 = abs(centerDistanceRate * preViewZ11.xyz - distanceRate);
              
            //float disocclusionThreshold = 1.0f / (camFar - camNear);
            float3 disocclusion00 = step(relativePlaneDist00, disocclusionThreshold);
            float3 disocclusion10 = step(relativePlaneDist10, disocclusionThreshold);
            float3 disocclusion01 = step(relativePlaneDist01, disocclusionThreshold);
            float3 disocclusion11 = step(relativePlaneDist11, disocclusionThreshold);
            float4 bilinearOcclusion = float4(disocclusion00.z, disocclusion10.y, disocclusion01.y, disocclusion11.x);
            
            uint viewTestPassCount = dot(disocclusion00, disocclusion10) * dot(disocclusion01, disocclusion11);
            result.passCount += viewTestPassCount;
            result.canUseCubic = viewTestPassCount == 9;
            result.canUseBilinear = any(bilinearOcclusion > 0);
    
            /*
             float2 offset = float2(1, 1) * invRtSize;
            float3 preNormalCenter = UnpackNormal(preNormalMap.SampleLevel(samLinearClmap, preUv, 0));
            float3 preNormalLeftUp = UnpackNormal(preNormalMap.SampleLevel(samLinearClmap, preUv + float2(-offset.x, -offset.y), 0));
            float3 preNormalLeftDown = UnpackNormal(preNormalMap.SampleLevel(samLinearClmap, preUv + float2(-offset.x, offset.y), 0));
            float3 preNormalRightUp = UnpackNormal(preNormalMap.SampleLevel(samLinearClmap, preUv + float2(offset.x, -offset.y), 0));
            float3 preNormalRightDown = UnpackNormal(preNormalMap.SampleLevel(samLinearClmap, preUv + float2(offset.x, offset.y), 0));
    
            uint normalTestPassCount = 0;
            normalTestPassCount += IsValidNormal(centerNormal, preNormalCenter);
            normalTestPassCount += IsValidNormal(centerNormal, preNormalLeftUp);
            normalTestPassCount += IsValidNormal(centerNormal, preNormalLeftDown);
            normalTestPassCount += IsValidNormal(centerNormal, preNormalRightUp);
            normalTestPassCount += IsValidNormal(centerNormal, preNormalRightDown);
            
            result.passCount += normalTestPassCount;
            result.canUseCubic &= normalTestPassCount == 5;
            result.canUseBilinear |= normalTestPassCount > 0;
            */
            
            result.bilinearParameter = CustomSampling::GetBilinearFilter(preUv, rtSize);
            result.customWeight = CustomSampling::GetBilinearCustomWeights(result.bilinearParameter, bilinearOcclusion);
            result.customWeight *= result.customWeight;
        }
    };
  
    void ComputeCubicWeight(in Actor actor, out Result result)
    {
        result.customWeight = float4(0, 0, 0, 0);
        result.passCount = 0;
        result.canUseCubic = false;
        result.canUseBilinear = false;
      
        bool isValidCoord = IsValidUv(actor.preUv);
        if (!isValidCoord)
            return;
    
        actor.Compute(result);
    }

}