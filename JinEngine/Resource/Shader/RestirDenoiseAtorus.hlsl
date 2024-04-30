#pragma once 
#include"DepthFunc.hlsl" 
#include"RandomNumberGenerator.hlsl"
#include"RestirDenoiseCommon.hlsl"  

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif  
 
Texture2D srcColorHistory : register(t0);
Texture2D<float> viewZMap : register(t1);
Texture2D normalMap : register(t2);
Texture2D historyLength : register(t3);
Texture2D<float2> depthDerivative : register(t4);
RWTexture2D<float4> destColorHistory : register(u0); 
SamplerState samLinearClmap : register(s0);

cbuffer AtorusConstants : register(b1)
{
    uint stepSize;
};
 
[numthreads(DIMX, DIMY, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    const int2 pixelCoord = dispatchThreadID.xy;  
    const float2 pixelCenterCoord = pixelCoord + float2(0.5f, 0.5f);
    const float2 uv = pixelCenterCoord * cb.invRtSize;
    
    const float epsVariance = 1e-10;
    const float kernelWeights[3] = { 1.0, 2.0 / 3.0, 1.0 / 6.0 };

    // constant samplers to prevent the compiler from generating code which
    // fetches the sampler descriptor from memory for each texture access
    const float4 centerColorHistory = srcColorHistory.SampleLevel(samLinearClmap, uv, 0);
    const float centerLuminance = RGBToLuminance(centerColorHistory.rgb);
  
    // number of temporally integrated pixels
    const float currHistoryLength = historyLength[pixelCoord].x;
    const float centerViewZ = viewZMap.SampleLevel(samLinearClmap, uv, 0);
    const float2 ddxy = depthDerivative.SampleLevel(samLinearClmap, uv, 0);
  
    const float viewRange = cb.camNearFar.y - cb.camNearFar.x; 
    if (centerViewZ.x < 0)
    {
        // not a valid depth => must be envmap => do not filter
        destColorHistory[pixelCoord] = centerColorHistory;
        return;
    }
    const float3 centerNormal = UnpackNormal(normalMap.SampleLevel(samLinearClmap, uv, 0));
   
    CrossBilateral::NormalDepthLuminance::Parameters param;
    param.normal.Initialize(centerNormal);
    param.depth.Initialize(centerViewZ, ddxy, stepSize);
    param.luminance.Initialize(centerLuminance, uv, cb.invRtSize, srcColorHistory, samLinearClmap);
 
    //const float phiLIllumination = PHI_COLOR * sqrt(max(0.0, epsVariance + centerVariance));
   // const float phiDepth = max(length(ddxy), 1e-8) * stepSize;
    
    // explicitly store/accumulate center pixel with weight 1 to prevent issues
    // with the edge-stopping functions
    float4 colorSum = centerColorHistory;
    float weightSum = 1.0;
    
    [unroll]
    for (int yy = -2; yy <= 2; yy++)
    {
        [unroll]
        for (int xx = -2; xx <= 2; xx++)
        {
            const float2 offset = float2(xx, yy) * stepSize;
            const float2 sampleUv = uv + offset * cb.invRtSize; 
            const float kernel = kernelWeights[abs(xx)] * kernelWeights[abs(yy)];

            if (IsValidUv(sampleUv) && (xx != 0 || yy != 0)) // skip center pixel, it is already accumulated
            {
                const float4 sampleColorHistory = srcColorHistory.SampleLevel(samLinearClmap, uv, 0);
                const float sampleLuminance = RGBToLuminance(sampleColorHistory.rgb); 
                const float sampleViewZ = viewZMap.SampleLevel(samLinearClmap, uv, 0);
                const float3 sampleNormalW = UnpackNormal(normalMap.SampleLevel(samLinearClmap, uv, 0));
                
                param.normal.Update(sampleNormalW);
                param.depth.Update(sampleViewZ, -offset);
                param.luminance.Update(sampleLuminance);
             
                // compute the edge-stopping functions
                float weight = CrossBilateral::NormalDepthLuminance::ComputeWeight(param);
                //float weight = CrossBilateral::SVGF::ComputeWeight(param, PHI_NORMAL, phiDepth * length(float2(xx, yy)), phiLIllumination);
                weight *= kernel;

                // alpha channel contains the variance, therefore the weights need to be squared, see paper for the formula
                colorSum += float4(weight, weight, weight, weight * weight) * sampleColorHistory;
                weightSum += weight;
            }
        }
    }

    // renormalization is different for variance, check paper for the formula
    float4 resultColor = float4(colorSum / float4(weightSum, weightSum, weightSum, weightSum * weightSum));
    destColorHistory[pixelCoord] = resultColor;
    //destColorHistory[pixelCoord] = centerColorHistory;
} 