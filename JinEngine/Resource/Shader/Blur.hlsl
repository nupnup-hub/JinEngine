
Texture2D srcTexture : register(t0); 
RWTexture2D<float4> destTexture : register(u0);
   
cbuffer BlurInfo : register(b0)
{
	float2 size;
	float2 invSize;
};
 
#ifdef USE_3x3_KERNEL
#define RADIUS 1
#elif USE_5x5_KERNEL
#define RADIUS 2
#elif USE_7x7_KERNEL
#define RADIUS 3
#else 
#endif

#ifdef RADIUS
#define KENELSIZE RADIUS * 2 + 1
cbuffer KernelData : register(b1)
{
	float kernel[KENELSIZE][KENELSIZE];
	float kernelPad00;
	float kernelPad01;
	float kernelPad02;
};
#endif

#if defined (DIMX) && defined (DIMY)
[numthreads(DIMX, DIMY, 1)]
void BoxBlur(int3 dispatchThreadID : SV_DispatchThreadID)
{
	if (size.x <= dispatchThreadID.x || size.y <= dispatchThreadID.y)
		return;
 
	float4 sum = 0.0f;
	int2 index = int2(dispatchThreadID.x, dispatchThreadID.y);
	[unroll]
	for (int i = -RADIUS; i <= RADIUS; ++i)
	{
		[unroll]
		for (int j = -RADIUS; j <= RADIUS; ++j)
			sum += srcTexture.Load(int3(index + int2(i, j), 0));
	}
	destTexture[index] = sum / (RADIUS * RADIUS);
}

[numthreads(DIMX, DIMY, 1)]
void GausiaanBlur(int3 dispatchThreadID : SV_DispatchThreadID)
{
	if (size.x <= dispatchThreadID.x || size.y <= dispatchThreadID.y)
		return;
 
	float4 sum = 0.0f;
	int2 index = int2(dispatchThreadID.x, dispatchThreadID.y);
	[unroll]
	for (int i = -RADIUS; i <= RADIUS; ++i)
	{
		[unroll]
		for (int j = -RADIUS; j <= RADIUS; ++j)
			sum += srcTexture.Load(int3(index + int2(i, j), 0)) * kernel[RADIUS + i][RADIUS + j];
	}
	destTexture[index] = sum;
}
#endif