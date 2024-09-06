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
  
Texture2D colorMap : register(t0);
Texture2D<float> viewZMap : register(t1);
Texture2D normalMap : register(t2);
Texture2D<float> preViewZMap : register(t3);
Texture2D preNormalMap : register(t4); 
Texture2D<float4> preColorHistory : register(t5);
Texture2D<float4> preFastColorHistory : register(t6);
Texture2D<uint> preHistoryLength : register(t7);
Texture2D lightProp : register(t8);
Texture2D preLightProp : register(t9); 
RWTexture2D<float4> colorHistory : register(u0);
RWTexture2D<float4> fastColorHistory : register(u1);
RWTexture2D<uint> historyLength : register(u2);
SamplerState samPointClmap : register(s0);
SamplerState samLinearClmap : register(s1);

//temporal accumulation에 구현은 denoiser algorithm에 종속적이다.
//우선은 Svgf에 사용된 구현을 참조 결과를 관찰하며
//추후에 수정하도록한다.

//Relax와 TAA 참조하여 수정.

#define SKIP_VELOCITY 1e-06 //1e-07
#define FAST_RESTORE_BILINEAR_FRAME MAX_FAST_FRAME_ACCMURATION + 1
#define FAST_RESTORE_DISOCCLUSION_FRAME 2
#define PREVENT_THIN_OBJECT_FLICK_SPEED 0.05f

void Reproject(in ReprojectionIn input, out ReprojectionOut output)
{  
    output.preColor = float4(0, 0, 0, 0);
    output.preFastColor = float4(0, 0, 0, 0);
    output.curHistoryLength = 0;
    output.minAccumSpeed = 0.0f; 
    
    int2 prePixelCenterCoord = int2(input.preUv * cb.rtSize);
    uint curHistoryLength = preHistoryLength[prePixelCenterCoord].x;
    
    if (all(input.velocity == 0))
    {
        output.preColor = preColorHistory.SampleLevel(samLinearClmap, input.preUv, 0);
        output.preFastColor = preFastColorHistory.SampleLevel(samLinearClmap, input.preUv, 0);
        output.curHistoryLength = curHistoryLength + 1;
        return; 
    }
    
    TA::GeometryErrorResult result;
    TA::GeometryErrorEstimationActor actor = RestirTA::CreateActor(input, preViewZMap, preLightProp, preNormalMap, samPointClmap, samLinearClmap);
    TA::ComputeGeometryErrorEstimate(actor, result);
     
    if (result.canUseCubic)
    { 
        output.preColor = Catmul::Compute(preColorHistory, samLinearClmap, result.bicubicParameter);
        output.preFastColor = Catmul::Compute(preFastColorHistory, samLinearClmap, result.bicubicParameter);
        output.curHistoryLength = curHistoryLength + 1;
        //output.preColor = float4(0, 0, 0, 1);
        //output.preFastColor = float4(0, 0, 0, 1);      
    }
    else if (result.canUseBilinear)
    {
        //output.preColor = Bilinear::Compute(preColorHistory, samLinearClmap, result.bilinearParameter, actor.invRtSize, result.customWeight);
        //output.preFastColor = Bilinear::Compute(preFastColorHistory, samLinearClmap, result.bilinearParameter, actor.invRtSize, result.customWeight); 
        output.preColor = Catmul::Compute(preColorHistory, samLinearClmap, result.bicubicParameter);
        output.preFastColor = Catmul::Compute(preFastColorHistory, samLinearClmap, result.bicubicParameter);
        output.curHistoryLength = curHistoryLength + 1;
        //output.curHistoryLength = curHistoryLength + 1;
    }
    else
    {
        output.preColor = preColorHistory.SampleLevel(samLinearClmap, input.preUv, 0);
        output.preFastColor = preFastColorHistory.SampleLevel(samLinearClmap, input.preUv, 0);
        output.curHistoryLength = 1;
        output.minAccumSpeed = 1.0f;
    }
}

[numthreads(DIMX, DIMY, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
   
    int2 pixelCoord = dispatchThreadID.xy;
    float2 uv = (pixelCoord + float2(0.5f, 0.5f)) * cb.invRtSize;
    
    float3 pixelColor = colorMap.SampleLevel(samPointClmap, uv, 0).xyz;
    float viewZ = viewZMap.SampleLevel(samLinearClmap, uv, 0);
 
    float3 normal = UnpackNormal(normalMap.SampleLevel(samLinearClmap, uv, 0));
    uint materialID = UnpackMaterialID(lightProp.SampleLevel(samPointClmap, uv, 0));
    
    double3 posV = UVToViewSpace(uv, viewZ, cb.uvToViewA, cb.uvToViewB);
    double3 posW = mul(float4(posV, 1.0f), cb.camInvView).xyz;
    double4 prePosH = mul(float4(posW, 1.0f), cb.camPreViewProj);
    double2 preUv = double2(prePosH.xy / prePosH.w) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    double2 velocity = preUv - uv;
    
    //부동소수점 오차 교정
    if (abs(velocity.x) <= SKIP_VELOCITY)
        velocity.x = 0;
    if (abs(velocity.y) <= SKIP_VELOCITY)
        velocity.y = 0;
    preUv = velocity + uv;
    velocity = abs(velocity);
 
    ReprojectionIn input;
    input.preUv = preUv;
    input.curCenterPosW = posW;
    input.curCenterNormalW = normal;
    input.curCenterViewZ = viewZ;
    input.curCenterMaterialID = materialID;
    input.velocity = velocity;
    
    ReprojectionOut output; 
    Reproject(input, output);
     
    output.curHistoryLength = min(MAX_FRAME_ACCMURATION, output.curHistoryLength);

    const float accSpeed = max(AccumSpeed(output.curHistoryLength), output.minAccumSpeed);
    const float fastAccSpeed = max(FastAccumSpeed(output.curHistoryLength), output.minAccumSpeed);
    
    //Huristic
    //카메라 회전 혹은 Dynamic object가 움직일시 어떠한 수를 적용한다고 하더라도 레이트레이싱의 입력에 노이즈가 껴있는 이상
    //수 프레임은 오류가 누적되서 번져나가며 Restir Gi는 검은색 오류가 번져나간다.
    //이를 조금 더 밝은 색으도 대체하면 눈에 거슬리는 정도가 줄어든다.
    if (output.curHistoryLength < 2)
        pixelColor = max(pixelColor, float3(0.2f, 0.2f, 0.2f));
    
    float3 newHistoryColor = lerp(output.preColor.xyz, pixelColor, accSpeed);
    float3 newFastHistoryColor = lerp(output.preFastColor.xyz, pixelColor, fastAccSpeed);
 
    colorHistory[pixelCoord] = float4(newHistoryColor, ComputeColorVariance(newHistoryColor));
    fastColorHistory[pixelCoord] = float4(newFastHistoryColor, ComputeColorVariance(newFastHistoryColor));
    historyLength[pixelCoord] = output.curHistoryLength;
}