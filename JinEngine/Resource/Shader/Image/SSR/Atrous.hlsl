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
#include"../../Common/RandomNumberGenerator.hlsl"
 
#define RADIUS 2

cbuffer AtorusConstants : register(b0)
{
    float4 dimData; //xy = rtSize, zw = invRtSize
    float camFar;
    uint stepSize;
};

Texture2D src : register(t0);
Texture2D<float> viewZMap : register(t1); 
Texture2D lightProp : register(t2);
RWTexture2D<float4> dest : register(u0);
SamplerState samLinearClmap : register(s0);
 
[numthreads(DIMX, DIMY, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= dimData.x || dispatchThreadID.y >= dimData.y)
        return;
    
    const int2 pixelCoord = dispatchThreadID.xy;
    const float2 pixelCenterCoord = pixelCoord + float2(0.5f, 0.5f);
    const float2 uv = pixelCenterCoord * dimData.zw;
  
    const float4 centerColor = src.SampleLevel(samLinearClmap, uv, 0);   
    const float centerViewZ = viewZMap.SampleLevel(samLinearClmap, uv, 0); 
   
    if (centerViewZ == camFar)
    {
        // not a valid depth => must be envmap => do not filter
        dest[pixelCoord] = centerColor;
        return;
    } 
    
    float roughness = UnpackRoughness(lightProp.SampleLevel(samLinearClmap, uv, 0)); 
    float4 colorSum = centerColor;
    float weightSum = 1.0;
 
    //glossy한 효과는 반사되는 사물의 외곽선을 넘어서 번져보이는 효과를 예상하며
    //blur가 가장 필요한 부분은 외곽선 주위에 픽셀들이므로 edge-stopping functions은 사용하지 않는다.
    const float kernelWeights[RADIUS + 1] = { 1.0f, 2.0f / 3.0f, 1.0f / 6.0f };
    
    [unroll]
    for (int yy = -RADIUS; yy <= RADIUS; yy++)
    {
        if (roughness == 0)
            continue;
        
        [unroll]
        for (int xx = -RADIUS; xx <= RADIUS; xx++)
        {
            if(yy == 0 && xx == 0)
                continue;
            
            const float2 offset = float2(xx, yy) * stepSize;
            const float2 sampleUv = uv + offset * dimData.zw;
            if (!IsValidUv(sampleUv))
                continue;
            
            const float sampleViewZ = viewZMap.SampleLevel(samLinearClmap, sampleUv, 0);        
            if (centerViewZ == sampleViewZ)
                continue;
            
            const float kernel = kernelWeights[abs(xx)] * kernelWeights[abs(yy)];
            const float4 sampleColor = src.SampleLevel(samLinearClmap, sampleUv, 0);
            float weight = kernel;
            colorSum += sampleColor * weight;
            weightSum += weight;
        }
    }
    weightSum = max(weightSum, EPSILON);
     
    float4 resultColor = colorSum / weightSum;
    dest[pixelCoord] = resultColor; 
}