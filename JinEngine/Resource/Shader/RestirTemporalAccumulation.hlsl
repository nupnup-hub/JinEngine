#pragma once 
#include"DepthFunc.hlsl"
#include"RestirDenoiseCommon.hlsl"
#include"Sampling.hlsl" 

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif 
#define VIEW_Z_THRESHOLD 10.0f
#define NORMAL_THRESHOLD 0.75f
#define DETERMIN_FILTER_SIZE 4
 
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
 
/*
bool IsValid(int2 coord, const float viewZ, const float preViewZ, const float viewZDelta, const float3 normal, const float3 preNormal)
{
    if (any(coord < int2(1, 1)) || any(coord > cb.rtSize - int2(1, 1)))
        return false;
    if ((abs(preViewZ - viewZ) / (viewZDelta + 1e-2f)) > VIEW_Z_THRESHOLD)
        return false;
    if (dot(normal, preNormal) < NORMAL_THRESHOLD)
        return false;
    
    return true;
}
*/

bool IsValid(float2 uv, const float viewZ, const float preViewZ, const float viewZDelta, const float3 normal, const float3 preNormal)
{
    if (any(uv < float2(0, 0)) || any(uv > float2(1, 1)))
        return false;
    if ((abs(preViewZ - viewZ) / (viewZDelta + 1e-2f)) > VIEW_Z_THRESHOLD)
        return false;
    if (dot(normal, preNormal) < NORMAL_THRESHOLD)
        return false;
    
    return true;
}
bool IsValidCoord(int2 coord)
{
    return all(coord > int2(1, 1)) && all(coord < cb.rtSize);
}
bool IsValidViewZ(const float viewZ, const float preViewZ, const float viewZDelta)
{
    return (abs(preViewZ - viewZ) / (viewZDelta + 1e-2f)) < VIEW_Z_THRESHOLD;
}
uint4 IsValidViewZ(const float viewZ, const float4 preViewZ, const float viewZDelta)
{
    float4 distance = abs(preViewZ - viewZ.xxxx) / (viewZDelta + 1e-2f); 
    return distance < VIEW_Z_THRESHOLD;
}
bool IsValidNormal(const float3 normal, const float3 preNormal)
{
    return dot(normal, preNormal) > NORMAL_THRESHOLD;
}

