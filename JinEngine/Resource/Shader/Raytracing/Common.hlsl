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
#include"../Common/CommonConstantsStructureDefine.hlsl"

#ifdef USE_DEBUG

#ifndef CB_RAYTRACING_DEBUG_REGISTER
#define CB_RAYTRACING_DEBUG_REGISTER u0
#endif

#ifndef DEBUG_SAMPLE_RAY_COUNT
#define DEBUG_SAMPLE_RAY_COUNT 4
#endif 

struct RaytracingDebug
{
	float4 rayOriginAndMinT[DEBUG_SAMPLE_RAY_COUNT];
	float4 rayDirAndMaxT[DEBUG_SAMPLE_RAY_COUNT];
	float4 intersectPosition[DEBUG_SAMPLE_RAY_COUNT];		//x, y, z = position , w = result 0, 1
	int2 index;
};

RWStructuredBuffer<RaytracingDebug> debugInfo : register(CB_RAYTRACING_DEBUG_REGISTER);
#endif

#ifndef OBJECT_REF_INSTANCE_ID_OFFSET
#define OBJECT_REF_INSTANCE_ID_OFFSET 0
#endif
//#ifndef POINT_LIGHT_INSTANCE_ID_OFFSET
//#define POINT_LIGHT_INSTANCE_ID_OFFSET 0
//#endif
//#ifndef SPOT_LIGHT_INSTANCE_ID_OFFSET
//#define SPOT_LIGHT_INSTANCE_ID_OFFSET 0
//#endif
#ifndef RECT_LIGHT_INSTANCE_ID_OFFSET
#define RECT_LIGHT_INSTANCE_ID_OFFSET 0
#endif
#ifndef LIGHT_INSTANCE_ID_END
#define LIGHT_INSTANCE_ID_END 0
#endif
 
#ifndef OBJECT_MASK
#define OBJECT_MASK 0
#endif
//#ifndef POINT_LIGHT_MASK
//#define POINT_LIGHT_MASK 0
//#endif
//#ifndef SPOT_LIGHT_MASK
//#endif
#ifndef RECT_LIGHT_MASK
#define RECT_LIGHT_MASK 0
#endif
  
#define STATIC_VERTEX 0
#define SKNNED_VERTEX 1

#define INDEX_UINT_16 0
#define INDEX_UINT_32 1
 
#define HIT_OPAQUE_OBJECT 0
#define HIT_LIGHT_OBJECT 1

struct StaticVertex
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 texC : TEXCOORD;
    float3 tangent : TANGENT;
};
struct SkinnedVertex
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 texC : TEXCOORD;
    float3 tangent : TANGENT;
    float3 boneWeights : WEIGHTS;
    uint4 boneIndices : BONEINDICES;
}; 


RaytracingAccelerationStructure sceneAs : register(t0);
StructuredBuffer<InstanceData> instanceInfo : register(t1);

//return isVisible
bool VisibilityRayQuery(RayDesc desc)
{
    //template flag | dynamic flag
    RayQuery < RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH > rayQuery;
    rayQuery.TraceRayInline(sceneAs, RAY_FLAG_NONE, 0xff, desc);

    // Proceed() below is where behind-the-scenes traversal happens,
    // including the heaviest of any driver inlined code.
    // In this simplest of scenarios, Proceed() only needs
    // to be called once rather than a loop:
    // Based on the template specialization above,
    // traversal completion is guaranteed.
    while (rayQuery.Proceed())
    {
        if (rayQuery.CandidateType() == CANDIDATE_NON_OPAQUE_TRIANGLE)
            rayQuery.CommitNonOpaqueTriangleHit();
        
        if (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
            return false;
    }
    return true;
}

float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}
float3 HitWorldPosition(float3 posW, float3 dir, float t)
{
    return posW + t * dir;
}

float BarycentricLerpF1(in float v0, in float v1, in float v2, in float3 barycentrics)
{
    return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}
float2 BarycentricLerpF2(in float2 v0, in float2 v1, in float2 v2, in float3 barycentrics)
{
    return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}
float3 BarycentricLerpF3(in float3 v0, in float3 v1, in float3 v2, in float3 barycentrics)
{
    return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}
float4 BarycentricLerpF4(in float4 v0, in float4 v1, in float4 v2, in float3 barycentrics)
{
    return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}

StaticVertex BarycentricLerpV(in StaticVertex v0, in StaticVertex v1, in StaticVertex v2, in float3 barycentrics)
{
    StaticVertex v;
    v.pos = BarycentricLerpF3(v0.pos, v1.pos, v2.pos, barycentrics);
    v.normal = normalize(BarycentricLerpF3(v0.normal, v1.normal, v2.normal, barycentrics));
    v.texC = BarycentricLerpF2(v0.texC, v1.texC, v2.texC, barycentrics);
    v.tangent = normalize(BarycentricLerpF3(v0.tangent, v1.tangent, v2.tangent, barycentrics));
    return v;
}

