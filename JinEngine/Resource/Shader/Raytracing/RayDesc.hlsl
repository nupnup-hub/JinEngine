#pragma once
 
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
//raycast시 자기교차를 피하는 방법에 대한 적용이 필요하다
//ref raytracing gem1 ch 6
RayDesc CreateRayDesc(float3 pos, float3 endPoint, float tMin)
{
    float3 offset = endPoint - pos;
    float tMax = length(offset);
    float3 dir = offset / tMax;
    RayDesc rayDesc;
    rayDesc.Origin = pos;
    rayDesc.Direction = dir;
    rayDesc.TMin = tMin;
    rayDesc.TMax = 0.999f * tMax;
    return rayDesc;
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
RayDesc CreateRayDesc(float3 pos, float3 dir, float3 normal, float distance, float tMin)
{
    RayDesc rayDesc;
    rayDesc.Origin = ComputeRayOrigin(pos, normal);
    rayDesc.Direction = dir;
    rayDesc.TMin = tMin;
    rayDesc.TMax = 0.999f * distance;
    return rayDesc;
} 
