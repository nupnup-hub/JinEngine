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
#include"../../../Common/TemporalAccumulationCommon.hlsl"

struct ReserviorDenoiseConstants
{
    float4x4 camInvView;
    float4x4 camPreInvView;
    float4x4 camPreViewProj;
    
    float2 rtSize;
    float2 invRtSize;
    
    float2 uvToViewA;
    float2 uvToViewB;
    
    float2 preUvToViewA;
    float2 preUvToViewB;
    
    float2 camNearFar;
    float camNearMulFar;
    float denoiseRange;
    
    float baseRadius;
    float radiusRange;
    uint sampleNumber;
    uint pad00;
};

ConstantBuffer<ReserviorDenoiseConstants> cb : register(b0);

#ifndef NORMAL_THRESHOLD
#define NORMAL_THRESHOLD 0.75f
#endif
#ifndef DISOCCLUSION_THRES_HOLD
#define DISOCCLUSION_THRES_HOLD 0.01f 
#endif 

namespace RestirTA
{ 
    TA::Actor CreateActor(float2 preUv,
        float3 centerPos,
        float3 centerNormal, 
        float centerViewZ,
        uint centerMaterialID,
        Texture2D<float> preViewZMap,
        Texture2D preLightPropMap,
        Texture2D preNormalMap,
        SamplerState samPointClamp,
        SamplerState samLinearClmap)
    {
        float centerPlaneDist = dot(centerPos, centerNormal);
        TA::Actor actor;
        actor.Initialze(preUv,
            centerPos,
            centerNormal,
            centerMaterialID,
            cb.rtSize,
            cb.invRtSize,
            1.0f / centerPlaneDist,
            DISOCCLUSION_THRES_HOLD * (cb.camNearFar.y - cb.camNearFar.x),
            NORMAL_THRESHOLD,
            (float3x3)cb.camPreInvView,
            cb.preUvToViewA,
            cb.preUvToViewB,
            preViewZMap, 
            preLightPropMap,
            preNormalMap, 
            samPointClamp,
            samLinearClmap);
        
        return actor;
    }
}