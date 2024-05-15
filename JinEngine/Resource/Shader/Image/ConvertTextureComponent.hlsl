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