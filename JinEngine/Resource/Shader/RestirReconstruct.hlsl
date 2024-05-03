#pragma once  
#include"RestirDenoiseCommon.hlsl" 
#include"DepthFunc.hlsl"

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif 

#define MIPMAP_COUNT 4 
#define WEIGHT_THRESHOLD 128
#define RADIUS 2
#define LOOP_RANGE 5 
#define MIP_WEIGHT_STEP 0.25f
#define BASE_COLOR_NEAR_THRESHOLD 25
#define COLOR_TA_NEAR_THRESHOLD 75 

Texture2D mipmap[MIPMAP_COUNT] : register(t0, space0);
Texture2D<float> viewZMap : register(t1, space1);
Texture2D<float2> depthDerivative : register(t2, space2);
RWTexture2D<float4> colorHistory : register(u0);
SamplerState samLinearClmap : register(s0);
    
float CrossBilateralWeight(float r, float centerViewZ, float sampleViewZ)
{
    const float blurSigma = (RADIUS + 1.0f) * 0.5f;
    const float blurFalloff = 1.0f / (2.0f * blurSigma * blurSigma);

    float dz = sampleViewZ - centerViewZ;
    return exp2(-r * r * blurFalloff - dz * dz);
}
void HierarchicalInterpolate(float2 uv, float2 ddxy, float viewZ, float weightMax, Texture2D currMipmap, inout float3 colorSum, inout float weightSum)
{
    CrossBilateral::Depth::Parameters param;
    param.Initialize(viewZ, ddxy);
    
    [unroll]
    for (uint sampleY = 0; sampleY < LOOP_RANGE; ++sampleY)
    {
        [unroll]
        for (uint sampleX = 0; sampleX < LOOP_RANGE; ++sampleX)
        {
            int2 offset = int2(sampleX - RADIUS, sampleY - RADIUS);
            float2 sampleUv = uv + offset * cb.invRtSize;
            float4 sampleData = currMipmap.SampleLevel(samLinearClmap, sampleUv, 0);
            
            float3 sampleColor = sampleData.xyz;
            //float samplelinearDepth = sampleData.w; 
            param.Update(viewZMap.SampleLevel(samLinearClmap, sampleUv, 0), -offset); 
            
            float weight = IsValidUv(sampleUv);
            //weight *= sampleZ < cb.denoiseRange;
            weight *= CrossBilateral::Depth::ComputeWeight(param);
            weight = min(weight, weightMax);
            
            colorSum += sampleColor * weight;
            weightSum += weight;
        }
    } 
}

[numthreads(DIMX, DIMY, 1)]
void main(int groupIndex : SV_GroupIndex, int3 dispatchThreadID : SV_DispatchThreadID)
{
    //thread dimension is same as src half resolution 
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
      
    int2 pixelCoord = dispatchThreadID.xy;
    float2 uv = (pixelCoord + 0.5f) * cb.invRtSize;
    float2 ddxy = depthDerivative.SampleLevel(samLinearClmap, uv, 0);     
    //float linearDepth = mipmap[0].SampleLevel(samLinearClmap, uv, 0).w;  
    float viewZ = viewZMap.SampleLevel(samLinearClmap, uv, 0);
    
    float3 colorSum = float3(0, 0, 0);
    float weightSum = 0;
    
    float3 baseMipLevelColorSum = float3(0, 0, 0); 
    [unroll]
    for (uint i = 3; i < MIPMAP_COUNT; ++i)
    {
        float weightMax = (MIP_WEIGHT_STEP * i + MIP_WEIGHT_STEP);
        //float weightMax = 1.0f;
        float3 localColorSum = float3(0, 0, 0);
        float localWeightSum = 0;
        HierarchicalInterpolate(uv, ddxy, viewZ, weightMax, mipmap[MIPMAP_COUNT - 1 - i], localColorSum, localWeightSum);
        //if (weightSum >= WEIGHT_THRESHOLD)
        //    break;
        colorSum += localColorSum;
        weightSum += localWeightSum;
        if (i == MIPMAP_COUNT - 1)
            baseMipLevelColorSum = localColorSum / localWeightSum;
    }
    colorSum /= weightSum;
    
    float3 oriColor = mipmap[0].SampleLevel(samLinearClmap, uv, 0).xyz; 
    float baseColorFactor = viewZ < BASE_COLOR_NEAR_THRESHOLD ? (viewZ / BASE_COLOR_NEAR_THRESHOLD) : 1.0f;
    float taColorFactor = viewZ < COLOR_TA_NEAR_THRESHOLD ? (viewZ / COLOR_TA_NEAR_THRESHOLD) : 1.0f;
    
    float3 baseColor = lerp(oriColor, baseMipLevelColorSum, baseColorFactor); 
    colorHistory[dispatchThreadID.xy].xyz = lerp(baseColor, colorSum, taColorFactor);
    //colorHistory[pixelCoord].xyz = mipmap[0].SampleLevel(samLinearClmap, uv, 0).xyz;

}