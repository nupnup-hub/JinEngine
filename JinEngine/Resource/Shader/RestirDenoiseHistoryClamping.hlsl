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
#define RADIUS 2
#define LOOP_COUNT 25

Texture2D scrColorHistory : register(t0);
Texture2D scrFastColorHistory : register(t1);
Texture2D<float> historyLength : register(t2);
RWTexture2D<float4> destColorHistory : register(u0);
RWTexture2D<float4> destFastColorHistory : register(u1);
SamplerState samLinearClmap : register(s0);
  
float3 Clamping(int2 pixelCoord, float2 pixelCenterCoord, Texture2D src, RWTexture2D<float4> dest)
{
    float3 m1 = float3(0, 0, 0);
    float3 m2 = float3(0, 0, 0);
    
    [unroll]
    for (int y = -RADIUS; y <= RADIUS; ++y)
    {
        [unroll]
        for (int x = -RADIUS; x <= RADIUS; ++x)
        {
            float2 sampleUv = float2(pixelCenterCoord + float2(x, y)) * cb.invRtSize;
            float3 color = src.SampleLevel(samLinearClmap, sampleUv, 0);
            m1 += RGBToYCoCg(color);
            m2 += m1 * m1;
        }
    }
    m1 /= LOOP_COUNT;
    m2 /= LOOP_COUNT;
    float sigma = sqrt(m2 - m1 * m1); 
    float4 centerColorHistory = src.SampleLevel(samLinearClmap, pixelCenterCoord * cb.invRtSize, 0);
    float3 centerColor = centerColorHistory.xyz;
    float3 colorYCoCg = RGBToYCoCg(centerColor);
    colorYCoCg = clamp(colorYCoCg, m1 - sigma, m1 + sigma);

    float3 clampColor = YCoCgToRGB(colorYCoCg);  
    return clampColor;
}

[numthreads(DIMX, DIMY, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
    
    int2 pixelCoord = dispatchThreadID.xy;
    float2 pixelCenterCoord = pixelCoord + float2(0.5f, 0.5f);
 
    float3 clampColor = Clamping(pixelCoord, pixelCenterCoord, scrColorHistory, destColorHistory);
    float3 fastClampColor = Clamping(pixelCoord, pixelCenterCoord, scrFastColorHistory, destFastColorHistory);
         
    uint currHistoryLength = historyLength[pixelCoord];
    if (currHistoryLength <= FIXED_FRAME_COUNT)
        clampColor.xyz = fastClampColor.xyz;
   
    destColorHistory[pixelCoord].xyz = clampColor;
    destColorHistory[pixelCoord].w = ComputeColorVariance(clampColor);
    
    destFastColorHistory[pixelCoord].xyz = fastClampColor;
    destFastColorHistory[pixelCoord].w = ComputeColorVariance(fastClampColor);
}