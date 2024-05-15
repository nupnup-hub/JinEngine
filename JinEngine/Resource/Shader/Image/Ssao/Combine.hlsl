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

Texture2DArray aoMap : register(t0);
Texture2D<float> depthMap : register(t1);
SamplerState samPointClamp : register(s0);
 
#ifdef USE_BLUR 
	typedef float2 PixelOut;
#else	
	typedef float PixelOut;
#endif

PixelOut SsaoCombine(VertexOut pin) : SV_Target
{
#ifndef USE_BLUR 
    SubtractViewportOrigin(pin, cbPass.viewPortTopLeft, cbPass.camInvRtSize);
#endif
	
	int2 fullResPos = int2(pin.posH.xy);
	int2 offset = fullResPos & 3;
	int sliceId = offset.y * 4 + offset.x;
	int2 quarterResPos = fullResPos >> 2;
	float ao = aoMap.Load(int4(quarterResPos, sliceId, 0));
	
#if USE_BLUR 
    float viewDepth = depthMap.Sample(samPointClamp, pin.texC);
    return float2(ao, viewDepth);
#else    
    return pow(saturate(ao), cbPass.sharpness);
#endif 
}
