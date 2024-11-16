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
#include"../../Common/MaterialData.hlsl" 
#include"../../Raytracing/RayDesc.hlsl" 

#ifndef T_MIN
#define T_MIN 1e-1
#endif   
   
#define DEFAULT_SHADER 0
#define VIEW_Z_THRESHOLD 8

#ifndef TEXTURE_2D_COUNT
#define TEXTURE_2D_COUNT 1
#endif 

ConstantBuffer<SSRData> cb : register(b0);
RaytracingAccelerationStructure sceneAs : register(t0);
//StructuredBuffer<InstanceData> instanceInfo : register(t1);
//StructuredBuffer<MaterialData> materialData : register(t2);
Texture2D srcMap : register(t1);
Texture2D<float> viewZMap : register(t2);
Texture2D normalMap : register(t3);
TextureCube skyMap : register(t4);
//Texture2D textureMaps[TEXTURE_2D_COUNT] : register(t7);
RWTexture2D<float4> ssrMap : register(u0);
SamplerState samPointClamp : register(s0);
SamplerState samLinearClamp : register(s1);
  
bool RayCast(RayDesc desc, out float3 hitPos, out uint instanceID)
{
    hitPos = float3(0, 0, 0); 
    
    RayQuery <RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;
    rayQuery.TraceRayInline(sceneAs, RAY_FLAG_NONE, 0xff, desc);
  
    rayQuery.Proceed(); 
    if (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        hitPos = rayQuery.WorldRayOrigin() + rayQuery.CommittedRayT() * rayQuery.WorldRayDirection(); 
        instanceID = rayQuery.CommittedInstanceID();
        return true;
    }
    else
        return false;
}
float ComputeAlpha(float2 uv)
{
    float fadeDistance = cb.fadeDistance;
    float fadeFactor = saturate(max(abs(uv.x - 0.5f) * 2.0f, abs(uv.y - 0.5f) * 2.0f));
 
    float alpha = fadeFactor > (1.0f - fadeDistance) ? (1.0f - fadeFactor) * cb.fadeOneRate : 1.0f;
    return alpha * alpha * alpha; 
}

[numthreads(DIMX, DIMY, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    const uint2 pixelCoord = dispatchThreadID.xy;
    if (pixelCoord.x >= cb.rtSize.x || pixelCoord.y >= cb.rtSize.y)
        return;
      
    float2 centerUv = (pixelCoord + float2(0.5f, 0.5f)) * cb.invRtSize;
    float viewZ = viewZMap.SampleLevel(samPointClamp, centerUv, 0).x;
    if (viewZ == cb.nearFarZ.y)
    {
        ssrMap[dispatchThreadID.xy] = float4(0, 0, 0, 0);
        return;
    } 
    
    const float3 posV = GetViewPos(centerUv, viewZ, cb.uvToViewA, cb.uvToViewB);
    const float3 posW = mul(float4(posV, 1.0f), cb.camInvView).xyz;
    
    const float3 normalW = UnpackNormal(normalMap.SampleLevel(samLinearClamp, centerUv, 0));
   // const float3 normalV = normalize(mul(normalW, (float3x3)cb.camView));
 
    //float3 reflectV = normalize(reflect(normalize(posV), normalV));
   // float3 reflectW = normalize(mul(float4(reflectV, 1.0f), cb.camInvView).xyz);
     
    float3 dir = normalize(reflect(normalize(posW - cb.camPosW), normalW));
    float3 startPoint = posW;
    float3 endPoint = startPoint + dir * cb.rayDistance;
     
    RayDesc rayDesc = CreateRayDesc(startPoint, endPoint, normalW, T_MIN); 
    
    float3 hitPos; 
    uint instanceID;
    bool isHit = RayCast(rayDesc, hitPos, instanceID);
      
    float4 color = float4(0, 0, 0, 0);
    color.xyz = srcMap.SampleLevel(samLinearClamp, centerUv, 0).xyz;
    
    if (isHit)
    {
        const float4 hitPosV = mul(float4(hitPos, 1.0f), cb.camView);
        const float4 hitPosC = mul(hitPosV, cb.camProj);
        const float3 hitNDC = hitPosC.xyz / hitPosC.w;
        const float2 hitUv = hitNDC.xy * float2(0.5f, -0.5f) + 0.5f;
        const float hitViewZ = viewZMap.SampleLevel(samPointClamp, hitUv, 0).x;
        if (IsValidUv(hitUv) && abs(hitViewZ - hitPosV.z) < VIEW_Z_THRESHOLD) 
        {
            color = float4(srcMap.SampleLevel(samLinearClamp, hitUv, 0).xyz, 1.0f);
        }
        else
        {
            //Hit point가 렌더링된 물체 뒤에 있거나 화면밖에 존재하는 경우. 
            //추가적인 음영계산이 필요하다.
            //IBL을 이용한다면 좋을거라 예상되며 IBL 구현 후 수정예정.   -2024.11.08-
            //MaterialData meshMatData = materialData[instanceInfo[instanceID].materialIndex];
            //float4 albedoColor = meshMatData.albedoColor;
            //if (meshMatData.albedoMapIndex != MISSING_TEXTURE_INDEX)
             //   albedoColor *= textureMaps[meshMatData.albedoMapIndex].SampleLevel(samLinearClamp, hitUv, 0);
             
           // color = float4(albedoColor.xyz, 1.0f);
        }
    }
    else
    {
        color = float4(skyMap.SampleLevel(samLinearClamp, rayDesc.Direction, 0).xyz, 0.5f);
        //color = float4(0, 0, 1, 1); 
    } 
    ssrMap[pixelCoord] = color;
}
  
 