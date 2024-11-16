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
#include"ReflectionData.hlsl"  

  
 
//Process
//1. Unpack texture data
//2. Create raymarching data
//3. Raymarching
//  3.1 Prepare ray info
//  3.2 Compute DDA
//  3.3 Loop
//  3.4 Binary search   [Optional]
//4. Sample color
 
#define DISTANCE_THRESHOLD 0.001f
#define ROW_DISTANCE_OFFSET 0.01f  

#ifndef BINARY_SEARCH_LOOP_COUNT 
#define BINARY_SEARCH_LOOP_COUNT 8
#endif

#ifdef USE_HI_Z
#else  
#define USE_BINARY_SEARCH
#endif

class RaymarchingActor
{
    SSR::HitData hit;           //16
    
    float2 pixelCoord;          //24
    float inverseW;             //28
    
    float2 stepDelta;           //36
    float inverseDelta;         //40
    
    float stepDir;              //44
    float end;                  //48
    
    float objectFrontViewZ;     //52
    float rayViewZ;             //56
    
    bool permute;               //60
    bool isHit;                 //64
    
    void Initialize(const float3 posV, const float3 dirV)
    {
        //3. Raymarching
    
        //3.1 Prepare ray info
        //V = view space
        //C = clip space
        //S = screen space     
        
        float rayLength = (posV.z + dirV.z * cb.rayDistance) < cb.ta.camNearFar.x ? ((cb.ta.camNearFar.x - posV.z) / dirV.z) : cb.rayDistance;
        float3 rayEndV = posV + dirV * rayLength;
    
        float4 rayBeginC = mul(float4(posV, 1.0f), cb.camProj);
        float4 rayEndC = mul(float4(rayEndV, 1.0f), cb.camProj);
     
        float rayBeginInverseW = 1.0f / rayBeginC.w;
        float rayEndInverseW = 1.0f / rayEndC.w;
     
        //Clip -> Ndc -> Screen
        float2 rayBeginS = (rayBeginC.xy * rayBeginInverseW * float2(0.5f, -0.5f) + 0.5f) * cb.halfRtSize;
        float2 rayEndS = (rayEndC.xy * rayEndInverseW * float2(0.5f, -0.5f) + 0.5f) * cb.halfRtSize;
        rayEndS += distance(rayEndS, rayBeginS) < DISTANCE_THRESHOLD ? ROW_DISTANCE_OFFSET : 0;
     
        //3.2 Compute DDA
        /*
            알고리즘.
            1. 기울기에 따라 기준 축 결정 ... delta < 1 => x, else y
            2. 기준 축을 따라 진행  ... x: x += 1, y += delta, y: y += 1, y += inverse(delta)
            3. 결과값이 실수일 경우 반올림. ... 기준 축이 아닌경우만 해당.
        
            항상 정수값을 반환하며 중간에 실수값을 취급하지 않는다.
        */
        permute = false;
        float2 delta = rayEndS - rayBeginS;
        if (abs(delta.x) < abs(delta.y))
        {
            permute = true;
            delta = delta.yx;
            rayBeginS = rayBeginS.yx;
            rayEndS = rayEndS.yx;
        }
    
        stepDir = sign(delta.x);
        float invDeltaX = stepDir / delta.x;
    
        stepDelta = float2(stepDir, delta.y * invDeltaX) * cb.stepScale;
        inverseDelta = (rayEndInverseW - rayBeginInverseW) * invDeltaX * cb.stepScale;
     
        //초기위치 + stepDelta * jitter에서 마칭 시작.(자가 충돌을 피하기 위해)
        pixelCoord = rayBeginS + stepDelta * cb.startOffset;
        inverseW = rayBeginInverseW + inverseDelta * cb.startOffset;
         
        end = rayEndS.x * stepDir;
        
        rayViewZ = posV.z;
        objectFrontViewZ = rayViewZ + cb.thickness + 1;
        
        hit = SSR::CreateHitData();
    }
    void Execute()
    { 
        //3.3 Loop
        
        //Optional(Not implemented) 
        //1. HI-Z Structure (ref: GPU pro 5 ch 4 hi-z cone tracing, Frostbite SSR) for reduce step overhead
        [loop]
        for (uint stepCount = 0;
        ((pixelCoord.x * stepDir) <= end) && (stepCount < cb.maxStepCount) && !hit.isValid && objectFrontViewZ != 0;
            pixelCoord += stepDelta, inverseW += inverseDelta, ++stepCount)
        {
            hit.uv = (permute ? pixelCoord.yx : pixelCoord) * cb.halfInvRtSize;
        
            const float viewZ = viewZMap.SampleLevel(samPointClamp, hit.uv, 0).r;
            objectFrontViewZ = viewZ + cb.objectViewZBias;
        
            rayViewZ = 1.0f / inverseW;
            hit.viewZ = viewZ;
            hit.isValid = IsValidUv(hit.uv) && viewZ != cb.ta.camNearFar.y && ((objectFrontViewZ <= rayViewZ) && (rayViewZ <= (objectFrontViewZ + cb.thickness)));
        }
    }
    void BinarySearch()
    { 
        //3.3 Binary search
        //for fit intersection point      
        pixelCoord -= (stepDelta + (stepDelta * 0.5f));
        inverseW -= (inverseDelta + (inverseDelta * 0.5f));
        
        stepDelta *= 0.5f;
        inverseDelta *= 0.5f;
        
        [unroll]
        for (uint i = 0; i < BINARY_SEARCH_LOOP_COUNT; ++i)
        {
            const float2 uv = (permute ? pixelCoord.yx : pixelCoord) * cb.halfInvRtSize;
            const float viewZ = viewZMap.SampleLevel(samPointClamp, uv, 0);
            
            objectFrontViewZ = viewZ + cb.objectViewZBias;        
            rayViewZ = 1.0f / inverseW;
            
            const bool isValid = IsValidUv(uv) && viewZ != cb.ta.camNearFar.y && ((objectFrontViewZ <= rayViewZ) && (rayViewZ <= (objectFrontViewZ + cb.thickness)));
             
            stepDelta *= 0.5f;
            inverseDelta *= 0.5f;
            
            if (isValid)
            {
                pixelCoord -= stepDelta;
                inverseW -= inverseDelta;
                hit.uv = uv;
                hit.viewZ = viewZ;
            }
            else
            {
                pixelCoord += stepDelta;
                inverseW += inverseDelta;
            }
        }
    }
};

