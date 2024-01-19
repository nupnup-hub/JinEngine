#include"FilterCommon.hlsl"
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
   
cbuffer cbBlurPass : register(b0)
{
	float2 size;						//8
	float2 invSize;						//16
	float4x2 kernel[KERNEL_MAX_SIZE];	//240	 
	int mipLevel;						//244
	float blurPad00;					//248
	float blurPad01;					//252
	float blurPad02;					//256
}; 

SamplerState samLinearClamp : register(s0);
#ifdef USE_KERNEL_DATA
[numthreads(DIMX, DIMY, 1)]
void Blur(int3 dispatchThreadID : SV_DispatchThreadID)
{
	if (size.x <= dispatchThreadID.x || size.y <= dispatchThreadID.y)
		return;
  
	//texel mid
	int2 index = int2(dispatchThreadID.x, dispatchThreadID.y);
	float2 uv = (index + float2(0.5f, 0.5f)) * invSize;
	float4 sum = 0.0f;

	[unroll]
	for (int i = KERNEL_START; i <= KERNEL_END; ++i)
	{ 
#ifdef USE_3x3_KERNEL
	float localKernel[KERNEL_SIZE] = {kernel[i][0].x, kernel[i][0].y, kernel[i][1].x};
#elif USE_5x5_KERNEL
	float localKernel[KERNEL_SIZE]= {kernel[i][0].x, kernel[i][0].y, kernel[i][1].x, kernel[i][1].y, kernel[i][2].x};
#elif USE_7x7_KERNEL
	float localKernel[KERNEL_SIZE]= {kernel[i][0].x, kernel[i][0].y, kernel[i][1].x, kernel[i][1].y, kernel[i][2].x, kernel[i][2].y, kernel[i][3].x};
#elif USE_2x2_KERNEL
	float localKernel[KERNEL_SIZE] = {kernel[i][0].x, kernel[i][0].y};
#else
	float localKernel[KERNEL_SIZE] = { 0, 0, 0 };
#endif 
		[unroll]
		for (int j = KERNEL_START; j <= KERNEL_END; ++j)
		{
			float2 offset = int2(i, j) * invSize;
			sum += srcTexture.SampleLevel(samLinearClamp, uv + offset, mipLevel) * localKernel[j + KERNEL_OFFSET];
		}
	}
	destTexture[index] = sum;
}

#endif
 