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
#include"../../Common/FilterCommon.hlsl"  

#ifndef SSAO_SAMPLE_COUNT 
#define SSAO_SAMPLE_COUNT 16
#endif

#ifndef SSAO_RANDOM_MAP_SIZE 
#define SSAO_RANDOM_MAP_SIZE 4.0f
#endif
 
#ifndef SSAO_DIR_COUNT
#define SSAO_DIR_COUNT 8
#endif 
 
#ifndef USE_COMPUTE_SHADER
#include"../../Common/FullScreenTriangleVs.hlsl" 
#endif

struct SsaoPassData
{
    float4x4 camView;
    float4x4 camProj;
    float radius;
    float radius2;
    float bias;
    float sharpness;
    float2 camNearFar;
    float2 camRtSize;
    float2 camInvRtSize;
    float2 aoRtSize;
    float2 aoInvRtSize;
    float2 aoInvQuaterRtSize;
    float2 uvToViewA;
    float2 uvToViewB;
    float2 viewPortTopLeft;
    float radiusToScreen;
    float negInvR2;
    float tanBias;
    float smallScaleAOAmount;
    float largeScaleAOAmount;
    float camNearMulFar;
};

struct SsaoSliceData  
{
    float4 jitter;
    float2 posOffset;
    float sliceIndex;
    unsigned int uSliceIndex;
};  

struct SsaoSampleData  
{
    float4 sample[SSAO_SAMPLE_COUNT];
}; 

ConstantBuffer<SsaoPassData> cbPass : register(b0);
#ifdef USE_HBAO
ConstantBuffer<SsaoSliceData> cbSlice : register(b1);
#elif USE_SSAO
ConstantBuffer<SsaoSampleData> cbSample : register(b1);
#endif