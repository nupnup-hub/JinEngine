Texture2D srcTexture : register(t0); 
RWTexture2D<float4> destTexture : register(u0);
   
cbuffer BlurInfo : register(b0)
{
	float2 size;
	float2 invSize;
	int mipLevel;
	int blurPad00;
	int blurPad01;
	int blurPad02;
};
 
SamplerState samLinearClamp : register(s0);

#ifdef USE_3x3_KERNEL
#define RADIUS 1
#elif USE_5x5_KERNEL
#define RADIUS 2
#elif USE_7x7_KERNEL
#define RADIUS 3
#elif USE_2x2_KERNEL
#define RADIUS 1
#define KERNEL_COUNT_IS_EVEN_NUMBER 1
#else 
#endif

#ifdef RADIUS
#ifdef KERNEL_COUNT_IS_EVEN_NUMBER
#define KENEL_SIZE RADIUS * 2
#define KENEL_START -RADIUS + 1
#define KENEL_END RADIUS
#define KENEL_OFFSET RADIUS - 1
#else
#define KENEL_SIZE RADIUS * 2 + 1
#define KENEL_START -RADIUS
#define KENEL_END RADIUS
#define KENEL_OFFSET RADIUS
#endif
cbuffer KernelData : register(b1)
{
	float kernel[KENEL_SIZE][KENEL_SIZE];
	float kernelPad00;
	float kernelPad01;
	float kernelPad02;
};
#endif

#if defined (DIMX) && defined (DIMY)
#ifdef RADIUS
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
	for (int i = KENEL_START; i <= KENEL_END; ++i)
	{
		[unroll]
		for (int j = KENEL_START; j <= KENEL_END; ++j)
		{
			float2 offset = int2(i, j) * invSize;
			sum += srcTexture.SampleLevel(samLinearClamp, uv + offset, mipLevel) * kernel[i + KENEL_OFFSET][j + KENEL_OFFSET];
		}
	}
	destTexture[index] = sum;
}
#endif
#endif