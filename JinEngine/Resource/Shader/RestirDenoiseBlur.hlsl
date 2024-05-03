#pragma once 
#include"DepthFunc.hlsl" 
#include"RandomNumberGenerator.hlsl"
#include"RestirDenoiseCommon.hlsl"
#include"ColorSpaceUtility.hlsl" 

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif 
 
#ifndef SAMPLE_COUNT  
#define POISSON_16
#endif
 

#if defined(POISSON_8)
// samples = 8, min distance = 0.5, average samples on radius = 2
#define USE_POISSON8_F3 
#define SAMPLE_COUNT 8
#define HALF_SAMPLE_COUNT 4
#include"Poisson.hlsl"
float3 GetPoisson(const uint index)
{
    return poissonDiskF3_8[index];
}
#elif defined(POISSON_16)
#define USE_POISSON16_F3 
#define SAMPLE_COUNT 16
#define HALF_SAMPLE_COUNT 8
#include"Poisson.hlsl"
// samples = 16, min distance = 0.38, average samples on radius = 2
float3 GetPoisson(const uint index)
{
    return poissonDiskF3_16[index];
}
#else
// samples = 32, min distance = 0.26, average samples on radius = 3
#define USE_POISSON32_F3 
#define SAMPLE_COUNT 32
#define HALF_SAMPLE_COUNT 16
#include"Poisson.hlsl" 
float3 GetPoisson(const uint index)
{
    return poissonDiskF3_32[index];
}
#endif

Texture2D colorHistory : register(t0);
Texture2D<float> viewZMap : register(t1);
Texture2D normalMap : register(t2); 
Texture2D historyLength : register(t3);  
Texture2D<float2> depthDerivative : register(t4);  
RWTexture2D<float4> resultColor : register(u0);
SamplerState samLinearClmap : register(s0);
  
float3 DiffuseFiltering(int2 pixelCoord, float2 centerUv, float3 diffuse, float3 centerNormal, float centerViewZ, float2 ddxy, float radius)
{ 
    CrossBilateral::NormalDepth::Parameters param;
    param.normal.Initialize(centerNormal);
    param.depth.Initialize(centerViewZ, ddxy);
 
    float3 result = diffuse;
    float weightSum = 1.0f;  // CrossBilateral::NormalDepth::ComputeWeight(param);
      
    RandomNumberGenerator rng;
    rng.Initialize(pixelCoord, cb.sampleNumber);
    float2 trigger = RandomRotationTrig(rng.Random01()); 
    
    [unroll]
    for (uint i = 0; i < SAMPLE_COUNT; ++i)
    {
        //poisson + random direction rotate
        float3 poisson = GetPoisson(i);

        //Per pixel kernel rotation
        //Input signal is already noisy 
        float2 offset = Rotate(poisson.xy, trigger) * radius;
        float2 uv = centerUv + offset * cb.invRtSize;
        //uv = max(uv, float2(0, 0));
        
        float3 sampleDiffuse = colorHistory.SampleLevel(samLinearClmap, uv, 0).xyz;  
        float sampleViewZ= viewZMap.SampleLevel(samLinearClmap, uv, 0);
        float3 sampleNormalW = UnpackNormal(normalMap.SampleLevel(samLinearClmap, uv, 0));
         
        param.normal.Update(sampleNormalW);
        param.depth.Update(sampleViewZ, -offset);
   
        float weight = IsValidUv(uv); 
        weight *= CrossBilateral::NormalDepth::ComputeWeight(param);
        result += sampleDiffuse * weight;
        weightSum += weight;
    } 
    return result / weightSum;
}
[numthreads(DIMX, DIMY, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    //Process
    //1. calculate radius
    //2. blur history map color
    //A. geometry, normal, distance, roughness, depth등 다양한 weight가 사용될수있으나 현재는 normal과 viewZ값만 사용
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
    float radiusOffset =  1.0f - abs(variance - diffuseVariance);
    
    float3 centerNormal = UnpackNormal(normalMap.SampleLevel(samLinearClmap, uv, 0));
    float centerViewZ = viewZMap.SampleLevel(samLinearClmap, uv, 0); 
    float viewRange = cb.camNearFar.y - cb.camNearFar.x;
    float2 ddxy = depthDerivative.SampleLevel(samLinearClmap, uv, 0);
 
    //float radius = cb.baseRadius + (1.0f / float(currHistoryLength)) * cb.radiusRange * (1.0f - centerLinearDepth);
    float radius = cb.baseRadius + cb.radiusRange * (1.0f - ((centerViewZ - cb.camNearFar.x) / viewRange));
    float3 denoiseColor = DiffuseFiltering(pixelCoord, uv, diffuse, centerNormal, centerViewZ, ddxy, radius);
     
    resultColor[pixelCoord].xyz = denoiseColor;
    
    //resultColor[pixelCoord].xyz = diffuse;
}