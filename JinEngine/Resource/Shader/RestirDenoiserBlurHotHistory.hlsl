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
  
#define VALID_THRESHOLD 2

Texture2D colorHistoryIntermediate : register(t0);
Texture2D<float2> momentHistory : register(t1);
Texture2D<float> historyLength : register(t2);
Texture2D<float> viewZMap : register(t3);
Texture2D normalMap : register(t4);
Texture2D<float2> depthDerivative : register(t5);
RWTexture2D<float4> colorHistory : register(u0);
SamplerState samLinearClmap : register(s0);
  
[numthreads(DIMX, DIMY, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
    
    int2 pixelCoord = dispatchThreadID.xy; 
    float2 uv = (pixelCoord + float2(0.5f, 0.5f)) * cb.invRtSize;
    float currHistoryLength = historyLength[pixelCoord];
 
    if (currHistoryLength < VALID_THRESHOLD) // not enough temporal history available
    {
        float3 colorSum = float3(0.0f, 0.0f, 0.0f);
        float2 momentSum = float2(0.0f, 0.0f);
        float weightSum = 0.0f;
        
        const float4 centerColor = colorHistoryIntermediate.SampleLevel(samLinearClmap, uv, 0);
        const float centerLuminance = RGBToLuminance(centerColor.rgb);
        const float centerViewZ = viewZMap.SampleLevel(samLinearClmap, uv, 0);
        if (centerViewZ < 0)
        {
            // current pixel does not a valid depth => must be envmap => do nothing
            colorHistory[pixelCoord] = centerColor;
            return;
        }     
        const float3 centerNormal = UnpackNormal(normalMap.SampleLevel(samLinearClmap, uv, 0));
        const float2 ddxy = depthDerivative.SampleLevel(samLinearClmap, uv, 0);

        // compute first and second moment spatially. This code also applies cross-bilateral
        // filtering on the input illumination.
        const int radius = 3;
        const float distance = 0.5f;
        
        CrossBilateral::NormalDepthLuminance::Parameters param;
        param.depth.Initialize(centerViewZ, ddxy, 3.0f);
        param.normal.Initialize(centerNormal);
        param.luminance.Initialize(centerLuminance, uv, cb.invRtSize, colorHistoryIntermediate, samLinearClmap);
 
        //const float phiDepth = max(length(ddxy), 1e-8) * 3.0; 
        
        for (int yy = -radius; yy <= radius; yy++)
        {
            for (int xx = -radius; xx <= radius; xx++)
            {
                const float2 offset = float2(xx, yy) * distance;
                const float2 sampleUv = uv + offset * cb.invRtSize;
                if (IsValidUv(sampleUv))
                { 
                    const float3 sampleColor = colorHistoryIntermediate.SampleLevel(samLinearClmap, sampleUv, 0).xyz;
                    const float2 sampleMoment = momentHistory.SampleLevel(samLinearClmap, sampleUv, 0);
                    const float sampleLuminance = RGBToLuminance(sampleColor);
                    const float sampleViewZ = viewZMap.SampleLevel(samLinearClmap, sampleUv, 0);
                    const float3 sampleNomral = UnpackNormal(normalMap.SampleLevel(samLinearClmap, sampleUv, 0));
                     
                    param.normal.Update(sampleNomral);
                    param.depth.Update(sampleViewZ, -offset);
                    param.luminance.Update(sampleLuminance);
                    
                    const float weight = CrossBilateral::NormalDepthLuminance::ComputeWeight(param);
                    //const float weight = CrossBilateral::SVGF::ComputeWeight(param, PHI_NORMAL, phiDepth * length(float2(xx, yy)), PHI_COLOR);
                    colorSum += sampleColor * weight;
                    momentSum += sampleMoment * weight;
                    weightSum += weight; 
                }
            }
        }

        // Clamp sum to >0 to avoid NaNs.
        weightSum = max(weightSum, 1e-6f);
        colorSum /= weightSum;
        momentSum /= weightSum;

        // compute variance using the first and second moments
        //float variance = max(0.0f, momentSum.g - momentSum.r * momentSum.r);
        float variance = momentSum.g - momentSum.r * momentSum.r;
        // give the variance a boost for the first frames
        variance *= 4.0 / currHistoryLength;

        colorHistory[pixelCoord] = float4(colorSum, variance);
    }
    else
        colorHistory[pixelCoord] = colorHistoryIntermediate.SampleLevel(samLinearClmap, uv, 0);
}