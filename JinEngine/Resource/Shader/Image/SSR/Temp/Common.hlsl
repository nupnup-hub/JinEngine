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
#include"../../Common/CommonConstantsStructureDefine.hlsl" 
#include"../../Common/GBufferCommon.hlsl"
#include"../../Common/DepthFunc.hlsl"
#include"../../Common/PixelCommon.hlsl"

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif  
 
struct SSRData
{
    float4x4 camView;
    float4x4 camProj;
    float4x4 camInvView;
    
    float2 rtSize;
    float2 invRtSize;
     
    float2 uvToViewA;
    float2 uvToViewB;
    
    float2 nearFarZ;
    float nearFarMul;
    float startOffset;
    
    float3 camPosW;  
    float stepScale;
    
    float maxStepCount;
    float rayDistance; //0 ~ 10000   
    float thickness; //0 ~ 10000    
    float objectViewZBias; //0 ~ 10000
    
    float fadeDistance; //0 ~ 1.0f
    float fadeOneRate; //to [0.0f ~ 1.0f]
    int pad00;
    int pad01;
};