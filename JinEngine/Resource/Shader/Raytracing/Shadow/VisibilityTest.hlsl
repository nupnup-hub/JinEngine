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
#include "Common.hlsl"
#include "../../Common/TemporalAccumulationCommon.hlsl"


struct RayPayload
{
    float hitDistance;
};

Texture2D preShadowHistory : register(t0);
Texture2D<float> viewZMap : register(t1);
Texture2D normalMap : register(t2);
Texture2D<float> preViewZMap : register(t3);
Texture2D preNormalMap : register(t4);
Texture2D<float2> depthDerivative : register(t5);  
Texture2D lightProp : register(t9);
Texture2D preLightProp : register(t10);
RWTexture2D<float4> shadowHistory : register(u0); 
SamplerState samPointClmap : register(s0);
SamplerState samLinearClmap : register(s1);

bool DetermineDisOcclusion(const int2 pixelCoord, const float2 uv, const float3 posW, const float3 normal, const float viewZ, const float2 velocity, out float4 preColor, out float4 preFastColor, out uint currentHistory)
{
    preColor = float4(0, 0, 0, 0);
    preFastColor = float4(0, 0, 0, 0);
    currentHistory = 0;
    
    float2 preUv = uv + velocity.xy;
    float2 prePixelCenterCoord = preUv * cb.rtSize; //pixelCoord + float2(0.5f, 0.5f);
 
    uint materialID = UnpackMaterialID(lightProp.SampleLevel(samPointClmap, uv, 0));
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
            (float3x3) cb.camPreInvView,
            cb.preUvToViewA,
            cb.preUvToViewB,
            preViewZMap,
            preLightPropMap,
            preNormalMap,
            samPointClamp,
            samLinearClmap);
    
    TA::Result result;
    TA::ComputeCubicWeight(actor, result);
    
    result.canUseBilinear &= all(abs(velocity) <= 0.001f);
    if (result.canUseCubic)
    { 
        //preColor = float4(0, 0, 1, 1);
        //preFastColor = float4(0, 0, 1, 1);
        return true;
    }
    else if (result.canUseBilinear)
    { 
        //preColor = float4(1, 0, 0, 1);
        //preFastColor = float4(1, 0, 0, 1);
        return true;
    }
    else
        return false; //invalid prePixel
}
 
[shader("raygeneration")]
void RayGenShader()
{
    const uint2 pixelCoord = DispatchRaysIndex().xy;
    const uint pixelIndex = pixelCoord.y * DispatchRaysDimensions().x + pixelCoord.x;
    uint sampleSetIndex = 0;
 
    float2 uv = (pixelCoord + float2(0.5f, 0.5f)) * cb.halfInvRtSize;
 
}
[shader("closesthit")]
void HitShader(inout RayPayload rayPayload, in BuiltInTriangleIntersectionAttributes attr)
{
 
}
[shader("miss")]
void MissShader(inout RayPayload rayPayload)
{ 
}
