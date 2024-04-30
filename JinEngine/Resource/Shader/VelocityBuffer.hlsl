#pragma once
#include"DepthFunc.hlsl"
#include"Packing.hlsl"
#include"GBufferCommon.hlsl"
#include"CommonConstantsStructureDefine.hlsl"

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif
#ifndef DIMZ
#define DIMZ 1
#endif 

Texture2D depthMap : register(t0);
RWTexture2D<uint> velocityMap : register(u0);
ConstantBuffer<CameraData> cbCam : register(b0);
/*
dim default value 
thread 16, 16, 1
*/ 
[numthreads(DIMX, DIMY, DIMZ)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (cbCam.renderTargetSize.x <= dispatchThreadID.x || cbCam.renderTargetSize.y <= dispatchThreadID.y)
        return;
 
    int3 mapLocation = int3(dispatchThreadID.xy, 0); 
    float2 uv = (mapLocation.xy + float2(0.5f, 0.5f)) * cbCam.invRenderTargetSize;
    
    float depth = depthMap.Load(mapLocation).x;
    float viewZ = NdcToViewPZ(depth, cbCam.nearZ, cbCam.FarZ);
    float3 posV = UVToViewSpace(uv, viewZ, cbCam.uvToViewA, cbCam.uvToViewB);
    float3 posW = mul(float4(posV, 1.0f), cbCam.invView).xyz;

    float4 prePosH = mul(float4(posW, 1.0f), cbCam.preViewProj);
    float2 preUv = (prePosH.xy / prePosH.w) * float2(0.5f, -0.5f) + 0.5f;
    
    float3 velocity = float3(preUv - uv, NdcToViewPZ(((prePosH.z / prePosH.w) - depth), cbCam.nearZ, cbCam.FarZ));
    velocityMap[mapLocation.xy] = PackVelocity(velocity);
}
 