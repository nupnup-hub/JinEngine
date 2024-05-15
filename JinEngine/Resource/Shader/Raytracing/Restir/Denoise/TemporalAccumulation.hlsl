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
Texture2D<float2> depthDerivative : register(t5);
Texture2D<float4> preColorHistory : register(t6);
Texture2D<float4> preFastColorHistory : register(t7);
Texture2D<float> preHistoryLength : register(t8);
RWTexture2D<float4> colorHistory : register(u0);
RWTexture2D<float4> fastColorHistory : register(u1);
RWTexture2D<float> historyLength : register(u2);
SamplerState samPointClmap : register(s0);
SamplerState samLinearClmap : register(s1);

//temporal accumulation에 구현은 denoiser algorithm에 종속적이다.
//우선은 Svgf에 사용된 구현을 참조 결과를 관찰하며
//추후에 수정하도록한다.
  
bool DetermineDisOcclusion(const int2 pixelCoord, const float2 uv, const float3 normal, const float2 velocity, out float4 preColor, out float4 preFastColor, out float currentHistory)
{
    preColor = float4(0, 0, 0, 0);
    preFastColor = float4(0, 0, 0, 0);
    currentHistory = 0;
    
    float2 preUv = uv + velocity.xy;  
    float2 prePixelCenterCoord = preUv * cb.rtSize; //pixelCoord + float2(0.5f, 0.5f);
 
    TA::Result result;
    TA::Actor actor = RestirTA::CreateActor(preUv, normal, preViewZMap, preNormalMap, samPointClmap, samLinearClmap);
    TA::ComputeCubicWeight(actor, result);
    
    if (result.canUseCubic)
    {
        Catmul::Parameter param;
        param.Initialize(preUv, cb.rtSize, cb.invRtSize);
        
        preColor = Catmul::Compute(preColorHistory, samLinearClmap, param);
        preFastColor = Catmul::Compute(preFastColorHistory, samLinearClmap, param);
        currentHistory = preHistoryLength.SampleLevel(samLinearClmap, preUv, 0);
        //preColor = float4(0, 0, 1, 1);
        return true;
    }
    else if (result.canUseBilinear)
    { 
        preColor = CustomSampling::ComputeBilinear(preColorHistory, samLinearClmap, result.bilinearParameter, actor.invRtSize, result.customWeight);
        preFastColor = CustomSampling::ComputeBilinear(preFastColorHistory, samLinearClmap, result.bilinearParameter, actor.invRtSize, result.customWeight);
        currentHistory = preHistoryLength.SampleLevel(samLinearClmap, preUv, 0);
        //preColor = float4(1, 0, 0, 1);
        return true;
    }
    else
        return false; //invalid prePixel
}

[numthreads(DIMX, DIMY, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
   
    int2 pixelCoord = dispatchThreadID.xy;
    float2 uv = (pixelCoord + float2(0.5f, 0.5f)) * cb.invRtSize;
    
    float3 color = colorMap.SampleLevel(samLinearClmap, uv, 0).xyz;
    float viewZ = viewZMap.SampleLevel(samLinearClmap, uv, 0);
 
    float3 normal = UnpackNormal(normalMap.SampleLevel(samLinearClmap, uv, 0));
    //float2 velocity = UnpackVelocity(velocityMap[pixelCoord]).xy;
    //float2 dxdy = depthDerivative.SampleLevel(samLinearClmap, uv, 0);
    
    float3 posV = UVToViewSpace(uv, viewZ, cb.uvToViewA, cb.uvToViewB);
    float3 posW = mul(float4(posV, 1.0f), cb.camInvView).xyz;
    float4 prePosH = mul(float4(posW, 1.0f), cb.camPreViewProj);
    float2 preUv = (prePosH.xy / prePosH.w) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f); 
    float2 velocity = preUv - uv;
    
    //if (length(velocity) < 0.001f)
    //    velocity = float2(0, 0);
    float4 preColor;
    float4 preFastColor;
    float currHistoryLength;
    bool isValid = DetermineDisOcclusion(pixelCoord, uv, normal, velocity, preColor, preFastColor, currHistoryLength);
    
    currHistoryLength = min(MAX_FRAME_ACCMURATION, currHistoryLength + 1.0f);
    // this adjusts the alpha for the case where insufficient history is available.
    // It boosts the temporal accumulation to give the samples equal weights in
    // the beginning. 
    //const float alpha = isValid ? max(ALPHA, 1.0 / currHistoryLength) : 1.0;
     
    const float accSpeed = isValid ? AccumSpeed(currHistoryLength) : 1.0f;
    const float fastAccSpeed = isValid ? FastAccumSpeed(currHistoryLength) : 1.0f;
  
    float3 newColor = lerp(preColor.xyz, color, accSpeed);
    float3 newFastColor = lerp(preFastColor.xyz, color, fastAccSpeed);
 
    colorHistory[pixelCoord] = float4(newColor, ComputeColorVariance(newColor));
    fastColorHistory[pixelCoord] = float4(newFastColor, ComputeColorVariance(newFastColor));
    //if (currHistoryLength < MAX_FAST_FRAME_ACCMURATION)
    //    colorHistory[pixelCoord].xyz = float3(1, 0, 0);
    historyLength[pixelCoord] = currHistoryLength;
}