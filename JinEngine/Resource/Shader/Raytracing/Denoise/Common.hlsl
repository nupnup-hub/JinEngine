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
#include"../../Common/GBufferCommon.hlsl"  
 
//test code
#define POISSON_SAMPLE_COUNT 8

//blur process
#ifdef POISSON_SAMPLE_COUNT
#include"../../Common/RandomNumberGenerator.hlsl" 
#include"../../Common/PixelCommon.hlsl" 
#ifndef DIFFUSE_BLUR
#define DIFFUSE_BLUR
#endif
#endif

#define USE_RECURRENT
#define MAX_FAST_FRAME_ACCMURATION 6
#define MAX_FRAME_ACCMURATION 60  
#include"../../Common/TemporalAccumulationCommon.hlsl"

struct DenoiseConstants
{
    float4x4 camView;
    float4x4 camProj;
    float4x4 camInvView;
    float4x4 camPreViewProj;
    float2 rtSize;
    float2 invRtSize;
    float2 uvToViewA;
    float2 uvToViewB;
    float2 camNearFar;
    float camNearMulFar;
    float denoiseRange;
    float baseRadius;
    float radiusRange;
    uint sampleNumber;
    uint pad00;
};

ConstantBuffer<DenoiseConstants> cb : register(b0);
 
  
/* 
reference :
- s22699 Fast Denoising with Self Stabilizing Recurrent Blurs
- Raytracing gems2 ch 48 ReBlur
- 2023 ReSTIR_Course_Cyberpunk_2077_Integration
*/
namespace ReCurrentBlur
{
    struct Parameter
    {
        int2 pixelCoord;
        float2 centerUv;
        float4 centerColor;
        float3 centerPosV;          
        float3 centerNormal;
        float centerRoughness;
        float centerViewZ;

        float planeDistNorm;
        float accumSpeed;
        float radius;
        Texture2D colorMap; 
        Texture2D<float> viewZMap;
        Texture2D normalMap;            //g-buffer layer
        Texture2D lightPropMap;         //g-buffer layer
        SamplerState samLinearClamp;
        
        void Initialize(int2 _pixelCoord,
            float _accumSpeed,
            float _radius,
            Texture2D _colorMap,
            Texture2D<float> _viewZMap,
            Texture2D _normalMap,
            Texture2D _lightPropMap,
            SamplerState _samLinearClamp)
        {     
            pixelCoord = _pixelCoord;
            
            // where planeDistNorm = accumSpeedFactor / ( 1.0 + centerZ );
            // It represents { 1 / "max possible allowed distance between a point and the plane" }
            planeDistNorm = _accumSpeed / (1.0 + centerViewZ);
            accumSpeed = _accumSpeed;
            radius = _radius;
            colorMap = _colorMap;
            viewZMap = _viewZMap;
            normalMap = _normalMap;
            lightPropMap = _lightPropMap;
            samLinearClamp = _samLinearClamp;  
            
            centerUv = (pixelCoord + 0.5f) * cb.invRtSize;
            centerColor = colorMap.SampleLevel(samLinearClamp, centerUv, 0);   
            centerViewZ = viewZMap.SampleLevel(samLinearClamp, centerUv, 0);
            centerNormal = UnpackNormal(normalMap.SampleLevel(samLinearClamp, centerUv, 0));           
            centerRoughness = UnpackRoughness(lightPropMap.SampleLevel(samLinearClamp, centerUv, 0));
     
            centerPosV = GetViewPos(centerUv, centerViewZ, cb.uvToViewA, cb.uvToViewB);
        }
        
