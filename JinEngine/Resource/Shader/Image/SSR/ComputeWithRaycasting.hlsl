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
   
#ifndef T_MIN
#define T_MIN 1e-2
#endif   
   
#define DEFAULT_SHADER 0
#define VIEW_Z_THRESHOLD 8

ConstantBuffer<SSRData> cb : register(b0);
RaytracingAccelerationStructure sceneAs : register(t0);
Texture2D srcMap : register(t1);
Texture2D<float> viewZMap : register(t2);
Texture2D normalMap : register(t3);
TextureCube skyMap : register(t4);
RWTexture2D<float4> ssrMap : register(u0);
SamplerState samPointClamp : register(s0);
SamplerState samLinearClamp : register(s1);

float3 ComputeRayOrigin(float3 pos, float3 normal)
{
    const float origin = 1.f / 16.f;
    const float fScale = 3.f / 65536.f;
    const float iScale = 3 * 256.f;

    // Per-component integer offset to bit representation of fp32 position.
    int3 iOff = int3(normal * iScale);
    float3 iPos = asfloat(asint(pos) + select(pos < 0.0f, -iOff, iOff));

    // Select per-component between small fixed offset or above variable offset depending on distance to origin.
    float3 fOff = normal * fScale;
    return select(abs(pos) < origin, pos + fOff, iPos);
}
RayDesc CreateRayDesc(float3 pos, float3 endPoint, float3 normal, float tMin)
{
    float3 offset = endPoint - pos;
    float tMax = length(offset);
    float3 dir = offset / tMax;
    RayDesc rayDesc;
    rayDesc.Origin = ComputeRayOrigin(pos, normal);
    rayDesc.Direction = dir;
    rayDesc.TMin = tMin;
    rayDesc.TMax = 0.999f * tMax;
    return rayDesc;
}
bool RayCast(RayDesc desc, out float3 hitPos, out float3 debugColor)
{
    hitPos = float3(0, 0, 0);
    debugColor = float3(0, 0, 0);
    RayQuery < RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH > rayQuery;
    rayQuery.TraceRayInline(sceneAs, RAY_FLAG_NONE, 0xff, desc);
  
    rayQuery.Proceed();
    if (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        hitPos = rayQuery.WorldRayOrigin() + rayQuery.CommittedRayT() * rayQuery.WorldRayDirection();
        debugColor = float3(1, 0, 0);
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
    const float3 normalV = normalize(mul(normalW, (float3x3)cb.camView));
 
    float3 reflectV = normalize(reflect(normalize(posV), normalV));
    float3 reflectW = normalize(mul(float4(reflectV, 1.0f), cb.camInvView).xyz);
    
    float3 reflectS = reflect(normalize(posW - cb.camPosW), normalW);
    float3 dir = reflectS * cb.stepScale;
    float3 startPoint = posW;
    float3 endPoint = startPoint + dir * cb.rayDistance;
    
    RayDesc rayDesc = CreateRayDesc(startPoint, endPoint, normalW, T_MIN); 
    
    float3 hitPos;
    float3 debugColor;
    bool isHit = RayCast(rayDesc, hitPos, debugColor);
      
    float4 color = float4(0, 0, 0, 0);
    if (isHit)
    {
        const float4 hitPosV = mul(float4(hitPos, 1.0f), cb.camView);
        const float4 hitPosC = mul(hitPosV, cb.camProj);
        const float3 hitNDC = hitPosC.xyz / hitPosC.w;
        const float2 hitUv = hitNDC.xy * float2(0.5f, -0.5f) + 0.5f;
        if (IsValidUv(hitUv))
        { 
            const float hitViewZ = viewZMap.SampleLevel(samPointClamp, hitUv, 0).x;
            if (abs(hitViewZ - hitPosV.z) < VIEW_Z_THRESHOLD)
            { 
                color = float4(srcMap.SampleLevel(samLinearClamp, hitUv, 0).xyz, ComputeAlpha(hitUv));
            }
        } 
    }
    else
    {
        color = float4(skyMap.SampleLevel(samLinearClamp, rayDesc.Direction, 0).xyz, 0.25f);
    }
    ssrMap[pixelCoord] = color;
}
  
 