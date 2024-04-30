#pragma once

Texture2D<float4> src : register(t0);
RWTexture2D<float4> desc : register(u0);

cbuffer Constants
{
    uint2 resolution;
    float2 invResolution;
}

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif

[numthreads(DIMX, DIMY, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= resolution.x || dispatchThreadID.y >= resolution.y)
        return;
    
    int3 mapLocation = int3(dispatchThreadID.xy, 0);
    float4 srcColor = src.Load(mapLocation);
#ifdef REVERSE_X
    desc[mapLocation.xy].xyz = float4(-srcColor.x, srcColor.yzw);
#elif REVERSE_Y
    desc[mapLocation.xy].xyz = float4(srcColor.x, -srcColor.y, srcColor.zw);
#elif REVERSE_Z
    desc[mapLocation.xy].xyz = float4(srcColor.xy, -srcColor.z, srcColor.w);
#else
    desc[mapLocation.xy] = srcColor;
#endif
    
}