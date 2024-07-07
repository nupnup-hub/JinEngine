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
#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif

struct RaytracingShadowConstants
{
    float4x4 camInvView;
    float4x4 camPreViewProj;
    float2 camNearFar;
    float2 uvToViewA;
    float2 uvToViewB;
    float2 halfRtSize;
    float2 halfInvRtSize;
    float2 originalRtSize;
    float2 originalInvRtSize;
    float tMax;
    uint totalNumPixels; //mul rtSize.x * rtSize.y
    
    float3 camPosW;
    float camNearMulFar;
    float3 camPrePosW;
    uint sampleSetSize;
    uint sampleSetMax;
    uint currSampleSetIndex;
    uint updateCount;
    
    uint directionalLightRange;
    uint pointLightRange;
    uint spotLightRange;
    uint rectLightRange;
    
    uint directionalLightOffset;
    uint pointLightOffset;
    uint spotLightOffset;
    uint rectLightOffset;
    
    uint totalLightCount;
    float invTotalLightCount;
    uint forceClearPrevalue; ///< Clear temporal and spatial reservoirs. 
    uint pad00;
    uint pad01;
}; 

ConstantBuffer<RaytracingShadowConstants> cb : register(b0);



