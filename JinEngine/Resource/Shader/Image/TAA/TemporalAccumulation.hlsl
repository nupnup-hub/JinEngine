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
Texture2D<float4> preColorHistory : register(t5); 
Texture2D lightProp : register(t6);
Texture2D preLightProp : register(t7);
RWTexture2D<float4> colorHistory : register(u0);  
SamplerState samPointClmap : register(s0);
SamplerState samLinearClmap : register(s1);
 
struct DetermineDisOcclusionIn
{  
    float2 preUv;
    float3 posW;
    float3 normal;
    float viewZ;
    uint materialID; 
}; 

struct ValidateHistoryColorIn
{
    int groupIndex;
    float2 uv;
    float2 preUv; 
    float3 preHistoryColor; 
};

bool DetermineDisOcclusion(const DetermineDisOcclusionIn input, out float3 preHistoryColor, out uint currentHistoryLength)
{
    preHistoryColor = float3(0, 0, 0); 
    currentHistoryLength = 0;
 
    //Geometry 차이에서 오는 Disocclusion 계산
    TA::GeometryErrorResult result;
    TA::GeometryErrorEstimationActor actor = TAA::CreateGeometryActor(input.preUv, input.posW, input.normal, input.viewZ, input.materialID, preViewZMap, preLightProp, preNormalMap, samPointClmap, samLinearClmap);
    TA::ComputeCubicWeight(actor, result); 
         
    float2 prePixelCenterCoord = input.preUv * cb.rtSize;
    if (result.canUseCubic)
    {
        preHistoryColor = Catmul::Compute(preColorHistory, samLinearClmap, result.bicubicParameter).xyz; 
        currentHistoryLength = preColorHistory[prePixelCenterCoord].w;
        //preColor = float4(0, 0, 1, 1);
        //preFastColor = float4(0, 0, 1, 1);
        return true;
    }
    else if (result.canUseBilinear)
    {
        preHistoryColor = CustomSampling::ComputeBilinear(preColorHistory, samLinearClmap, result.bilinearParameter, actor.invRtSize, result.customWeight).xyz;
        currentHistoryLength = preColorHistory[prePixelCenterCoord].w;
        //preColor = float4(1, 0, 0, 1);
        //preFastColor = float4(1, 0, 0, 1);
        return true;
    }
    else
        return false; //invalid prePixel
}
bool ValidateHistoryColor(const ValidateHistoryColorIn input, inout uint historyLength, out float3 newHistoryColor)
{
    const float3 newColor = colorMap.SampleLevel(samLinearClmap, input.uv, 0).xyz;
    TA::sharedColor[input.groupIndex] = float4(RGBToYCoCg(newColor), 1.0f);
    GroupMemoryBarrierWithGroupSync();
     
    TA::ColorErrorResult result;
    TA::ColorErrorEstimateActor actor = TAA::CreateColorActor(input.uv, input.preUv, MAX_FRAME_ACCMURATION, colorMap, preColorHistory, samPointClmap, samLinearClmap, input.groupIndex);
    const bool suc = TA::ComputeColorErrorEstimate(actor, result);
    const float3 clampColor = suc ? result.clampColor : preColorHistory.SampleLevel(samLinearClmap, input.preUv, 0).xyz;
    
    float accSpeed = AccumSpeed(historyLength); 
    if (!result.isSafe)
        accSpeed = max(accSpeed, result.minSpeed);
    newHistoryColor = lerp(clampColor, newColor, accSpeed);
 
    return result.isSafe;
}
[numthreads(DIMX, DIMY, 1)]
void main(int groupIndex : SV_GroupIndex, int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
   
    const float2 halton64[64] =
    {
        { 0.5, 0.333333 },
        { 0.25, 0.666667 },
        { 0.75, 0.111111 },
        { 0.125, 0.444444 },
        { 0.625, 0.777778 },
        { 0.375, 0.222222 },
        { 0.875, 0.555556 },
        { 0.0625, 0.888889 },
        { 0.5625, 0.037037 },
        { 0.3125, 0.37037 },
        { 0.8125, 0.703704 },
        { 0.1875, 0.148148 },
        { 0.6875, 0.481481 },
        { 0.4375, 0.814815 },
        { 0.9375, 0.259259 },
        { 0.03125, 0.592593 },
        { 0.53125, 0.925926 },
        { 0.28125, 0.0740741 },
        { 0.78125, 0.407407 },
        { 0.15625, 0.740741 },
        { 0.65625, 0.185185 },
        { 0.40625, 0.518519 },
        { 0.90625, 0.851852 },
        { 0.09375, 0.296296 },
        { 0.59375, 0.62963 },
        { 0.34375, 0.962963 },
        { 0.84375, 0.0123457 },
        { 0.21875, 0.345679 },
        { 0.71875, 0.679012 },
        { 0.46875, 0.123457 },
        { 0.96875, 0.45679 },
        { 0.015625, 0.790123 },
        { 0.515625, 0.234568 },
        { 0.265625, 0.567901 },
        { 0.765625, 0.901235 },
        { 0.140625, 0.0493827 },
        { 0.640625, 0.382716 },
        { 0.390625, 0.716049 },
        { 0.890625, 0.160494 },
        { 0.078125, 0.493827 },
        { 0.578125, 0.82716 },
        { 0.328125, 0.271605 },
        { 0.828125, 0.604938 },
        { 0.203125, 0.938272 },
        { 0.703125, 0.0864198 },
        { 0.453125, 0.419753 },
        { 0.953125, 0.753086 },
        { 0.046875, 0.197531 },
        { 0.546875, 0.530864 },
        { 0.296875, 0.864198 },
        { 0.796875, 0.308642 },
        { 0.171875, 0.641975 },
        { 0.671875, 0.975309 },
        { 0.421875, 0.0246914 },
        { 0.921875, 0.358025 },
        { 0.109375, 0.691358 },
        { 0.609375, 0.135802 },
        { 0.359375, 0.469136 },
        { 0.859375, 0.802469 },
        { 0.234375, 0.246914 },
        { 0.734375, 0.580247 },
        { 0.484375, 0.91358 },
        { 0.984375, 0.0617284 },
        { 0.0078125, 0.395062 },
    }; 
    
    int2 pixelCoord = dispatchThreadID.xy;
    
    RandomNumberGenerator rng;
    rng.Initialize(pixelCoord, cb.sampleNumber);
    
    int haltonIndex = min(int(rng.Random01() * 64), 63);
    float2 jitter = halton64[haltonIndex];
    
    float2 uv = (pixelCoord + jitter) * cb.invRtSize;
     
    float viewZ = viewZMap.SampleLevel(samLinearClmap, uv, 0);
    float3 normal = UnpackNormal(normalMap.SampleLevel(samLinearClmap, uv, 0));
    //float2 velocity = UnpackVelocity(velocityMap[pixelCoord]).xy;
    //float2 dxdy = depthDerivative.SampleLevel(samLinearClmap, uv, 0);
    
    float3 posV = UVToViewSpace(uv, viewZ, cb.uvToViewA, cb.uvToViewB);
    float3 posW = mul(float4(posV, 1.0f), cb.camInvView).xyz;
    float4 prePosH = mul(float4(posW, 1.0f), cb.camPreViewProj);
    float2 preUv = (prePosH.xy / prePosH.w) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    float2 velocity = preUv - uv;
     
    uint materialID = UnpackMaterialID(lightProp.SampleLevel(samPointClmap, uv, 0));
 
    DetermineDisOcclusionIn dInput;
    dInput.preUv = preUv;
    dInput.posW = posW;
    dInput.normal = normal;
    dInput.viewZ = viewZ;
    dInput.materialID = materialID;
     
    float3 preHistoryColor;
    uint currentHistoryLength;
    bool isValidGeometry = DetermineDisOcclusion(dInput, preHistoryColor, currentHistoryLength);
    currentHistoryLength = min(MAX_FRAME_ACCMURATION, currentHistoryLength + 1.0f);
    
    ValidateHistoryColorIn vIn;
    vIn.groupIndex = groupIndex;
    vIn.uv = uv;
    vIn.preUv = preUv;
    vIn.preHistoryColor = preHistoryColor; 
    
    float3 newHistoryColor;
    bool isSafeColor = ValidateHistoryColor(vIn, currentHistoryLength, newHistoryColor);
   
    colorHistory[pixelCoord] = float4(newHistoryColor, currentHistoryLength); 
}