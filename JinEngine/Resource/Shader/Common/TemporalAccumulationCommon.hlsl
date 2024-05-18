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

float AccumSpeed(const uint currHistoryLength)
{
#if 1
    return 1.0f / float(min(currHistoryLength, MAX_FRAME_ACCMURATION));
#else 
    return 1.0f / float(min(currHistoryLength + 1.0f, MAX_FRAME_ACCMURATION));
#endif
}
float4 AccumSpeed4(const uint4 currHistoryLength)
{
    return float4(AccumSpeed(currHistoryLength.x), AccumSpeed(currHistoryLength.y), AccumSpeed(currHistoryLength.z), AccumSpeed(currHistoryLength.w));
}
float FastAccumSpeed(const uint currHistoryLength)
{
#if 1
    return 1.0f / float(min(currHistoryLength, MAX_FAST_FRAME_ACCMURATION));
#else 
    return 1.0f / float(min(currHistoryLength + 1.0f, MAX_FAST_FRAME_ACCMURATION));
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
        float3 centerPosW; 
        float3 centerNormal;
        uint centerMaterialID;
        
        float2 rtSize;
        float2 invRtSize;
            
        float invCenterDistToPoint;
        float disocclusionThreshold;
        float normalThresHold;
         
        float3x3 camPreInvView;
        float2 preUvToViewA;
        float2 preUvToViewB;
        
        Texture2D<float> preViewZMap;
        Texture2D preLightPropMap;
        Texture2D preNormalMap;
        SamplerState samPointClamp;
        SamplerState samLinearClmap;
          
        float4 ComputeDisocclusion2x2(float2 sampleCenterUv, float4 prevViewZ)
        {
            float3 sampleCenterNormal = UnpackNormal(preNormalMap.SampleLevel(samLinearClmap, sampleCenterUv, 0));
            if (dot(centerNormal, sampleCenterNormal) <= 0)
                return float4(0, 0, 0, 0); 
            
            float sampleCenterViewZ = preViewZMap.SampleLevel(samLinearClmap, sampleCenterUv, 0).x;
            float3 preSampleCenterPosV = GetViewPos(sampleCenterUv, sampleCenterViewZ, preUvToViewA, preUvToViewB);
            float3 preSampleCenterPosW = mul(preSampleCenterPosV, camPreInvView);

            float sampleDistance00 = abs(dot(preSampleCenterPosW, centerNormal));
            float sampleDistance01 = abs(dot(preSampleCenterPosW, sampleCenterNormal));
            
            //Ratio of distance between sample and center
            float centerDistanceRate = max(sampleDistance00, sampleDistance01) * invCenterDistToPoint;
            
            //Ratio of distance between sample center and sample tabs center
            float4 sampleViewZRate = (1.0f / abs(preSampleCenterPosV.z)) * abs(prevViewZ);
            
            float4 relativePlaneDist = centerDistanceRate * sampleViewZRate - centerDistanceRate;
            //float4 relativePlaneDist = (centerDistanceRate / abs(preSampleCenterPosV.z)) * abs(prevViewZ) - centerDistanceRate;
            return step(relativePlaneDist, disocclusionThreshold);
        }
        bool IsValidNormal(const float3 normal, const float3 preNormal)
        {
            return dot(normal, preNormal) > normalThresHold;
        }
        
        void Initialze(float2 _preUv,
            float3 _centerPosW, 
            float3 _centerNormal, 
            uint _centerMaterialID,
            float2 _rtSize,
            float2 _invRtSize, 
            float _invCenterDistToPoint,
            float _disocclusionThreshold,
            float _normalThresHold,
            float3x3 _camPreInvView,
            float2 _preUvToViewA,
            float2 _preUvToViewB,
            Texture2D<float> _preViewZMap,
            Texture2D _preLightPropMap,
            Texture2D _preNormalMap,
            SamplerState _samPointClamp,
            SamplerState _samLinearClmap)
        {
            preUv = _preUv;
            centerPosW = _centerPosW; 
            centerNormal = _centerNormal; 
            centerMaterialID = _centerMaterialID;
            
            rtSize = _rtSize;
            invRtSize = _invRtSize; 
            invCenterDistToPoint = _invCenterDistToPoint;
            disocclusionThreshold = _disocclusionThreshold;
            normalThresHold = _normalThresHold;
                   
            camPreInvView = _camPreInvView;
            preUvToViewA = _preUvToViewA;
            preUvToViewB = _preUvToViewB;
            
            preViewZMap = _preViewZMap;
            preLightPropMap = _preLightPropMap;
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
                    
            //float preCenterViewZ = preViewZMap.SampleLevel(samLinearClmap, preUv, 0);
            //float3 preCenterPosV = GetViewPos(preUv, preCenterViewZ, preUvToViewA, preUvToViewB);
            //float3 preCenterPosW = mul(preCenterPosV, camPreInvView);
            float3 preCenterNormal = UnpackNormal(preNormalMap.SampleLevel(samLinearClmap, preUv, 0));
        
            /*
                Since the catmulParam origin is located at the center,
                it is necessary to subtract 0.5f in order to perform the GatherRed operation in Cubic 
            */
            float2 sampleCenterUv = (catmulParam.originSampleCoord - 0.5f) * invRtSize;
            float4 preViewZ00 = preViewZMap.GatherRed(samLinearClmap, sampleCenterUv).wzxy;
            float4 preViewZ10 = preViewZMap.GatherRed(samLinearClmap, sampleCenterUv + float2(2.0f, 0.0f) * invRtSize).wzxy;
            float4 preViewZ01 = preViewZMap.GatherRed(samLinearClmap, sampleCenterUv + float2(0.0f, 2.0f) * invRtSize).wzxy;
            float4 preViewZ11 = preViewZMap.GatherRed(samLinearClmap, sampleCenterUv + float2(2.0f, 2.0f) * invRtSize).wzxy;
            //float disocclusionThreshold = 1.0f / (camFar - camNear);
            
            float3 disocclusion00 = ComputeDisocclusion2x2(sampleCenterUv + float2(-1.0f, -1.0f) * invRtSize, preViewZ00).yzw;
            float3 disocclusion10 = ComputeDisocclusion2x2(sampleCenterUv + float2(1.0f, -1.0f) * invRtSize, preViewZ10).xzw;
            float3 disocclusion01 = ComputeDisocclusion2x2(sampleCenterUv + float2(-1.0f, 1.0f) * invRtSize, preViewZ01).xyw;
            float3 disocclusion11 = ComputeDisocclusion2x2(sampleCenterUv + float2(1.0f, 1.0f) * invRtSize, preViewZ11).xyz;
            float4 bilinearOcclusion = float4(disocclusion00.z, disocclusion10.y, disocclusion01.y, disocclusion11.x);
            
            float4 lightProp00 = preLightPropMap.GatherAlpha(samPointClamp, sampleCenterUv).wzxy;
            float4 lightProp10 = preLightPropMap.GatherAlpha(samPointClamp, sampleCenterUv + float2(2.0f, 0.0f) * invRtSize).wzxy;
            float4 lightProp01 = preLightPropMap.GatherAlpha(samPointClamp, sampleCenterUv + float2(0.0f, 2.0f) * invRtSize).wzxy;
            float4 lightProp11 = preLightPropMap.GatherAlpha(samPointClamp, sampleCenterUv + float2(2.0f, 2.0f) * invRtSize).wzxy;
      
            uint3 materialID00 = float3(lightProp00.y, lightProp00.z, lightProp00.w) * MATERIAL_ID_RANGE;
            uint3 materialID10 = float3(lightProp10.x, lightProp10.z, lightProp10.w) * MATERIAL_ID_RANGE;
            uint3 materialID01 = float3(lightProp01.x, lightProp01.y, lightProp01.w) * MATERIAL_ID_RANGE;
            uint3 materialID11 = float3(lightProp11.x, lightProp11.y, lightProp11.z) * MATERIAL_ID_RANGE;
            
            disocclusion00 *= all(materialID00 == centerMaterialID);
            disocclusion10 *= all(materialID10 == centerMaterialID);
            disocclusion01 *= all(materialID01 == centerMaterialID);
            disocclusion11 *= all(materialID11 == centerMaterialID);
       
            uint viewTestPassCount = dot(disocclusion00 + disocclusion10 + disocclusion01 + disocclusion11, 1.0f);
            result.passCount += viewTestPassCount;
            result.canUseCubic = viewTestPassCount > 11.0f; 
            
            result.bilinearParameter = CustomSampling::GetBilinearFilter(sampleCenterUv, rtSize);
            result.customWeight = CustomSampling::GetBilinearCustomWeights(result.bilinearParameter, bilinearOcclusion);
            result.canUseBilinear = any(result.customWeight > 0.0f);
            //any(result.customWeight > 0.0f);
            //any(result.customWeight > 0.0f);
            //result.customWeight *= result.customWeight;
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