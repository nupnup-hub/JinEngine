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
#include"../Common.hlsl" 

#ifndef DIMX
#define DIMX 8
#endif
#ifndef DIMY
#define DIMY 8
#endif 
 
Texture2D src : register(t0); //base level
RWTexture2D<float4> mipLevel0 : register(u0); //1x1 same as base level 
RWTexture2D<float4> mipLevel1 : register(u1); //2x2 
RWTexture2D<float4> mipLevel2 : register(u2); //4x4
RWTexture2D<float4> mipLevel3 : register(u3); //8x8
SamplerState samLinearClmap : register(s0);
   
groupshared float4 groupTile[DIMX * DIMY]; // 8x8 input pixels

[numthreads(DIMX, DIMY, 1)]
void main(int groupIndex : SV_GroupIndex, int3 dispatchThreadID : SV_DispatchThreadID)
{
    //thread dimension is same as src half resolution 
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
     
    uint threadBit = dispatchThreadID.x | dispatchThreadID.y;
    float4 average = src[dispatchThreadID.xy];
   
    mipLevel0[dispatchThreadID.xy] = average;
    groupTile[groupIndex] = average;
    GroupMemoryBarrierWithGroupSync();
    
    //4x4 downsample
    if ((threadBit & 1) == 0)
    {
        //avarage neighbor 1x1 block
        average = 0.25f * (average + groupTile[groupIndex + 1] + groupTile[groupIndex + 8] + groupTile[groupIndex + 9]);
        mipLevel1[dispatchThreadID.xy >> 1] = average;
        groupTile[groupIndex] = average;
    }
    GroupMemoryBarrierWithGroupSync();
    
    //2x2 downsample
    if ((threadBit & 3) == 0)
    {
        //avarage neighbor 2x2 block
        average = 0.25f * (average + groupTile[groupIndex + 2] + groupTile[groupIndex + 16] + groupTile[groupIndex + 18]);
        mipLevel2[dispatchThreadID.xy >> 2] = average;
        groupTile[groupIndex] = average;
    }
    GroupMemoryBarrierWithGroupSync();
    
    //1x1 downsample   
    if ((threadBit & 7) == 0)
    {
        //avarage neighbor 4x4 block
        average = 0.25f * (average + groupTile[groupIndex + 4] + groupTile[groupIndex + 32] + groupTile[groupIndex + 36]);
        mipLevel3[dispatchThreadID.xy >> 3] = average;
    }
}