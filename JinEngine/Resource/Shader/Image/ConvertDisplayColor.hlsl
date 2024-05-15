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
#include "../Common/ColorPacking.hlsl"

Texture2D<float3> src : register(t0);
Texture2D<float4> ori : register(t1);
RWTexture2D<float4> dst : register(u0);

cbuffer cbToDisplayColorPass : register(b0)
{
	float2 textureSize;
	float2 invTextureSize; 
};

#ifndef DIMX
#define DIMX 8
#endif

#ifndef DIMY
#define DIMY 8
#endif

[numthreads(DIMX, DIMY, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	if (textureSize.x <= dispatchThreadID.x || textureSize.y <= dispatchThreadID.y)
		return;
	
	float2 texCoord = (dispatchThreadID.xy + 0.5) * invTextureSize;
	float3 color = src[dispatchThreadID.xy];
#ifdef HDR_DISPLAY_MAPPING
	dst[dispatchThreadID.xy].xyz = ApplyREC709Curve(color);  
	//dst[dispatchThreadID.xy].xyz = ApplyREC2084Curve(REC709toREC2020(color));
#else
	dst[dispatchThreadID.xy].xyz = ApplySRGBCurve(color);
#endif
	dst[dispatchThreadID.xy].w = ori[dispatchThreadID.xy].w;
}
