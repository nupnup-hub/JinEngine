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

#define SHARED_BUFFER_SIZE 256

Texture2D scrColorHistory : register(t0);
RWTexture2D<float4> destColorHistory : register(u0);
SamplerState samLinearClmap : register(s0);
  
groupshared float4 sharedColor[SHARED_BUFFER_SIZE];
void StuffShareMemory(int groupIndex, const float2 uv)
{
    sharedColor[groupIndex] = scrColorHistory.SampleLevel(samLinearClmap, uv, 0);
}

[numthreads(DIMX, DIMY, 1)]
void main(int groupIndex : SV_GroupIndex, int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
    
    int2 pixelCoord = dispatchThreadID.xy;
    float2 pixelCenterCoord = pixelCoord + float2(0.5f, 0.5f);
    float2 uv = pixelCenterCoord * cb.invRtSize;
    
    StuffShareMemory(groupIndex, uv);
    GroupMemoryBarrierWithGroupSync();
    
    float3 centerColor = sharedColor[groupIndex].rgb;
    float centerVariance = sharedColor[groupIndex].a;
    float centerLuminance = RGBToLuminance(centerColor);
    int centerSharedCoord = groupIndex;
    
    float maxLuminance = -FLT_MAX;
    float minLuminance = FLT_MAX;
    int maxLuminanceCoord = groupIndex;
    int minLuminanceCoord = groupIndex;
    
    [unroll]
    for (int y = -1; y <= 1; y++)
    {
        [unroll]
        for (int x = -1; x <= 1; x++)
        {
            if ((x == 0) && (y == 0))
                continue;
            
            float2 sampleUv = uv + float2(x, y) * cb.invRtSize;
            if (IsValidUv(sampleUv))
                continue;
            
            int sampleGroupIndex = groupIndex + x + (y * DIMX);
            if (sampleGroupIndex < 0 || sampleGroupIndex >= SHARED_BUFFER_SIZE)
                continue;
            
            float3 sampleColor = sharedColor[sampleGroupIndex].xyz;
            float sampleLuminance = RGBToLuminance(sampleColor);
            
            if (sampleLuminance > maxLuminance)
            {
                maxLuminance = sampleLuminance;
                maxLuminanceCoord = sampleGroupIndex;
            }
            if (sampleLuminance < minLuminance)
            {
                minLuminance = sampleLuminance;
                minLuminanceCoord = sampleGroupIndex;
            }
        }
    }
    if (centerLuminance > maxLuminance)
        centerSharedCoord = maxLuminanceCoord;
    if (centerLuminance < minLuminance)
        centerSharedCoord = minLuminanceCoord;
    
    ///destColorHistory[pixelCoord] = scrColorHistory[pixelCoord];
    destColorHistory[pixelCoord] = float4(sharedColor[centerSharedCoord].xyz, centerVariance);
}