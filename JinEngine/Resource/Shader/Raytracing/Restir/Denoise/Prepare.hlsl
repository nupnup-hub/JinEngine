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
 
#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif

Texture2D depthMap : register(t0);
Texture2D preDepthMap : register(t1);
RWTexture2D<float> linearDepthMap : register(u0);
RWTexture2D<float> preLinearDepthMap : register(u1);
RWTexture2D<float2> depthDerivative : register(u2);
 
[numthreads(DIMX, DIMY, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
       
    float depth = depthMap[dispatchThreadID.xy].x; 
    float viewZ = NdcToViewPZ(depth, cb.camNearMulFar, cb.camNearFar);
    linearDepthMap[dispatchThreadID.xy] = viewZ;
     
    float preDepth = preDepthMap[dispatchThreadID.xy].x; 
    float preViewZ = NdcToViewPZ(preDepth, cb.camNearMulFar, cb.camNearFar);
    preLinearDepthMap[dispatchThreadID.xy] = preViewZ;
    
    //                x
    //        ----------------->
    //    |    x     [top]     x
    // y  |  [left]   DTiD   [right]
    //    v    x    [bottom]   x
    //
    uint2 top = clamp(dispatchThreadID.xy + uint2(0, -1), 0, cb.rtSize - 1);
    uint2 bottom = clamp(dispatchThreadID.xy + uint2(0, 1), 0, cb.rtSize - 1);
    uint2 left = clamp(dispatchThreadID.xy + uint2(-1, 0), 0, cb.rtSize - 1);
    uint2 right = clamp(dispatchThreadID.xy + uint2(1, 0), 0, cb.rtSize - 1);

    float centerValue = viewZ;
    float2 backwardDifferences = centerValue - float2(depthMap[left].x, depthMap[top].x);
    float2 forwardDifferences = float2(depthMap[right].x, depthMap[bottom].x) - centerValue;

    centerValue = NdcToViewPZ(centerValue, cb.camNearMulFar,cb.camNearFar);
    backwardDifferences.x = NdcToViewPZ(backwardDifferences.x, cb.camNearMulFar, cb.camNearFar);
    backwardDifferences.y = NdcToViewPZ(backwardDifferences.y, cb.camNearMulFar, cb.camNearFar);
    forwardDifferences.x = NdcToViewPZ(forwardDifferences.x, cb.camNearMulFar, cb.camNearFar);
    forwardDifferences.y = NdcToViewPZ(forwardDifferences.y, cb.camNearMulFar, cb.camNearFar);
    
    // Calculates partial derivatives as the min of absolute backward and forward differences. 

    // Find the absolute minimum of the backward and foward differences in each axis
    // while preserving the sign of the difference.
    float2 ddx = float2(backwardDifferences.x, forwardDifferences.x);
    float2 ddy = float2(backwardDifferences.y, forwardDifferences.y);

    uint2 minIndex =
    {
        GetIndexOfValueClosestToTheReference(0, ddx),
        GetIndexOfValueClosestToTheReference(0, ddy)
    };
    float2 ddxy = float2(ddx[minIndex.x], ddy[minIndex.y]);

    // Clamp ddxy to a reasonable value to avoid ddxy going over surface boundaries
    // on thin geometry and getting background/foreground blended together on blur.
    float maxDdxy = 1;
    float2 _sign = sign(ddxy);
    ddxy = _sign * min(abs(ddxy), maxDdxy);

    depthDerivative[dispatchThreadID.xy] = ddxy;
}