[numthreads(DIMX, DIMY, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.halfRtSize.x || dispatchThreadID.y >= cb.halfRtSize.y)
        return;
    
    //Preprocess
    //1. Unpack texture data   
    SSR::PixelData pixelData = SSR::CreatePixelData(dispatchThreadID);
    if (!pixelData.isValid)
    {
        pixelData.InsertFailColor();
        return;
    }
    
    //2. Compute reflect vector by view vec, normalV  
    SSR::ReflectionData viewReflection = SSR::CreateViewReflection(pixelData);
      
    [unroll]
    for (uint i = 0; i < RAY_COUNT; ++i)
    {       
        const float3 reflectionV = viewReflection.Compute(pixelData);
        
        //3. Raymarching           
        RaymarchingActor actor;
        actor.Initialize(pixelData.posV, reflectionV);
        actor.Execute();
 
        if (actor.hit.isValid)
        { 
            //Hi-Z는 최초 충돌 후 fit한 지점을 찾기위해 레벨을 조정하는 과정을 거치므로
            //Hi-Z가 적용되지 않은 구현에서 binary search 사용.
#ifdef USE_BINARY_SEARCH
            if (cb.stepScale > 1.0f)
                actor.BinarySearch();
#endif
            //4. Sample color
            pixelData.SampleHitcolor(actor.hit);
        }
        else
        { 
#ifndef SKIP_SKY_COLOR
            //Raycast와 다르게 Raymarching은 해당 지점에서 Sky의 가시성을 알 수 없음.
            //pixelData.SampleSkyColor(reflectionV);
#endif
        }
    }
    pixelData.InsertReflectionColor();
}