        float GetGeometryWeight(float3 samplePosV)
        { 
            return ReCurrent::GetGeometryWeight(centerPosV, samplePosV, centerNormal, planeDistNorm);
        }
        float GetNormalWeight(float3 sampleNormal, float sampleRoughness)
        {  
            return ReCurrent::GetNormalWeight(centerNormal, sampleNormal, sampleRoughness, accumSpeed);
        }
        float GetRoughnessWeight(float sampleRoughness)
        { 
            return ReCurrent::GetRoughnessWeight(centerRoughness, sampleRoughness);
        }
        void Compute(float2 uv, inout float3 colorSum, inout float weightSum)
        {
            float3 sampleColor = colorMap.SampleLevel(samLinearClamp, uv, 0).xyz;
            float sampleViewZ = viewZMap.SampleLevel(samLinearClamp, uv, 0);
            float3 sampleNormalW = UnpackNormal(normalMap.SampleLevel(samLinearClamp, uv, 0));
            float sampleRoughness = UnpackRoughness(lightPropMap.SampleLevel(samLinearClamp, uv, 0));
            float3 samplePosV = GetViewPos(uv, sampleViewZ, cb.uvToViewA, cb.uvToViewB);
                     
            float weight = IsValidUv(uv);
            weight *= GetGeometryWeight(samplePosV);
            weight *= GetNormalWeight(sampleNormalW, sampleRoughness);
            weight *= GetRoughnessWeight(sampleRoughness);
            
            colorSum += sampleColor * weight;
            weightSum += weight;
        }
    };
    
    float2 GetSampleCoordinates(float3 posV, float2 offset, float3 T, float3 B)
    {   
        posV += T * offset.x + B * offset.y;
        float3 clip = mul(float4(posV, 1.0f), cb.camProj).xyw;
        clip.xy /= clip.z;
        clip.y = -clip.y;

        float2 uv = clip.xy * 0.5f + 0.5f;
        return uv;
    }
    float3 PoissionBlur(Parameter param)
    {
#ifndef POISSON_SAMPLE_COUNT
        return parameter.centerColor;
#else
        
#endif
        float2 centerUv = CalculateCenterUv(param.pixelCoord, cb.invRtSize);
        float3 colorSum = param.centerColor.xyz;
        float weightSum = 1.0f;
        
        RandomNumberGenerator rng;
        rng.Initialize(param.pixelCoord, cb.sampleNumber);
        float2 trigger = RandomRotationTrig(rng.Random01());
         
        float3 normalV = mul(param.centerNormal, (float3x3)cb.camView);
#ifdef DIFFUSE_BLUR
        float2x3 basis = ReCurrent::GetDisffuseKernelBasis(normalV, param.radius);
#else     
#endif 
        [unroll]
        for (uint i = 0; i < POISSON_SAMPLE_COUNT; ++i)
        {
            //poisson + random direction rotate
            float3 poisson = GetPoisson(i);
             
            //Per pixel kernel rotation
            //Input signal is already noisy 
            float2 offset = Rotate(poisson.xy, trigger);
            float2 uv = GetSampleCoordinates(param.centerPosV, offset, basis[0], basis[1]);
   
            param.Compute(uv, colorSum, weightSum);
        }
        return colorSum / weightSum;
    }
} 

/*
 // N = surface normal for the current frame
 // Nprev = surface normal for the previous frame
 // prevViewZ = viewZ in the 2x2 footprint for the previous frame
float4 ComputeDisocclusion2x2(float threshold, float isInScreen, float3 Xprev, float3 normal, float3 preNormal, float invDistToPoint, float4 prevViewZ)
{
    threshold = lerp(-1.0, threshold, isInScreen);
    float NoXprev1 = abs(dot(Xprev, normal));
    float NoXprev2 = abs(dot(Xprev, preNormal));
    float NoXprev = max(NoXprev1, NoXprev2) * invDistToPoint;
    float Zprev = mul(mWorldToViewPrev, float4(Xprev, 1.0)).z;
    float NoVprev = NoXprev / abs(Zprev);
    float4 relativePlaneDist = abs(NoVprev * abs(prevViewZ) - NoXprev);
    return step(relativePlaneDist, threshold);
}
*/