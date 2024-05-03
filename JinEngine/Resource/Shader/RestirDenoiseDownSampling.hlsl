#pragma once  
#include"RestirDenoiseCommon.hlsl" 

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