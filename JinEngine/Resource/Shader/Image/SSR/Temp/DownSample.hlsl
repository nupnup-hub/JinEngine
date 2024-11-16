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

#include "Common.hlsl"
 
Texture2D<float4> ssr : register(t0);
RWTexture2D<float4> result0 : register(u0);
RWTexture2D<float4> result1 : register(u1);
#ifdef DOWN_SAMPLE4
RWTexture2D<float4> result2 : register(u2);
RWTexture2D<float4> result3 : register(u3); 
#endif 
SamplerState samLinearClmap : register(s0);

cbuffer cb0 : register(b0)
{
    float2 inverseDim;
};

#ifndef DIMX
#define DIMX 8
#endif
#ifndef DIMY
#define DIMY 8
#endif
groupshared float4 gTile[DIMX * DIMY]; // 8x8 input pixels
 
[numthreads(DIMX, DIMY, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{ 
    uint parity = dispatchThreadID.x | dispatchThreadID.y;

    // Store the first downsampled quad per thread
    float2 centerUV = (float2(dispatchThreadID.xy) * 2.0f + 1.0f) * inverseDim;
    float4 avgPixel = ssr.SampleLevel(samLinearClmap, centerUV, 0.0f);
#ifdef DOWN_SAMPLE4
	gTile[groupIndex] = avgPixel;
	result0[dispatchThreadID.xy] = avgPixel;

	GroupMemoryBarrierWithGroupSync();

    // Downsample and store the 4x4 block
	if ((parity & 1) == 0)
	{
		avgPixel = 0.25f * (avgPixel + gTile[groupIndex + 1] + gTile[groupIndex + 8] + gTile[groupIndex + 9]);
		gTile[groupIndex] = avgPixel;
		result1[dispatchThreadID.xy >> 1] = avgPixel;
	}

	GroupMemoryBarrierWithGroupSync();

    // Downsample and store the 2x2 block
	if ((parity & 3) == 0)
	{
		avgPixel = 0.25f * (avgPixel + gTile[groupIndex + 2] + gTile[groupIndex + 16] + gTile[groupIndex + 18]);
		gTile[groupIndex] = avgPixel;
		result2[dispatchThreadID.xy >> 2] = avgPixel;
	}

	GroupMemoryBarrierWithGroupSync();

    // Downsample and store the 1x1 block
	if ((parity & 7) == 0)
	{
		avgPixel = 0.25f * (avgPixel + gTile[groupIndex + 4] + gTile[groupIndex + 32] + gTile[groupIndex + 36]);
		result3[dispatchThreadID.xy >> 3] = avgPixel;
	}  
	
#else
    gTile[groupIndex] = avgPixel;
    result0[dispatchThreadID.xy] = avgPixel;
    
    GroupMemoryBarrierWithGroupSync();

     // Downsample and store the 4x4 block
    if ((parity & 1) == 0)
    {
        avgPixel = 0.25f * (avgPixel + gTile[groupIndex + 1] + gTile[groupIndex + 8] + gTile[groupIndex + 9]);
        result1[dispatchThreadID.xy >> 1] = avgPixel;
    }
#endif
}
