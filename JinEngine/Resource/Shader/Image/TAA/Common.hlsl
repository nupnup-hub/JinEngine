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
    TACommonPassData common;
};

ConstantBuffer<TAAPassConstants> cb : register(b0);
 
namespace TAA
{  
    class ReprojectionOut
    {
        float3 preColor;
        float accumSpeed;
        int safetyLevel; //0 pass bicubic, 1 pass bilinear, 2 fail
        bool isOutline;
    };
    ReprojectionOut CreateReprojectionOut()
    {
        ReprojectionOut output;
        output.preColor = float3(0, 0, 0);
        output.accumSpeed = 0;
        output.safetyLevel = 0;
        
        return output;
    }  
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