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

ConstantBuffer<SSRData> cb : register(b0);
Texture2D srcMap : register(t0);
Texture2D<float> viewZMap : register(t1);
Texture2D normalMap : register(t2);
RWTexture2D<float4> ssrMap : register(u0);
SamplerState samPointClamp : register(s0);
SamplerState samLinearClamp : register(s1);
//SamplerState samPointClamp : register(s1);

struct SSRInput
{
    float3 dir;
    float3 posV; 
    float2 uv;
};

struct SSROutput
{
    float2 hitUv;
    float alpha;
    bool isHit;
};

//Process
//1. Unpack texture data
//2. Compute reflect vector by view vec, normalV 
//3. Raymarching
//  3.1 Prepare ray info
//  3.2 Compute DDA
//  3.3 Loop
//4. Store
 
#define DISTANCE_THRESHOLD 0.001f
#define ROW_DISTANCE_OFFSET 0.01f  
 
SSROutput ComputeReflectColor(in SSRInput input)
{
    //3. Raymarching
    
    //3.1 Prepare ray info
    //V = view space
    //C = clip space
    //S = screen space     
    float rayLength = (input.posV.z + input.dir.z * cb.rayDistance) < cb.nearFarZ.x ? (cb.nearFarZ.x - input.posV.z) / input.dir.z : cb.rayDistance;
    float3 rayEndV = input.posV + input.dir * rayLength;
    
    float4 rayBeginC = mul(float4(input.posV, 1.0f), cb.camProj);
    float4 rayEndC = mul(float4(rayEndV, 1.0f), cb.camProj);
     
    float rayBeginInverseW = 1.0f / rayBeginC.w;
    float rayEndInverseW = 1.0f / rayEndC.w;
     
    //Clip -> Ndc -> Screen
    float2 rayBeginS = (rayBeginC.xy * rayBeginInverseW * float2(0.5f, -0.5f) + 0.5f) * cb.rtSize;
    float2 rayEndS = (rayEndC.xy * rayEndInverseW * float2(0.5f, -0.5f) + 0.5f) * cb.rtSize;
    rayEndS += distance(rayEndS, rayBeginS) < DISTANCE_THRESHOLD ? ROW_DISTANCE_OFFSET : 0;
     
    //3.2 Compute DDA
    /*
        알고리즘.
        1. 기울기에 따라 기준 축 결정 ... delta < 1 => x, else y
        2. 기준 축을 따라 진행  ... x: x += 1, y += delta, y: y += 1, y += inverse(delta)
        3. 결과값이 실수일 경우 반올림. ... 기준 축이 아닌경우만 해당.
        
        항상 정수값을 반환하며 중간에 실수값을 취급하지 않는다.
    */
    bool permute = false;
    float2 delta = rayEndS - rayBeginS;
    if (abs(delta.x) < abs(delta.y))
    {
        permute = true;
        delta = delta.yx;
        rayBeginS = rayBeginS.yx;
        rayEndS = rayEndS.yx;
    }
    
    float stepDir = sign(delta.x);
    float invDeltaX = stepDir / delta.x;
    
    float2 stepDelta = float2(stepDir, delta.y * invDeltaX) * cb.stepScale;
    float inverseDelta = (rayEndInverseW - rayBeginInverseW) * invDeltaX * cb.stepScale; 
    
    //3.3 Loop 
    float2 pixelCoord = rayBeginS + stepDelta * cb.startOffset;
    float inverseW = rayBeginInverseW + inverseDelta * cb.startOffset; 
         
    float stepCount = 0.0f;
    float end = rayEndS.x * stepDir;
    
    //depth backface를 사용하면 두 번 렌더링 해야하므로
    //사용자 설정값(thickness)으로 대체
    //초기 위치 Hit를 피하기 위한 값 설정.
    float rayViewZ = input.posV.z;
    float thickness = cb.thickness;
    float objectFrontViewZ = rayViewZ + thickness + 1;
      
    //초기위치 + stepDelta * jitter에서 마칭 시작. 
    SSROutput output;
    output.alpha = 1;
    output.hitUv = input.uv;
    output.isHit = false;
     
    //loop condition
    //1. Distance 
    //2. Step count
    //3. Hit
    
    [loop]
    for (;
        ((pixelCoord.x * stepDir) <= end) && (stepCount < cb.maxStepCount) && IsValidUv(output.hitUv) && !output.isHit && objectFrontViewZ != 0;
        pixelCoord += stepDelta, inverseW += inverseDelta, stepCount += 1.0f)
    {
        output.hitUv = (permute ? pixelCoord.yx : pixelCoord) * cb.invRtSize;
        
        const float viewZ = viewZMap.SampleLevel(samPointClamp, output.hitUv, 0).r;
        objectFrontViewZ = viewZ + cb.objectViewZBias;
        
        rayViewZ = 1.0f / inverseW;
        output.isHit = viewZ != cb.nearFarZ.y && ((objectFrontViewZ <= rayViewZ) && (rayViewZ <= (objectFrontViewZ + thickness)));
    }
     
    //0.0f ~ 1.0f 
    //값에 범위에만 관심이 있으므로 y역전은 적용x  
    
    float fadeDistance = cb.fadeDistance;
    float fadeFactor = saturate(max(abs(output.hitUv.x - 0.5f) * 2.0f, abs(output.hitUv.y - 0.5f) * 2.0f));
 
    output.alpha = fadeFactor > (1.0f - fadeDistance) ? (1.0f - fadeFactor) * cb.fadeOneRate : 1.0f;
    output.alpha = output.alpha * output.alpha * output.alpha;
     
    return output;
};

[numthreads(DIMX, DIMY, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
    
    //Preprocess
    //1. Unpack texture data
    //2. Compute reflect vector by view vec, normalV 
    
    const float2 centerCoord = dispatchThreadID.xy + 0.5f;
    const float2 centerUv = centerCoord * cb.invRtSize;
    
    const float viewZ = viewZMap.SampleLevel(samPointClamp, centerUv, 0).r;
    if (viewZ == cb.nearFarZ.y)
    {
        ssrMap[dispatchThreadID.xy] = float4(0, 0, 0, 0);
        return;
    }
    
    const float3 posV = GetViewPos(centerUv, viewZ, cb.uvToViewA, cb.uvToViewB);
 
    const float3 normalW = UnpackNormal(normalMap.SampleLevel(samLinearClamp, centerUv, 0));
    const float3 normalV = normalize(mul(normalW, (float3x3) cb.camView));
	   
    const float3 reflectV = normalize(reflect(posV, normalV));
     
    SSRInput input;
    input.dir = reflectV;
    input.posV = posV; 
    input.uv = centerUv;
    
    //3. Raymarching
    SSROutput output = ComputeReflectColor(input);
    
    //4. Store
    const float3 color = srcMap.SampleLevel(samLinearClamp, output.hitUv, 0).xyz;
    ssrMap[dispatchThreadID.xy] = float4(color, output.alpha) * output.isHit;
}