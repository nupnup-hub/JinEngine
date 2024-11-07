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
#include"../Common/CommonConstantsStructureDefine.hlsl"
#include"../Common/DepthFunc.hlsl" 
 
#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif

ConstantBuffer<CameraData> cb : register(b0);
Texture2D depthMap : register(t0);
Texture2D preDepthMap : register(t1);
RWTexture2D<float> viewZMap : register(u0);
RWTexture2D<float> preViewZMap : register(u1);
RWTexture2D<float2> depthDerivativeMap : register(u2);
  
uint GetIndexOfValueClosestToTheReference(const float refValue, const float2 vValues)
{
    float2 delta = abs(refValue - vValues);
    uint outIndex = delta[1] < delta[0] ? 1 : 0;
    return outIndex;
}
uint GetIndexOfValueClosestToTheReference(const float refValue, const float4 vValues)
{
    float4 delta = abs(refValue - vValues);

    uint outIndex = delta[1] < delta[0] ? 1 : 0;
    outIndex = delta[2] < delta[outIndex] ? 2 : outIndex;
    outIndex = delta[3] < delta[outIndex] ? 3 : outIndex;

    return outIndex;
}

[numthreads(DIMX, DIMY, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.renderTargetSize.x || dispatchThreadID.y >= cb.renderTargetSize.y)
        return;
       
    float2 camNearFar = float2(cb.nearZ, cb.farZ);
    float camNearMulFar = camNearFar.x * camNearFar.y;
    
    float depth = depthMap[dispatchThreadID.xy].x;
    float viewZ = NdcToViewPZ(depth, camNearMulFar, camNearFar);
    viewZMap[dispatchThreadID.xy] = viewZ;
     
    float preDepth = preDepthMap[dispatchThreadID.xy].x;
    float preViewZ = NdcToViewPZ(preDepth, camNearMulFar, camNearFar);
    preViewZMap[dispatchThreadID.xy] = preViewZ;
    
    //                x
    //        ----------------->
    //    |    x     [top]     x
    // y  |  [left]   DTiD   [right]
    //    v    x    [bottom]   x
    //
    uint2 top = clamp(dispatchThreadID.xy + uint2(0, -1), 0, cb.renderTargetSize - 1);
    uint2 bottom = clamp(dispatchThreadID.xy + uint2(0, 1), 0, cb.renderTargetSize - 1);
    uint2 left = clamp(dispatchThreadID.xy + uint2(-1, 0), 0, cb.renderTargetSize - 1);
    uint2 right = clamp(dispatchThreadID.xy + uint2(1, 0), 0, cb.renderTargetSize - 1);

    float centerValue = viewZ;
    float2 backwardDifferences = centerValue - float2(depthMap[left].x, depthMap[top].x);
    float2 forwardDifferences = float2(depthMap[right].x, depthMap[bottom].x) - centerValue;

    centerValue = NdcToViewPZ(centerValue, camNearMulFar, camNearFar);
    backwardDifferences.x = NdcToViewPZ(backwardDifferences.x, camNearMulFar, camNearFar);
    backwardDifferences.y = NdcToViewPZ(backwardDifferences.y, camNearMulFar, camNearFar);
    forwardDifferences.x = NdcToViewPZ(forwardDifferences.x, camNearMulFar, camNearFar);
    forwardDifferences.y = NdcToViewPZ(forwardDifferences.y, camNearMulFar, camNearFar);
    
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

    depthDerivativeMap[dispatchThreadID.xy] = ddxy;
}