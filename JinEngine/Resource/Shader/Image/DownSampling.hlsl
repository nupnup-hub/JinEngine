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


Texture2D srcTexture : register(t0);
RWTexture2D<float4> destTexture : register(u0);
 
cbuffer cbDownSamplePass : register(b0)
{
	uint2 srcSize;					//8
	uint2 destSize;					//16
	uint srcMipLevel;  
};
 
SamplerState samLinearClamp : register(s0);
    
[numthreads(DIMX, DIMY, 1)]
void DownSample(int3 dispatchThreadID : SV_DispatchThreadID)
{
	if (destSize.x <= dispatchThreadID.x || destSize.y <= dispatchThreadID.y)
		return;
	 
	float4 sampleColor = float4(0, 0, 0, 0);
	const float2 texelSize = 1.0f / destSize;
#ifdef EXIST 
	const float2 uv = (dispatchThreadID.xy + 0.5f) * texelSize;
	sampleColor = srcTexture.SampleLevel(samLinearClamp, uv, srcMipLevel);
#elif WIDTH_DOWN
	const float2 uv = (dispatchThreadID.xy + uint2(0.25f, 0.5f)) * texelSize;
	const float2 offset = float2(0.5f, 0.0f) * texelSize;
	sampleColor = (srcTexture.SampleLevel(samLinearClamp, uv, srcMipLevel) +
	srcTexture.SampleLevel(samLinearClamp, uv + offset, srcMipLevel)) * 0.5f;
#elif HEIGHT_DOWN
	const float2 uv = (dispatchThreadID.xy + uint2(0.5f, 0.25f)) * texelSize;
	const float2 offset = float2(0.0f, 0.5f) * texelSize;
	sampleColor = (srcTexture.SampleLevel(samLinearClamp, uv, srcMipLevel) +
	srcTexture.SampleLevel(samLinearClamp, uv + offset, srcMipLevel)) * 0.5f;
#elif WIDTH_HEIGHT_DOWN
	const float2 uv = (dispatchThreadID.xy + uint2(0.25f, 0.25f)) * texelSize;
	const float2 offset = texelSize * 0.5f;
	sampleColor = (srcTexture.SampleLevel(samLinearClamp, uv, srcMipLevel) +
	srcTexture.SampleLevel(samLinearClamp, uv + float2(offset.x, 0), srcMipLevel) +
	srcTexture.SampleLevel(samLinearClamp, uv + float2(0, offset.y), srcMipLevel) +
	srcTexture.SampleLevel(samLinearClamp, uv + offset, srcMipLevel)) * 0.25f;
#endif 
	destTexture[dispatchThreadID.xy] = sampleColor;
}
  
