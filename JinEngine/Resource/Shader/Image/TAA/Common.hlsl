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
  
#define MAX_SAMPLE_COUNT 16
#define MAX_FRAME_ACCMURATION MAX_SAMPLE_COUNT  

#include"../../Common/TemporalAccumulationCommon.hlsl"

struct TAAPassConstants
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
    uint sampleNumber; 
};

ConstantBuffer<TAAPassConstants> cb : register(b0);

#ifndef NORMAL_THRESHOLD
#define NORMAL_THRESHOLD 0.5f
#endif
#ifndef DISOCCLUSION_THRES_HOLD
#define DISOCCLUSION_THRES_HOLD 0.01f 
#endif 
 struct ReprojectionIn
{
    float2 preUv;
    float3 curCenterPosW;
    float3 curCenterNormalW;
    float curCenterViewZ;
    uint curCenterMaterialID;
    double2 velocity;
};

struct ReprojectionOut
{
    float3 preColor; 
    //uint curHistoryLength;
    float accumSpeed; 
    int safetyLevel;                    //0 pass bicubic, 1 pass bilinear, 2 fail
    bool isOutline;
};

struct ValidateHistoryColorIn
{
    int groupIndex;
    float2 curCenterUv;
    float2 curJitterUv;
    float2 preUv; 
};

namespace TAA
{
    TA::GeometryErrorEstimationActor CreateActor(in ReprojectionIn pixel,
        Texture2D<float> preViewZMap,
        Texture2D preLightPropMap,
        Texture2D preNormalMap,
        SamplerState samPointClamp,
        SamplerState samLinearClmap)
    {
        float centerPlaneDist = dot(pixel.curCenterPosW, pixel.curCenterNormalW);
        TA::GeometryErrorEstimationActor actor;
        actor.Initialze(pixel.preUv,
            pixel.curCenterPosW,
            pixel.curCenterNormalW,
            pixel.curCenterMaterialID,
            cb.rtSize,
            cb.invRtSize,
            1.0f / centerPlaneDist,
            DISOCCLUSION_THRES_HOLD * (cb.camNearFar.y - cb.camNearFar.x),
            NORMAL_THRESHOLD,
            (float3x3) cb.camPreInvView,
            cb.preUvToViewA,
            cb.preUvToViewB,
            preViewZMap,
            preLightPropMap,
            preNormalMap,
            samPointClamp,
            samLinearClmap);
        
        return actor;
    }
    
    TA::ColorErrorEstimateActor CreateColorActor(in ValidateHistoryColorIn input,
            float3 curPixelCenterColor,
            float3 preHistoryCenterColor,
            Texture2D srcColorMap, 
            SamplerState samPointClamp,
            SamplerState samLinearClmap)
    {
        TA::ColorErrorEstimateActor actor;
        actor.Initialize(input.curJitterUv, input.preUv, curPixelCenterColor, preHistoryCenterColor, cb.invRtSize, srcColorMap, samPointClamp, samLinearClmap);
        actor.SetGroupIndex(input.groupIndex);
        
        return actor;
    };

    //History R16G16B16A16
    float4 PackHistory(const float3 color, const bool isOutline)
    {
        //0, 1, 2, 3
        uint optionMark = isOutline ? 1 : 0; 
        return float4(color, float(optionMark) / 65535.0f);
    }
    void UnPackHistory(in float4 historyValue, out float3 color, out bool isOutline)
    {
        color = historyValue.xyz;
        
        uint optionMark = historyValue.w * 65535.0f; 
        isOutline = optionMark & 1; 
    }

}