void DeterminSamplingMethod(const float2 preUv, const float viewZ, const float3 normal, const float2 velocity, float2 dxdy, out bool cubicPass, out bool bilinearPass)
{
    cubicPass = false;
    bilinearPass = false;
     
    float2 prePixelCoord = preUv * cb.rtSize; 
    bool isValidUv = IsValidCoord(prePixelCoord);
    if (!isValidUv)
        return;
    
    // bc - bicubic tap,
    // bl - bicubic & bilinear tap
    //
    // -- bc bc --
    // bc bl bl bc
    // bc bl bl bc
    // -- bc bc --
    
    // Gather sample ordering: (-,+),(+,+),(+,-),(-,-),
    int2 bilinearOrigin = int2(floor(prePixelCoord - 0.5f));
    float2 gatherOrigin00 = (float2(bilinearOrigin) + float2(0.0, 0.0)) * cb.invRtSize;
    float2 gatherOrigin10 = (float2(bilinearOrigin) + float2(2.0, 0.0)) * cb.invRtSize;
    float2 gatherOrigin01 = (float2(bilinearOrigin) + float2(0.0, 2.0)) * cb.invRtSize;
    float2 gatherOrigin11 = (float2(bilinearOrigin) + float2(2.0, 2.0)) * cb.invRtSize;
    
    float4 preViewZ00 = preViewZMap.GatherRed(samPointClmap, gatherOrigin00).wzxy;
    float4 preViewZ10 = preViewZMap.GatherRed(samPointClmap, gatherOrigin10).wzxy;
    float4 preViewZ01 = preViewZMap.GatherRed(samPointClmap, gatherOrigin01).wzxy;
    float4 preViewZ11 = preViewZMap.GatherRed(samPointClmap, gatherOrigin11).wzxy;
        
    float depthDelta = length(dxdy);    
    uint4 viewTest00 = IsValidViewZ(viewZ, preViewZ00, depthDelta);
    uint4 viewTest10 = IsValidViewZ(viewZ, preViewZ10, depthDelta);
    uint4 viewTest01 = IsValidViewZ(viewZ, preViewZ01, depthDelta);
    uint4 viewTest11 = IsValidViewZ(viewZ, preViewZ11, depthDelta);  
         
    cubicPass = (dot(viewTest00, viewTest10) * dot(viewTest01, viewTest11)) == 16;
    bilinearPass = (viewTest00.w + viewTest10.z + viewTest01.y + viewTest11.x) > 0;
    
    float2 offset = float2(1, 1) * cb.invRtSize;
    float2 preCenterUv = (floor(prePixelCoord) + 0.5f) * cb.invRtSize;
    float3 preNormalCenter = UnpackNormal(preNormalMap.SampleLevel(samLinearClmap, preCenterUv, 0));
    float3 preNormalLeftUp = UnpackNormal(preNormalMap.SampleLevel(samLinearClmap, preCenterUv + float2(-offset.x, -offset.y), 0));
    float3 preNormalLeftDown = UnpackNormal(preNormalMap.SampleLevel(samLinearClmap, preCenterUv + float2(-offset.x, offset.y), 0));
    float3 preNormalRightUp = UnpackNormal(preNormalMap.SampleLevel(samLinearClmap, preCenterUv + float2(offset.x, -offset.y), 0));
    float3 preNormalRightDown = UnpackNormal(preNormalMap.SampleLevel(samLinearClmap, preCenterUv + float2(offset.x, offset.y), 0));
    
    uint normalTest = 0;
    normalTest += IsValidNormal(normal, preNormalCenter);
    normalTest += IsValidNormal(normal, preNormalLeftUp);
    normalTest += IsValidNormal(normal, preNormalLeftDown);
    normalTest += IsValidNormal(normal, preNormalRightUp);
    normalTest += IsValidNormal(normal, preNormalRightDown);
  
    cubicPass = cubicPass && (normalTest == 5);
    bilinearPass = bilinearPass || (normalTest > 0);
}
bool DetermineDisOcclusion(const int2 pixelCoord, const float2 uv, const float viewZ, const float3 normal, const float2 velocity, float2 dxdy, out float4 preColor, out float4 preFastColor, out float currentHistory)
{
    preColor = float4(0, 0, 0, 0);
    preFastColor = float4(0, 0, 0, 0);
    currentHistory = 0;
    
    float2 preUv = uv + velocity;
    float2 prePixelCenterCoord = preUv * cb.rtSize; //pixelCoord + float2(0.5f, 0.5f);
    
    bool cubicPass;
    bool bilinearPass;
    DeterminSamplingMethod(preUv, viewZ, normal, velocity, dxdy, cubicPass, bilinearPass);
     
    if (cubicPass)
    {
        preColor = TextureSampling::CatmullRom(preColorHistory, samLinearClmap, preUv, cb.rtSize);
        preFastColor = TextureSampling::CatmullRom(preFastColorHistory, samLinearClmap, preUv, cb.rtSize);
        currentHistory = preHistoryLength.SampleLevel(samLinearClmap, preUv, 0); 
        //preColor = float4(1, 1, 0, 1);
        return true;
    }
    else if (bilinearPass)
    {
        preColor = TextureSampling::Bilinear(preColorHistory, samLinearClmap, prePixelCenterCoord, cb.invRtSize);
        preFastColor = TextureSampling::Bilinear(preFastColorHistory, samLinearClmap, prePixelCenterCoord, cb.invRtSize);
        currentHistory = preHistoryLength.SampleLevel(samLinearClmap, preUv, 0);
        //preColor = float4(1, 0, 0, 1);
        return true;
    }
    else
        return false;       //invalid prePixel
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
    float2 dxdy = depthDerivative.SampleLevel(samLinearClmap, uv, 0);
    
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
    bool isValid = DetermineDisOcclusion(pixelCoord, uv, viewZ, normal, velocity, dxdy, preColor, preFastColor, currHistoryLength);
    
    currHistoryLength = min(MAX_FRAME_ACCMURATION, currHistoryLength + 1.0f);
    // this adjusts the alpha for the case where insufficient history is available.
    // It boosts the temporal accumulation to give the samples equal weights in
    // the beginning. 
    //const float alpha = isValid ? max(ALPHA, 1.0 / currHistoryLength) : 1.0;
    
    const float accSpeed = 1.0f / MAX_FRAME_ACCMURATION;
    const float fastAccSpeed = 1.0f / MAX_FAST_FRAME_ACCMURATION;
    
    const float alpha = isValid ? max(accSpeed, 1.0f / currHistoryLength) : 1.0;
    const float fastAlpha = isValid ? max(fastAccSpeed, 1.0f / currHistoryLength) : 1.0;
 
    const float3 newColor = lerp(preColor.xyz, color, alpha);
    const float3 newFastColor = lerp(preFastColor.xyz, color, fastAlpha);

    colorHistory[pixelCoord] = float4(newColor, ComputeColorVariance(newColor));
    fastColorHistory[pixelCoord] = float4(newFastColor, ComputeColorVariance(newFastColor));
    //if (currHistoryLength < MAX_FAST_FRAME_ACCMURATION)
    //    colorHistory[pixelCoord].xyz = float3(1, 0, 0);
    historyLength[pixelCoord] = currHistoryLength;

}