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


#include"../Common/FilterCommon.hlsl"
#include"../Common/Math.hlsl"
#ifndef DIMX
#define DIMX 1
#endif
#ifndef DIMY
#define DIMY 1
#endif
#ifndef DIMZ
#define DIMZ 1
#endif

Texture2D srcTexture : register(t0);
RWTexture2D<float4> destTexture : register(u0);
 
struct BlurPassDataIncludeKernel
{
    float2 size;
    float2 invSize;
    int mipLevel;
    float sharpness;
    int blurPad00;
    int blurPad01;
    float4 kernel[KERNEL_MAX_SIZE];
};
struct BlurPassData
{
    float2 size;
    float2 invSize;
    int mipLevel;
    float sharpness;
    int blurPad00;
    int blurPad01; 
}; 
#ifdef USE_KERNEL_DATA
ConstantBuffer<BlurPassDataIncludeKernel> cbPass : register(b0);
#else
ConstantBuffer<BlurPassData> cbPass : register(b0);
#endif
 
float CalWeight(const int x, const float sharpness)
{
#ifdef BOX_BLUR
	return 1.0f / KERNEL_SIZE;
#elif GAUS_BLUR
	return exp(-(x * x) / (2.0f * sharpness * sharpness)) / sqrt((2.0f * PI * sharpness * sharpness));
#elif KAISER_BLUR
	return 0;
	//return exp(-(x * x) / (2.0f * sharpness * sharpness)) / sqrt((2.0f * PI * sharpness * sharpness));
#endif
	return 0;
} 

SamplerState samLinearClamp : register(s0);
[numthreads(DIMX, DIMY, 1)]
void Blur(int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (cbPass.size.x <= dispatchThreadID.x || cbPass.size.y <= dispatchThreadID.y)
		return;

#ifndef USE_KERNEL_DATA
	float kernel[KERNEL_MAX_SIZE];
	float kernelSum = 0;
	int step = 0;
	[unroll]
	for (step = KERNEL_START; step <= KERNEL_END; ++step)
	{
        kernel[step + KERNEL_OFFSET] = CalWeight(step, cbPass.sharpness);
		kernelSum += kernel[step + KERNEL_OFFSET];
	}
	for (step = KERNEL_START; step <= KERNEL_END; ++step)
		kernel[step + KERNEL_OFFSET] /= kernelSum;
#endif
	//texel mid
	int2 index = int2(dispatchThreadID.x, dispatchThreadID.y);
    float2 uv = (index + float2(0.5f, 0.5f)) * cbPass.invSize;
	float4 sum = 0.0f;

	[unroll]
	for (int i = KERNEL_START; i <= KERNEL_END; ++i)
	{  
		float2 offset = float2(0, 0);
	#ifdef VERTICAL
		offset = float2(i * cbPass.invSize.x, 0);
	#elif HORIZONTAL
		offset = float2(0, i * cbPass.invSize.y);
	#endif
	#ifdef USE_KERNEL_DATA
        sum += srcTexture.SampleLevel(samLinearClamp, uv + offset, cbPass.mipLevel) * cbPass.kernel[i + KERNEL_OFFSET].x;
	#else
		sum += srcTexture.SampleLevel(samLinearClamp, uv + offset, cbPass.mipLevel) * kernel[i + KERNEL_OFFSET].x;
	#endif
    }
	destTexture[index] = sum;
}
 