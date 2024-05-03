#pragma once 
#include"RestirGiCommon.hlsl"
#include"Packing.hlsl"

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif

RWStructuredBuffer<RestirSamplePack> initialSample : register(u0);
RWStructuredBuffer<RestirReserviorPack> temporal00 : register(u1);
RWStructuredBuffer<RestirReserviorPack> temporal01 : register(u2);
RWStructuredBuffer<RestirReserviorPack> spatial00 : register(u3);
RWStructuredBuffer<RestirReserviorPack> spatial01 : register(u4);
 
[numthreads(DIMX, DIMY, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
      
    uint index = dispatchThreadID.x + dispatchThreadID.y * cb.rtSize.x;
    initialSample[index].Initialize();
    temporal00[index].Initialize();
    temporal01[index].Initialize();
    spatial00[index].Initialize();
    spatial01[index].Initialize();
}
  