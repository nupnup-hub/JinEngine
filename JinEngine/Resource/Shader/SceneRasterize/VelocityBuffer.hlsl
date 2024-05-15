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
#include"../Common/DepthFunc.hlsl"
#include"../Common/Packing.hlsl"
#include"../Common/GBufferCommon.hlsl"
#include"../Common/CommonConstantsStructureDefine.hlsl"

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
 