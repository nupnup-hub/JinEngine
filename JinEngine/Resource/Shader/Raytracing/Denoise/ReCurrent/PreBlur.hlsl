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

//Pre blur : Uses constant radius, pass is needed to fix outliers  
//created 2024-05-14 jinwoo jung
 

#pragma once 
#ifndef POISSON_SAMPLE_COUNT  
#define POISSON_8
#endif
#include"../../../Common/PoissonSelect.hlsl"

#include"../Common.hlsl" 
#include"../../../Common/DepthFunc.hlsl"   

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif 

#define BLUR_RADIUS 4

Texture2D color : register(t0);
Texture2D<float> viewZMap : register(t1);
Texture2D normalMap : register(t2); 
Texture2D lightProp : register(t3);
Texture2D historyLength : register(t4); 
RWTexture2D<float4> resultColor : register(u0);
SamplerState samLinearClmap : register(s0);
  
[numthreads(DIMX, DIMY, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{ 
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
    
    int2 pixelCoord = dispatchThreadID.xy; 
    float currHistoryLength = historyLength[pixelCoord].x;
    float accumSpeed = AccumSpeed(currHistoryLength);
    
    ReCurrentBlur::Parameter param;
    param.Initialize(pixelCoord, accumSpeed, BLUR_RADIUS, color, viewZMap, normalMap, lightProp, samLinearClmap);
  
    resultColor[pixelCoord] = float4(ReCurrentBlur::PoissionBlur(param), param.centerColor.w);
}