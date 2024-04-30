#pragma once  
#include"RestirDenoiseCommon.hlsl" 
#include"DepthFunc.hlsl"

#ifndef DIMX
#define DIMX 8
#endif
#ifndef DIMY
#define DIMY 8
#endif 

#define RADIUS 2
#define LOOP_COUNT 5
cbuffer DownSamplingConstants
{
    uint2 destRtSize;
    float2 srcInvRtSize;
    uint mipLevel;
};

Texture2D src : register(t0);
Texture2D normalMap : register(t1);
Texture2D viewZMap : register(t2);
Texture2D<float2> depthDerivative : register(t3);
RWTexture2D<float4> dest : register(u0);
SamplerState samPointClmap : register(s0);
SamplerState samLinearClmap : register(s1);
     
[numthreads(DIMX, DIMY, 1)]
void main(int groupIndex : SV_GroupIndex, int3 dispatchThreadID : SV_DispatchThreadID)
{
    //thread dimension is same as src half resolution 
    if (dispatchThreadID.x >= destRtSize.x || dispatchThreadID.y >= destRtSize.y)
        return;
      
    uint2 pixelCoord = dispatchThreadID.xy;
    if (mipLevel > 0)
        pixelCoord *= 2;
    float2 uv = (pixelCoord + 0.5f) * srcInvRtSize;
 
    float3 centerNormal = UnpackNormal(normalMap.SampleLevel(samLinearClmap, uv, 0));
    float centerViewZ = viewZMap.SampleLevel(samLinearClmap, uv, 0).x;
    float2 ddxy = depthDerivative.SampleLevel(samLinearClmap, uv, 0);
    
    float offsetDistance = 0.5f;     
    float4 colorSum = float4(0, 0, 0, 0);
    float weightSum = 0;
    
    CrossBilateral::NormalDepth::Parameters param;
    param.normal.Initialize(centerNormal);
    param.depth.Initialize(centerViewZ, ddxy);
    
    [unroll]
    for (int i = 0; i < LOOP_COUNT; i++)
    {
        [unroll]
        for (int j = 0; j < LOOP_COUNT; j++)
        {
            float2 offset = int2(j - RADIUS, i - RADIUS) * offsetDistance;
            float2 sampleUv = uv + (offset * srcInvRtSize);
            float4 sampleColor = src.SampleLevel(samLinearClmap, sampleUv, 0); 
            float sampleViewZ = viewZMap.SampleLevel(samLinearClmap, sampleUv, 0).x;
            float3 sampleNormalW = UnpackNormal(normalMap.SampleLevel(samLinearClmap, sampleUv, 0));
         
            param.normal.Update(sampleNormalW);           
            param.depth.Update(sampleViewZ, -offset); 
            
            float weight = IsValidUv(sampleUv);
            weight *= CrossBilateral::NormalDepth::ComputeWeight(param);
            //weight *= CrossBilateral::Depth::GetWeight(centerLinearDepth, ddxy, sampleLinearDepth, param.Depth);
            
            colorSum += sampleColor * weight;
            weightSum += weight;
        } 
    }
    dest[dispatchThreadID.xy] = colorSum / weightSum; 
}