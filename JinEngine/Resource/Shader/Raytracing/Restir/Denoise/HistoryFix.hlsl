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
#include"Common.hlsl"
#include"../../../Common/DepthFunc.hlsl" 
#include"../../../Common/RandomNumberGenerator.hlsl" 

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif 
   
#define BLUR_RADIUS 2
#define TAB_DISTANCE 4

Texture2D srcColorHistory : register(t0);
Texture2D scrFastColorHistory : register(t1);
Texture2D<uint> historyLength : register(t2);
Texture2D<float> viewZMap : register(t3);
Texture2D normalMap : register(t4);
Texture2D<float2> depthDerivative : register(t5);
RWTexture2D<float4> destColorHistory : register(u0);
RWTexture2D<float4> destFastColorHistory : register(u1);
SamplerState samLinearClmap : register(s0);
   
[numthreads(DIMX, DIMY, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
    
    int2 pixelCoord = dispatchThreadID.xy; 
    float2 centerUv = (pixelCoord + float2(0.5f, 0.5f)) * cb.invRtSize;
    uint currHistoryLength = historyLength[pixelCoord];
 
    if (currHistoryLength < FIXED_FRAME_COUNT) // not enough temporal history available
    {
        float4 centerHistory = srcColorHistory.SampleLevel(samLinearClmap, centerUv, 0);
        float3 centerNormal = UnpackNormal(normalMap.SampleLevel(samLinearClmap, centerUv, 0));
        float centerViewZ = viewZMap.SampleLevel(samLinearClmap, centerUv, 0);
        float2 ddxy = depthDerivative.SampleLevel(samLinearClmap, centerUv, 0);
        
        CrossBilateral::NormalDepthLuminance::Parameters param;
        param.normal.Initialize(centerNormal);
        param.depth.Initialize(centerViewZ, ddxy);
        param.luminance.Initialize(centerHistory.w, centerUv, cb.invRtSize, srcColorHistory, samLinearClmap);
        
        //float3 colorSum = centerHistory.xyz;
        //float momentSum = centerHistory.w;
        float3 colorSum = float3(0, 0, 0);
        //float momentSum = 0;
        float weightSum = 0.0f; // CrossBilateral::NormalDepth::ComputeWeight(param);
 
        [unroll]
        for (int i = -BLUR_RADIUS; i <= BLUR_RADIUS; ++i)
        {
            [unroll]
            for (int j = -BLUR_RADIUS; j <= BLUR_RADIUS; ++j)
            {
                //if(i == 0 && j == 0)
                //    continue;
                
                //Per pixel kernel rotation
                //Input signal is already noisy  
                int2 offset = int2(i, j) * TAB_DISTANCE;
                float2 uv = centerUv + offset * cb.invRtSize;
                //uv = max(uv, float2(0, 0));
        
                float4 sampleHistory = srcColorHistory.SampleLevel(samLinearClmap, uv, 0);
                float sampleViewZ = viewZMap.SampleLevel(samLinearClmap, uv, 0);
                float3 sampleNormalW = UnpackNormal(normalMap.SampleLevel(samLinearClmap, uv, 0));
         
                param.normal.Update(sampleNormalW);
                param.depth.Update(sampleViewZ, -offset);
                param.luminance.Update(sampleHistory.w);
                
                float weight = IsValidUv(uv);
                weight *= CrossBilateral::NormalDepthLuminance::ComputeWeight(param);
                colorSum += sampleHistory.xyz * weight;
                //momentSum += sampleHistory.w * weight;
                weightSum += weight;
            }
        } 
        weightSum = max(weightSum, EPSILON);
        colorSum /= weightSum; 
        //colorSum = float3(0, 0, 1);
        float variance = ComputeColorVariance(colorSum);
        destColorHistory[pixelCoord] = float4(colorSum, variance);
        destFastColorHistory[pixelCoord] = float4(colorSum, variance);
    }
    else
    { 
        destColorHistory[pixelCoord] = srcColorHistory[pixelCoord];
        destFastColorHistory[pixelCoord] = scrFastColorHistory[pixelCoord];
        
       // destColorHistory[pixelCoord].xyz = float3(1, 0, 0);
       // destFastColorHistory[pixelCoord].xyz = float3(1, 0, 0);
    }
}