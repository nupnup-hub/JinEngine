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
#include"../../Common/MaterialData.hlsl" 
#include"../../Raytracing/RayDesc.hlsl" 

//Process
//1. Unpack texture data
//2. Create Raycast desc
//3. Raycast with dxr query
//4. Determine color

#define SKIP_SKY_COLOR
#ifndef SSR_RAY_CAST_T_MIN_OFFSET
#define SSR_RAY_CAST_T_MIN_OFFSET 0.1f
#endif   
 
#define DEFAULT_SHADER 0
#define VIEW_Z_THRESHOLD 4

RaytracingAccelerationStructure sceneAs : register(t5);
  
bool RayCast(RayDesc desc, out float3 hitPos, out float distance, out uint instanceID)
{
    hitPos = float3(0, 0, 0);
    distance = 0;
    instanceID = 0;
    
    RayQuery < RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH > rayQuery;
    rayQuery.TraceRayInline(sceneAs, RAY_FLAG_NONE, 0xff, desc);
  
    rayQuery.Proceed();
    if (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        instanceID = rayQuery.CommittedInstanceID();
        distance = rayQuery.CommittedRayT();
        
        hitPos = rayQuery.WorldRayOrigin() + distance * rayQuery.WorldRayDirection();
        return true;
    }
    else
        return false;
}

[numthreads(DIMX, DIMY, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.ta.rtSize.x || dispatchThreadID.y >= cb.ta.rtSize.y)
        return;
      
    //1. Unpack texture data
    SSR::PixelData pixelData = SSR::CreatePixelData(dispatchThreadID);
    if (!pixelData.isValid)
    {
        pixelData.InsertFailColor();
        return;
    }
    
    SSR::ReflectionData worldReflection = SSR::CreateWorldReflectionData(pixelData);
    
    [unroll]
    for (uint i = 0; i < RAY_COUNT; ++i)
    {
        if (pixelData.roughness == 0 && i == 1)
            break;
         
        //2. Create Raycast desc
        float3 dir = worldReflection.Compute(pixelData);
        float3 startPoint = pixelData.posW;
        float3 endPoint = startPoint + dir * cb.rayDistance;
        
        float dotNR = dot(dir, pixelData.normalW);
        float tMin = cb.rayTMin + SSR_RAY_CAST_T_MIN_OFFSET * abs(1.0f - dotNR);
        RayDesc rayDesc = CreateRayDesc(startPoint, endPoint, pixelData.normalW, tMin);
  
        float3 hitPos;
        float distance;
        uint instanceID;
        
        //3. Raycast with dxr query
        SSR::HitData hitData = SSR::CreateHitData();
        hitData.isValid = RayCast(rayDesc, hitPos, distance, instanceID);
        
        //4. Determine color
        if (hitData.isValid)
        {
            const float4 hitPosV = mul(float4(hitPos, 1.0f), cb.camView);
            const float4 hitPosC = mul(hitPosV, cb.camProj);
            const float3 hitNDC = hitPosC.xyz / hitPosC.w;
            const float2 hitUv = hitNDC.xy * float2(0.5f, -0.5f) + 0.5f;
            const float hitViewZ = viewZMap.SampleLevel(samPointClamp, hitUv, 0).x;
        
            if (IsValidUv(hitUv) && abs(hitViewZ - hitPosV.z) < VIEW_Z_THRESHOLD)
            {
                hitData.uv = hitUv;
                hitData.viewZ = hitViewZ;
                pixelData.SampleHitcolor(hitData);
            }
            else
            {
                //Hit point가 렌더링된 물체 뒤에 있거나 화면밖에 존재하는 경우. 
                //추가적인 음영계산이 필요하다.
                //IBL을 이용한다면 좋을거라 예상되며 IBL 구현 후 수정예정.   -2024.11.08- 
                //pixelData.SampleSkyColor(rayDesc.Direction);
            }
        }
        else
        {
#ifndef SKIP_SKY_COLOR
            pixelData.SampleSkyColor(rayDesc.Direction);
#endif
        }
    };
    pixelData.InsertReflectionColor(); 
}
  
 