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
#define BI_CUBIC_SPEED (COMMON_SPEED* 2.5f)
#define BI_LINEAR_SPEED (COMMON_SPEED * 4.0f)
#define COLOR_ERROR_SPEED BI_CUBIC_SPEED

void Reproject(in ReprojectionIn input, out ReprojectionOut output)
{
    //motion, Hit Point Reproject을 거칠기에 따라 선택하는 기능 추가필요.. 
    //Hit Point Reproject는 hit distance에 정보가 필요.
    //반사되는 물체에 변화량과 비쳐지는 물체의 변화량이 다르기 때문에 고려할 필요가 있음. 
    output.preColor = float3(0, 0, 0);  
    output.accumSpeed = COMMON_SPEED;
    output.safetyLevel = FAIL_MARK;
    
    int2 prePixelCenterCoord = int2(input.preUv * cb.rtSize);
    float4 preHistoryValue = preHistory.SampleLevel(samLinearClmap, input.preUv, 0);
        
    TA::GeometryErrorResult result;
    TA::GeometryErrorEstimationActor actor = TAA::CreateActor(input, preViewZMap, preLightProp, preNormalMap, samPointClmap, samLinearClmap);
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
    float maxVelocityFactor = max(input.velocity.x, input.velocity.y);
    float speedOffset = float(maxVelocityFactor * (MAX_FRAME_ACCMURATION - 1)) * 0.0625f;
    
    if (result.canUseCubic)
    { 
        output.preColor = Catmul::Compute(preHistory, samLinearClmap, result.bicubicParameter);
        output.accumSpeed = BI_CUBIC_SPEED + speedOffset;
        output.safetyLevel = PASS_BICUBIC_MARK;
       // output.preColor = float3(0, 0, 1); 
    }
    else if (result.canUseBilinear)
    { 
        output.preColor = Catmul::Compute(preHistory, samLinearClmap, result.bicubicParameter);
        //Catmul::Compute(preHistory, samLinearClmap, result.bicubicParameter); 
        output.accumSpeed = BI_LINEAR_SPEED + speedOffset;
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
float3 Blur(const float3 centerColor, const float2 centerUv)
{ 
    //Accum speed는 1.0f 이므로 AA효과가 사라진다.
    //따라서 Blur로 AA를 대체.
        
    float weightSum = 0.25f;
    float3 colorSum = centerColor * weightSum;
        
    [unroll]
    for (int y = -TAA_COLOR_GAUS_BLUR_RADIUS; y <= TAA_COLOR_GAUS_BLUR_RADIUS; y++)
    {
        [unroll]
        for (int x = -TAA_COLOR_GAUS_BLUR_RADIUS; x <= TAA_COLOR_GAUS_BLUR_RADIUS; x++)
        {
            if (x == 0 && y == 0)
                continue;
                    
            float2 sampleUv = centerUv + float2(x, y) * cb.invRtSize;
            if (!IsValidUv(sampleUv))
                continue;
                    
            int factor = 2 - (abs(y) + abs(x));
            float weight = 0.0625f * (1 << factor);
                
            float3 sampleColor = colorMap.SampleLevel(samPointClmap, sampleUv, 0).xyz;
            colorSum += sampleColor * weight;
            weightSum += weight;
        }
    }
   // destMap[pixelCoord].xyz = float3(0,1, 0);
    return colorSum / weightSum;
}
void ValidateHistoryColor(in ValidateHistoryColorIn input, in ReprojectionOut repResult, out float4 newHistoryValue)
{
    float4 newPixelColor = colorMap.SampleLevel(samPointClmap, input.curJitterUv, 0);
    TA::sharedColor[input.groupIndex] = float4(RGBToYCoCg(newPixelColor.xyz), 1.0f);
    GroupMemoryBarrierWithGroupSync();
  
    float3 newHistoryColor = float3(0, 0, 0);
    float3 preHistoryColor = repResult.preColor; 
    float accumSpeed = repResult.accumSpeed;
    
    TA::ColorErrorResult result;
    TA::ColorErrorEstimateActor actor = TAA::CreateColorActor(input, newPixelColor.xyz, preHistoryColor, colorMap, samPointClmap, samLinearClmap);
     
    //Accum speed는 1.0f은 이는 이전 history 값을 폐기한다는 의미이므로 History에 ColorErrorEstimate 과정이 무의미해 진다.
    if (accumSpeed < DISCARD_PRE_HISTORY_SPEED)
    {    
        TA::ComputeColorErrorEstimate(actor, result);
        if (!result.isSafe)
            accumSpeed = COLOR_ERROR_SPEED;

        preHistoryColor = result.clampColor;      
        newPixelColor.xyz = lerp(preHistoryColor, newPixelColor.xyz, accumSpeed); 
    }
    
    if (accumSpeed > COMMON_SPEED)
    {
        //Accum speed는 COMMON_SPEED이상이면 무조건 Velocity가 존재하는 경우이다.
        //Ghosting이 생길거라고 예상되며 따라서 Blur를 통해 어느정도 완화를 시도한다.
        newPixelColor.xyz = Blur(newPixelColor.xyz, input.curCenterUv);
    }
    newHistoryValue = TAA::PackHistory(newPixelColor.xyz, repResult.isOutline);
}
[numthreads(DIMX, DIMY, 1)]
void main(int groupIndex : SV_GroupIndex, int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
   
    const float2 halton[MAX_SAMPLE_COUNT] =
    {
        //{0.5f, 0.5f },
        { 0.5f, 0.333333f },
        { 0.25f, 0.666667f },
        { 0.75f, 0.111111f },
        { 0.125f, 0.444444f },
        { 0.625f, 0.777778f },
        { 0.375f, 0.222222f },
        { 0.875f, 0.555556f },
        { 0.0625f, 0.888889f },
        { 0.5625f, 0.037037f },
        { 0.3125f, 0.37037f },
        { 0.8125f, 0.703704f },
        { 0.1875f, 0.148148f },
        { 0.6875f, 0.481481f },
        { 0.4375f, 0.814815f },
        { 0.9375f, 0.259259f },
        { 0.03125f, 0.592593f }
    }; 
      
    int2 pixelCoord = dispatchThreadID.xy; 
    float2 jitter = (halton[cb.sampleNumber] - float2(0.5f, 0.5f)) * 2.0f; 
    
    float2 jitterUv = float2(pixelCoord + float2(0.5f, 0.5f) + jitter) * cb.invRtSize;
    float2 centerUv = float2(pixelCoord + 0.5f) * cb.invRtSize;
    
    float viewZ = viewZMap.SampleLevel(samLinearClmap, centerUv, 0);
    float3 normal = UnpackNormal(normalMap.SampleLevel(samLinearClmap, centerUv, 0));
    uint materialID = UnpackMaterialID(lightProp.SampleLevel(samPointClmap, centerUv, 0));
 
    float3 posV = UVToViewSpace(centerUv, viewZ, cb.uvToViewA, cb.uvToViewB);
    float3 posW = mul(float4(posV, 1.0f), cb.camInvView).xyz;
    double4 prePosH = mul(float4(posW, 1.0f), cb.camPreViewProj);
    double2 preUv = (prePosH.xy / prePosH.w) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    double2 velocity = preUv - centerUv;
     
    //부동소수점 오차 교정
    //if (abs(velocity.x) <= SKIP_VELOCITY)
    //    velocity.x = 0;
    //if (abs(velocity.y) <= SKIP_VELOCITY)
     //   velocity.y = 0;
    
    //preUv = velocity + centerUv;
    //velocity = abs(velocity);
  
    ReprojectionIn input;
    input.preUv = preUv;
    input.curCenterPosW = posW;
    input.curCenterNormalW = normal;
    input.curCenterViewZ = viewZ;
    input.curCenterMaterialID = materialID;
    input.velocity = velocity;
    
    ReprojectionOut output;
    Reproject(input, output);
      
    ValidateHistoryColorIn vIn;
    vIn.groupIndex = groupIndex;    
    vIn.curJitterUv = jitterUv;
    vIn.curCenterUv = centerUv;
    vIn.preUv = preUv; 
 
    float4 newHistoryValue;
    ValidateHistoryColor(vIn, output, newHistoryValue);
    
    curHistory[pixelCoord] = newHistoryValue;   
}