StaticVertex GetStaticMeshVertex(const uint instanceID, uint primitiveIndex, in BuiltInTriangleIntersectionAttributes attr)
{
    StructuredBuffer<StaticVertex> vertexBuffer = ResourceDescriptorHeap[instanceInfo[instanceID].verticesIndex];
    Buffer<uint> indicesBuffr = ResourceDescriptorHeap[instanceInfo[instanceID].indicesIndex];
             
    uint iOffset = instanceInfo[instanceID].indicesOffset;
    uint vOffset = instanceInfo[instanceID].verticesOffset;
    
    uint i0 = indicesBuffr[primitiveIndex * 3 + iOffset];
    uint i1 = indicesBuffr[primitiveIndex * 3 + iOffset + 1];
    uint i2 = indicesBuffr[primitiveIndex * 3 + iOffset + 2];
        
    StaticVertex v0 = vertexBuffer[i0 + vOffset];
    StaticVertex v1 = vertexBuffer[i1 + vOffset];
    StaticVertex v2 = vertexBuffer[i2 + vOffset];
    
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    return BarycentricLerpV(v0, v1, v2, barycentrics);
} 
StaticVertex GetStaticMeshVertex(const uint verticesIndex, const uint indicesIndex, uint primitiveIndex, in BuiltInTriangleIntersectionAttributes attr)
{
    StructuredBuffer<StaticVertex> vertexBuffer = ResourceDescriptorHeap[verticesIndex];
    Buffer<uint> indicesBuffr = ResourceDescriptorHeap[indicesIndex];
 
    uint i0 = indicesBuffr[primitiveIndex * 3];
    uint i1 = indicesBuffr[primitiveIndex * 3 + 1];
    uint i2 = indicesBuffr[primitiveIndex * 3 + 2];
        
    StaticVertex v0 = vertexBuffer[i0];
    StaticVertex v1 = vertexBuffer[i1];
    StaticVertex v2 = vertexBuffer[i2];
    
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    return BarycentricLerpV(v0, v1, v2, barycentrics);
}
float3 GetSurfaceNormal(const uint instanceID, uint primitiveIndex, in BuiltInTriangleIntersectionAttributes attr)
{
    StructuredBuffer<StaticVertex> vertexBuffer = ResourceDescriptorHeap[instanceInfo[instanceID].verticesIndex];
    Buffer<uint> indicesBuffr = ResourceDescriptorHeap[instanceInfo[instanceID].indicesIndex];
             
    uint iOffset = instanceInfo[instanceID].indicesOffset;
    uint vOffset = instanceInfo[instanceID].verticesOffset;
    
    uint i0 = indicesBuffr[primitiveIndex * 3 + iOffset];
    uint i1 = indicesBuffr[primitiveIndex * 3 + iOffset + 1];
    uint i2 = indicesBuffr[primitiveIndex * 3 + iOffset + 2];
        
    StaticVertex v0 = vertexBuffer[i0 + vOffset];
    StaticVertex v1 = vertexBuffer[i1 + vOffset];
    StaticVertex v2 = vertexBuffer[i2 + vOffset];
    
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    return normalize(BarycentricLerpF3(v0.normal, v1.normal, v2.normal, barycentrics));
}
float3 GetSurfaceNormal(const uint verticesIndex, const uint indicesIndex, uint primitiveIndex, in BuiltInTriangleIntersectionAttributes attr)
{
    StructuredBuffer<StaticVertex> vertexBuffer = ResourceDescriptorHeap[verticesIndex];
    Buffer<uint> indicesBuffr = ResourceDescriptorHeap[indicesIndex];
 
    uint i0 = indicesBuffr[primitiveIndex * 3];
    uint i1 = indicesBuffr[primitiveIndex * 3 + 1];
    uint i2 = indicesBuffr[primitiveIndex * 3 + 2];
        
    StaticVertex v0 = vertexBuffer[i0];
    StaticVertex v1 = vertexBuffer[i1];
    StaticVertex v2 = vertexBuffer[i2];
    
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    return normalize(BarycentricLerpF3(v0.normal, v1.normal, v2.normal, barycentrics));
}
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
bool EvaluateSegmentVisibility(float3 pos, float3 endPoint, float tMin)
{  
    return VisibilityRayQuery(CreateRayDesc(pos, endPoint, tMin));
}

