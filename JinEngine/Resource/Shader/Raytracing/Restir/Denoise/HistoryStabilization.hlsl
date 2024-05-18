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
 
#ifndef POISSON_SAMPLE_COUNT  
#define POISSON_16
#endif
  
#include"../../../Common/PoissonSelect.hlsl"

Texture2D colorHistory : register(t0);
Texture2D<float> viewZMap : register(t1);
Texture2D normalMap : register(t2);
Texture2D<uint> historyLength : register(t3);
Texture2D<float2> depthDerivative : register(t4);
RWTexture2D<float4> resultColor : register(u0);
SamplerState samLinearClmap : register(s0);
  
float3 DiffuseFiltering(int2 pixelCoord, float2 centerUv, float3 diffuse, float3 centerNormal, float centerViewZ, float2 ddxy, float radius)
{
    float centerLuminance = RGBToLuminance(diffuse);
    CrossBilateral::NormalDepthLuminance::Parameters param;
    param.normal.Initialize(centerNormal);
    param.depth.Initialize(centerViewZ, ddxy);
    param.luminance.Initialize(centerLuminance, centerUv, cb.invRtSize, colorHistory, samLinearClmap);
 
    float3 result = diffuse;
    float weightSum = 1.0f; // CrossBilateral::NormalDepth::ComputeWeight(param);
      
    RandomNumberGenerator rng;
    rng.Initialize(pixelCoord, cb.sampleNumber);
    float2 trigger = RandomRotationTrig(rng.Random01());
    
    [unroll]
    for (uint i = 0; i < POISSON_SAMPLE_COUNT; ++i)
    {
        //poisson + random direction rotate
        float3 poisson = GetPoisson(i);

        //Per pixel kernel rotation
        //Input signal is already noisy 
        float2 offset = Rotate(poisson.xy, trigger) * radius;
        float2 uv = centerUv + offset * cb.invRtSize;
        //uv = max(uv, float2(0, 0));
        
        float3 sampleDiffuse = colorHistory.SampleLevel(samLinearClmap, uv, 0).xyz;
        float sampleLuminance = RGBToLuminance(sampleDiffuse);
        float sampleViewZ = viewZMap.SampleLevel(samLinearClmap, uv, 0);
        float3 sampleNormalW = UnpackNormal(normalMap.SampleLevel(samLinearClmap, uv, 0));
         
        param.normal.Update(sampleNormalW);
        param.depth.Update(sampleViewZ, -offset);
        param.luminance.Update(sampleLuminance);
   
        float weight = IsValidUv(uv);
        weight *= CrossBilateral::NormalDepthLuminance::ComputeWeight(param);
        result += sampleDiffuse * weight;
        weightSum += weight;
    }
    return result / weightSum;
}
[numthreads(DIMX, DIMY, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{ 
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
    
    int2 pixelCoord = dispatchThreadID.xy;
    float2 uv = (pixelCoord + float2(0.5f, 0.5f)) * cb.invRtSize;
    float currHistoryLength = historyLength[pixelCoord].x;
    
    float4 pixelColorHistory = colorHistory.SampleLevel(samLinearClmap, uv, 0);
    float3 diffuse = pixelColorHistory.xyz;
    float variance = pixelColorHistory.w;
 
    float2 moments;
    moments.r = RGBToLuminance(diffuse);
    moments.g = moments.r * moments.r;
 
    // temporal integration of the moments 
    float diffuseVariance = max(0.0f, moments.g - moments.r * moments.r);
    float radiusOffset = 1.0f - abs(variance - diffuseVariance);
    
    float3 centerNormal = UnpackNormal(normalMap.SampleLevel(samLinearClmap, uv, 0));
    float centerViewZ = viewZMap.SampleLevel(samLinearClmap, uv, 0);
    float viewRange = cb.camNearFar.y - cb.camNearFar.x;
    float2 ddxy = depthDerivative.SampleLevel(samLinearClmap, uv, 0);
 
    //float radius = cb.baseRadius + (1.0f / float(currHistoryLength)) * cb.radiusRange * (1.0f - centerLinearDepth);
    //float radius = cb.baseRadius + cb.radiusRange * (1.0f - ((centerViewZ - cb.camNearFar.x) / viewRange));
    float radius = cb.baseRadius + cb.radiusRange * (1.0f / float(1.0f + currHistoryLength));
   
    float3 denoiseColor = DiffuseFiltering(pixelCoord, uv, diffuse, centerNormal, centerViewZ, ddxy, radius);
    resultColor[pixelCoord].xyz = denoiseColor;
    
    //resultColor[pixelCoord].xyz = diffuse;
}