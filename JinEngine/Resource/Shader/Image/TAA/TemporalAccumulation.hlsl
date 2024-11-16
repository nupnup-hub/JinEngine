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
#define USE_GROUP_BUFFER_FOR_TAA
#define SHARED_BUFFER_SIZE DIMX * DIMY  
  
#include"Common.hlsl"  
#include"../../Common/RandomNumberGenerator.hlsl"

Texture2D colorMap : register(t0);
Texture2D<float> viewZMap : register(t1);
Texture2D normalMap : register(t2);
Texture2D<float> preViewZMap : register(t3);
Texture2D preNormalMap : register(t4); 
Texture2D<float4> preHistory : register(t5); 
Texture2D lightProp : register(t6);
Texture2D preLightProp : register(t7);
RWTexture2D<float4> curHistory : register(u0);
SamplerState samPointClmap : register(s0);
SamplerState samLinearClmap : register(s1);
 
#define SKIP_VELOCITY 1e-06 //EPSILON 
//#define FAST_RESTORE_ERROR_FRAME FIXED_FRAME_COUNT + 1

//#define PACK_HISTORY_LENGTH(x) (float(x / 255.0f))
//#define UNPACK_HISTORY_LENGTH(x) (uint(x * 255.0f))
 
#define PASS_BICUBIC_MARK 0
#define PASS_BILINEAR_MARK 1
#define PASS_NON_VELOCITY 2
#define FAIL_MARK -1
#define COMMON_SPEED (1.0f / float(MAX_FRAME_ACCMURATION))
#define DISCARD_PRE_HISTORY_SPEED (1.0f)
#define BI_CUBIC_SPEED (COMMON_SPEED)
#define BI_LINEAR_SPEED (COMMON_SPEED * 2.5f)
#define COLOR_ERROR_SPEED BI_CUBIC_SPEED

void Reproject(in TA::PixelData input, out TAA::ReprojectionOut output)
{
    //motion, Hit Point Reproject을 거칠기에 따라 선택하는 기능 추가필요.. 
    //Hit Point Reproject는 hit distance에 정보가 필요.
    //반사되는 물체에 변화량과 비쳐지는 물체의 변화량이 다르기 때문에 고려할 필요가 있음. 
    output.preColor = float3(0, 0, 0);  
    output.accumSpeed = COMMON_SPEED;
    output.safetyLevel = FAIL_MARK;
    
    int2 prePixelCenterCoord = int2(input.preCenterUv * cb.common.rtSize);
    float4 preHistoryValue = preHistory.SampleLevel(samLinearClmap, input.preCenterUv, 0);
        
    TA::GeometryErrorResult result;
    TA::GeometryErrorEstimationActor actor = TA::CreateActor(input, cb.common, preViewZMap, preLightProp, preNormalMap, samPointClmap, samLinearClmap);
    TA::ComputeGeometryErrorEstimate(actor, result);
     
    output.isOutline = !result.canUseCubic;
    if (all(input.velocity == 0))
    {
        output.preColor = preHistoryValue.xyz;
        output.accumSpeed = COMMON_SPEED;
        output.safetyLevel = PASS_NON_VELOCITY; 
        return;
    } 
     
    //float maxVelocityFactor = floor(max(input.velocity.x, input.velocity.y) * 10) * 0.1f;
    //float maxVelocityFactor = max(input.velocity.x, input.velocity.y);
    //float speedOffset = float(maxVelocityFactor * (MAX_FRAME_ACCMURATION - 1)) * 0.0625f;
    
    if (result.canUseCubic)
    { 
        output.preColor = Catmul::Compute(preHistory, samLinearClmap, result.bicubicParameter);
        output.accumSpeed = BI_CUBIC_SPEED;
        output.safetyLevel = PASS_BICUBIC_MARK;
       // output.preColor = float3(0, 0, 1); 
    }
    else if (result.canUseBilinear)
    { 
        //output.preColor = Catmul::Compute(preHistory, samLinearClmap, result.bicubicParameter); 
        output.preColor = Bilinear::Compute(preHistory, samLinearClmap, result.bilinearParameter, actor.invRtSize, result.customWeight);
        output.accumSpeed = BI_LINEAR_SPEED;
        output.safetyLevel = PASS_BILINEAR_MARK;    
        //output.preColor = float3(1, 0, 0);
    }
    else
    {
        output.preColor = preHistoryValue.xyz; 
        output.accumSpeed = DISCARD_PRE_HISTORY_SPEED;
        output.safetyLevel = FAIL_MARK;
    }  
    output.accumSpeed = saturate(output.accumSpeed); 
} 
void ValidateHistoryColor(in TA::PixelData input, in TAA::ReprojectionOut repResult, out float4 newHistoryValue)
{
    float4 newPixelColor = colorMap.SampleLevel(samPointClmap, input.jitteredCenterUv, 0);
    TA::sharedColor[input.groupIndex] = float4(RGBToYCoCg(newPixelColor.xyz), 1.0f);
    GroupMemoryBarrierWithGroupSync();
  
    float3 newHistoryColor = float3(0, 0, 0);
    float3 preHistoryColor = repResult.preColor; 
    float accumSpeed = repResult.accumSpeed;
    
    TA::ColorErrorResult result;
    TA::ColorErrorEstimateActor actor = TA::CreateColorActorWithJittered(input, cb.common, newPixelColor.xyz, preHistoryColor, colorMap, samPointClmap, samLinearClmap);
     
    //Accum speed는 1.0f은 이는 이전 history 값을 폐기한다는 의미이므로 History에 ColorErrorEstimate 과정이 무의미해 진다.
    if (accumSpeed < DISCARD_PRE_HISTORY_SPEED)
    {    
        TA::ComputeColorErrorEstimate(actor, result);
        if (!result.isSafe)
            accumSpeed = COLOR_ERROR_SPEED;

        preHistoryColor = result.clampColor;      
        newPixelColor.xyz = lerp(preHistoryColor, newPixelColor.xyz, accumSpeed); 
    }
    
    if (accumSpeed >= BI_LINEAR_SPEED)
    {
        //Accum speed는 BI_LINEAR_SPEED이상이면 무조건 Velocity가 존재하는 경우이다.
        //Ghosting이 생길거라고 예상되며 따라서 Blur를 통해 어느정도 완화를 시도한다.
        newPixelColor.xyz = TA::GaiussianBlur(colorMap, samPointClmap, newPixelColor.xyz, input.centerUv, cb.common.invRtSize);
    }
    newHistoryValue = TAA::PackHistory(newPixelColor.xyz, repResult.isOutline);
}
[numthreads(DIMX, DIMY, 1)]
void main(int groupIndex : SV_GroupIndex, int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.common.rtSize.x || dispatchThreadID.y >= cb.common.rtSize.y)
        return; 
    
    TA::PixelData pixelData;
    pixelData.Initialize(dispatchThreadID.xy, groupIndex, cb.common, viewZMap, normalMap, lightProp, samPointClmap, samLinearClmap);
    pixelData.SetJitter(cb.common.haltonSampleNumber, cb.common.invRtSize);
    pixelData.SkipVelocity(SKIP_VELOCITY);
    
    TAA::ReprojectionOut output;
    Reproject(pixelData, output);
       
    float4 newHistoryValue;
    ValidateHistoryColor(pixelData, output, newHistoryValue);
    
    curHistory[pixelData.coord] = newHistoryValue;
}