#pragma once 
#include"RestirDenoiseCommon.hlsl"
#include"Packing.hlsl"

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif

RWStructuredBuffer<float4> colorHistory : register(u0);
RWStructuredBuffer<float4> fastColorHistory : register(u1);
RWStructuredBuffer<float> historyLength : register(u2);

RWStructuredBuffer<float4> preColorHistory : register(u3);
RWStructuredBuffer<float4> preFastColorHistory : register(u4);
RWStructuredBuffer<float> preHistoryLength : register(u5);
 
[numthreads(DIMX, DIMY, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
      
    uint index = dispatchThreadID.x + dispatchThreadID.y * cb.rtSize.x;
    colorHistory[index] = float4(0, 0, 0, 0);
    fastColorHistory[index] = float4(0, 0, 0, 0);
    historyLength[index] = 0;
    
    preColorHistory[index] = float4(0, 0, 0, 0);
    preFastColorHistory[index] = float4(0, 0, 0, 0);
    preHistoryLength[index] = 0;
}
  