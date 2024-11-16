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
#include"../../Common/DepthFunc.hlsl" 

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif   

#define RADIUS 1
#define TAB_DISTANCE 1.0f
 
Texture2D history : register(t0);
RWTexture2D<float4> destMap : register(u0); 
SamplerState samPointClmap : register(s0);
SamplerState samLinearClmap : register(s1);

[numthreads(DIMX, DIMY, 1)]
void main(int groupIndex : SV_GroupIndex, int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.common.rtSize.x || dispatchThreadID.y >= cb.common.rtSize.y)
        return;
   
    int2 pixelCoord = dispatchThreadID.xy;
    float2 uv = (pixelCoord + float2(0.5f, 0.5f)) * cb.common.invRtSize;
      
    float3 historyColor; 
    bool isOutline;
    TAA::UnPackHistory(history.SampleLevel(samLinearClmap, uv, 0), historyColor, isOutline);
 
    if (!isOutline)
    {
        float3 center = historyColor;
        float3 left = history.SampleLevel(samLinearClmap, uv, 0, int2(-TAB_DISTANCE, 0)).xyz;
        float3 right = history.SampleLevel(samLinearClmap, uv, 0, int2(TAB_DISTANCE, 0)).xyz;
        float3 up = history.SampleLevel(samLinearClmap, uv, 0, int2(0, -TAB_DISTANCE)).xyz;
        float3 down = history.SampleLevel(samLinearClmap, uv, 0, int2(0, TAB_DISTANCE)).xyz;
 
        float3 sharpColor = saturate(center + 4 * center - left - right - up - down);
        destMap[pixelCoord].xyz = sharpColor;
        
        //destMap[pixelCoord].xyz = float3(1, 0, 0);
    }
    else
    {
        destMap[pixelCoord].xyz = historyColor;
     //   destMap[pixelCoord].xyz = float3(0, 0, 1);
    }
}