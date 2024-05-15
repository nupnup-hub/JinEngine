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
#include"../../../Common/Packing.hlsl"

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