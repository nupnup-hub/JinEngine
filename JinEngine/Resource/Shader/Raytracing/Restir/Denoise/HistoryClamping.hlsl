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
#define RADIUS 2
#define LOOP_COUNT 25

#define SCALE 1.0f

#define SHARED_BUFFER_SIZE DIMX * DIMY

Texture2D scrColorHistory : register(t0);
Texture2D scrFastColorHistory : register(t1);
Texture2D<uint> historyLength : register(t2); 
RWTexture2D<float4> destColorHistory : register(u0); 
SamplerState samLinearClmap : register(s0);
  
groupshared float4 sharedColor[SHARED_BUFFER_SIZE];
void StuffShareMemory(int groupIndex, const float2 uv)
{
    sharedColor[groupIndex] = scrFastColorHistory.SampleLevel(samLinearClmap, uv, 0);
}

float3 Clamping(int groupIndex, float3 centerColor, float2 pixelCenterCoord)
{
    float3 m1 = float3(0, 0, 0);
    float3 m2 = float3(0, 0, 0);
    
    uint loopCount = 0;
    [unroll]
    for (int y = -RADIUS; y <= RADIUS; ++y)
    {
        [unroll]
        for (int x = -RADIUS; x <= RADIUS; ++x)
        {
            //float2 sampleUv = float2(pixelCenterCoord + float2(x, y)) * cb.invRtSize;
            int sampleGroupIndex = groupIndex + x + (y * DIMX);
            if (sampleGroupIndex < 0 || sampleGroupIndex >= SHARED_BUFFER_SIZE)
                continue;
            
            float3 color = sharedColor[sampleGroupIndex].xyz;
            m1 += RGBToYCoCg(color);
            m2 += m1 * m1;           
            ++loopCount;
        }
    }
    m1 /= loopCount;
    m2 /= loopCount;
    //분산 = 제평 - 평제
    //표준편차 = sqrt(분산)
    float sigma = sqrt(m2 - m1 * m1) * SCALE;
    float3 minColorYCoCg = m1 - sigma;
    float3 maxColorYCoCg = m1 + sigma;
    
    float3 colorYCoCg = clamp(RGBToYCoCg(centerColor.xyz), minColorYCoCg, maxColorYCoCg); 
    float3 clampColor = YCoCgToRGB(colorYCoCg);  
    return clampColor;
}  
[numthreads(DIMX, DIMY, 1)]
void main(int groupIndex : SV_GroupIndex, int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
    
    int2 pixelCoord = dispatchThreadID.xy;
    float2 pixelCenterCoord = pixelCoord + float2(0.5f, 0.5f);
    float2 centerUv = pixelCenterCoord * cb.invRtSize;
    
    StuffShareMemory(groupIndex, centerUv);
    GroupMemoryBarrierWithGroupSync();
    
    float3 centerColor = scrColorHistory.SampleLevel(samLinearClmap, pixelCenterCoord * cb.invRtSize, 0);
    float3 clampColor = Clamping(groupIndex, centerColor, pixelCenterCoord);
         
    uint currHistoryLength = historyLength[pixelCoord];
    if (currHistoryLength <= FIXED_FRAME_COUNT)
        centerColor = clampColor;
    
    destColorHistory[pixelCoord].xyz = centerColor;
    destColorHistory[pixelCoord].w = ComputeColorVariance(centerColor);
}