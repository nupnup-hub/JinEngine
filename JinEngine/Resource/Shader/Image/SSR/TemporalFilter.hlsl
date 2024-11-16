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
 
#define MAX_SAMPLE_COUNT 4
#define MAX_FRAME_ACCMURATION MAX_SAMPLE_COUNT  

#include"Common.hlsl"  
 
ConstantBuffer<SSRData> cb : register(b0);
Texture2D colorMap : register(t0);
Texture2D<float> viewZMap : register(t1);
Texture2D<float> preViewZMap : register(t2);
Texture2D normalMap : register(t3);
Texture2D preNormalMap : register(t4);
Texture2D lightProp : register(t5);
Texture2D preLightProp : register(t6);
Texture2D<float4> preHistory : register(t7);
RWTexture2D<float4> curHistory : register(u0);
SamplerState samPointClmap : register(s0);
SamplerState samLinearClmap : register(s1);
  
struct ReprojectionOut
{
    float4 preColor; 
    float accumSpeed;
    int safetyLevel; //0 pass bicubic, 1 pass bilinear, 2 fail
    bool isOutline;
};
 

#define SKIP_VELOCITY 1e-06 
 
#define PASS_BICUBIC_MARK 0
#define PASS_BILINEAR_MARK 1
#define PASS_NON_VELOCITY 2
#define FAIL_MARK -1
#define COMMON_SPEED (1.0f / float(MAX_FRAME_ACCMURATION))
#define DISCARD_PRE_HISTORY_SPEED (1.0f)
#define BI_CUBIC_SPEED (COMMON_SPEED)
#define BI_LINEAR_SPEED (COMMON_SPEED * 2.5f)
#define COLOR_ERROR_SPEED BI_CUBIC_SPEED
 
void Reproject(in TA::PixelData input, out ReprojectionOut output)
{
    //motion, Hit Point Reproject을 거칠기에 따라 선택하는 기능 추가필요.. 
    //Hit Point Reproject는 hit distance에 정보가 필요.
    //반사되는 물체에 변화량과 비쳐지는 물체의 변화량이 다르기 때문에 고려할 필요가 있음. 
    output.preColor = float4(0, 0, 0, 0);
    output.accumSpeed = COMMON_SPEED;
    output.safetyLevel = FAIL_MARK;
    
    int2 prePixelCenterCoord = int2(input.preCenterUv * cb.ta.rtSize);
    float4 preHistoryValue = preHistory.SampleLevel(samLinearClmap, input.preCenterUv, 0);
        
    TA::GeometryErrorResult result;
    TA::GeometryErrorEstimationActor actor = TA::CreateActor(input, cb.ta, preViewZMap, preLightProp, preNormalMap, samPointClmap, samLinearClmap);
    TA::ComputeGeometryErrorEstimate(actor, result);
     
    output.isOutline = !result.canUseCubic;
    if (all(input.velocity == 0))
    {
        output.preColor = preHistoryValue;
        output.accumSpeed = COMMON_SPEED;
        output.safetyLevel = PASS_NON_VELOCITY;
        return;
    }

    if (result.canUseCubic)
    {
        output.preColor = Catmul::Compute(preHistory, samLinearClmap, result.bicubicParameter);
        output.accumSpeed = BI_CUBIC_SPEED ;
        output.safetyLevel = PASS_BICUBIC_MARK; 
    }
    else if (result.canUseBilinear)
    {
        output.preColor = Bilinear::Compute(preHistory, samLinearClmap, result.bilinearParameter, actor.invRtSize, result.customWeight);
        //output.preColor = Catmul::Compute(preHistory, samLinearClmap, result.bicubicParameter);
        output.accumSpeed = BI_LINEAR_SPEED;
        output.safetyLevel = PASS_BILINEAR_MARK; 
    }
    else
    {
        output.preColor = preHistoryValue;
        output.accumSpeed = DISCARD_PRE_HISTORY_SPEED;
        output.safetyLevel = FAIL_MARK;
    }
    output.accumSpeed = saturate(output.accumSpeed);
}
void ValidateHistoryColor(in TA::PixelData input, in ReprojectionOut repResult, out float4 newHistoryValue)
{
    float4 newPixelColor = colorMap.SampleLevel(samLinearClmap, input.centerUv, 0);
    float accumSpeed = repResult.accumSpeed; 
    
    newPixelColor = lerp(repResult.preColor, newPixelColor, accumSpeed);
    if (accumSpeed >= BI_LINEAR_SPEED)
    {
        //Accum speed는 BI_LINEAR_SPEED이상이면 무조건 Velocity가 존재하는 경우이다.
        //Ghosting이 생길거라고 예상되며 따라서 Blur를 통해 어느정도 완화를 시도한다.
        //newPixelColor.xyz = TA::GaiussianBlur(colorMap, samPointClmap, newPixelColor.xyz, input.centerUv, cb.ta.invRtSize);
    } 
    newHistoryValue = newPixelColor;

}
[numthreads(DIMX, DIMY, 1)]
void main(int groupIndex : SV_GroupIndex, int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.ta.rtSize.x || dispatchThreadID.y >= cb.ta.rtSize.y)
        return;
    
    TA::PixelData pixelData;
    pixelData.Initialize(dispatchThreadID.xy, groupIndex, cb.ta, viewZMap, normalMap, lightProp, samPointClmap, samLinearClmap);
    pixelData.SkipVelocity(SKIP_VELOCITY);
    
    ReprojectionOut output;
    Reproject(pixelData, output);
       
    float4 newHistoryValue;
    ValidateHistoryColor(pixelData, output, newHistoryValue);
    
    curHistory[pixelData.coord] = newHistoryValue;
}