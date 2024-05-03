#pragma once
#include"RestirGiCommon.hlsl"
#include"Packing.hlsl"

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif
  
StructuredBuffer<RestirReserviorPack> src : register(t0);
//StructuredBuffer<RestirSamplePack> intial : register(t1);         //for debugging
RWTexture2D<float4> dst : register(u0);

[numthreads(DIMX, DIMY, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
     
    RestirReserviorPack data = src[dispatchThreadID.x + dispatchThreadID.y * cb.rtSize.x]; 
    dst[dispatchThreadID.xy] = float4(data.sample.UnpackRadiance() * data.W, 1.0f); 
    
    //for debugging
    //RestirSamplePack init = intial[dispatchThreadID.x + dispatchThreadID.y * cb.rtSize.x];
   // dst[dispatchThreadID.xy] = float4(init.UnpackRadiance(), 1.0f);
}  