Texture2D srcTexture : register(t0);
RWTexture2D<float4> destTexture : register(u0);

cbuffer DownSamplingInfo : register(b0)
{
	uint2 srcSize;
	uint2 destSize;
	uint srcMipLevel;
	uint samplingState;
	uint2 dispatchDim; 
};

SamplerState samLinearClamp : register(s0);

#define WIDTH_HEIGHT_EVEN 1
#define WIDTH_EVEN_HEIGHT_ODD 2
#define WIDTH_ODD_HEIGHT_EVEN 3
#define WIDTH_HEIGHT_ODD 4
#define BLUR_FACTOR 2

#ifdef USE_3x3_KERNEL
#define RADIUS 1
#elif USE_5x5_KERNEL
#define RADIUS 2
#elif USE_7x7_KERNEL
#define RADIUS 3
#elif USE_2x2_KERNEL
#define RADIUS 1
#define KERNEL_COUNT_IS_EVEN_NUMBER 1
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
	uint kernelPad00;
	uint kernelPad01;
	uint kernelPad02;
};
float4 SampleSrcTexure(float2 uv)
{
	const float2 texelSize = 1.0f / srcSize;
	float4 sum = float4(0, 0, 0, 0);
	[unroll]
	for (int i = KENEL_START; i <= KENEL_END; ++i)
	{
		[unroll]
		for (int j = KENEL_START; j <= KENEL_END; ++j)
		{
			sum += srcTexture.SampleLevel(samLinearClamp, uv + texelSize * int2(i, j), srcMipLevel) * kernel[i + KENEL_OFFSET][j + KENEL_OFFSET];
		}
	}
	return sum;
}
#endif
 
#if defined (DIMX) && defined (DIMY)
[numthreads(DIMX, DIMY, 1)]
void DownSamplingUseBox(int3 dispatchThreadID : SV_DispatchThreadID)
{
	if (destSize.x <= dispatchThreadID.x || destSize.y <= dispatchThreadID.y)
		return;
  
	uint textureXFactor = dispatchThreadID.x;
	uint textureYFactor = dispatchThreadID.y;
	const float2 texelSize = 1.0f / srcSize;
	const uint maxPixelCount = destSize.x * destSize.y;
	
	while (maxPixelCount > (textureXFactor + (textureYFactor * destSize.x)))
	{
		int2 destIndex = int2(textureXFactor, textureYFactor);
		float4 sum = float4(0, 0, 0, 0);
		[unroll]
		for (int i = 0; i < BLUR_FACTOR; ++i)
		{
			[unroll]
			for (int j = 0; j < BLUR_FACTOR; ++j)
			{
				int2 srcIndex = destIndex * 2 + int2(i, j);
				float2 uv = (srcIndex + float2(0.5f, 0.5f)) * texelSize;
				float4 sampleColor = srcTexture.SampleLevel(samLinearClamp, uv, srcMipLevel) +
				srcTexture.SampleLevel(samLinearClamp, uv + texelSize * uint2(1, 0), srcMipLevel) +
				srcTexture.SampleLevel(samLinearClamp, uv + texelSize * uint2(0, 1), srcMipLevel) +
				srcTexture.SampleLevel(samLinearClamp, uv + texelSize * uint2(1, 1), srcMipLevel);
				
				int sampleCount = 4;
				bool shouldIncludeExtraColumnFromPreviousLevel = (((uint) srcSize.x & 1) != 0);
				bool shouldIncludeExtraRowFromPreviousLevel = (((uint) srcSize.y & 1) != 0);
				if (shouldIncludeExtraColumnFromPreviousLevel)
				{
					sampleColor += srcTexture.SampleLevel(samLinearClamp, uv + texelSize * uint2(2, 0), srcMipLevel);
					sampleColor += srcTexture.SampleLevel(samLinearClamp, uv + texelSize * uint2(2, 1), srcMipLevel);

					// In the case where the width and height are both odd, need to include the
					// 'corner' value as well.
					if (shouldIncludeExtraRowFromPreviousLevel)
					{
						sampleColor += srcTexture.SampleLevel(samLinearClamp, uv + texelSize * uint2(2, 2), srcMipLevel);
						++sampleCount;
					}
					sampleCount += 2;
				}
				if (shouldIncludeExtraRowFromPreviousLevel)
				{
					sampleColor += srcTexture.SampleLevel(samLinearClamp, uv + texelSize * uint2(0, 2), srcMipLevel);
					sampleColor += srcTexture.SampleLevel(samLinearClamp, uv + texelSize * uint2(1, 2), srcMipLevel);
					sampleCount += 2;
				}
				sum += sampleColor / sampleCount;
			}
		}
			 
		destTexture[destIndex.xy] = sum / (BLUR_FACTOR * BLUR_FACTOR);
		textureXFactor += dispatchDim.x;
		if (textureXFactor >= destSize.x)
		{
			textureXFactor = dispatchThreadID.x;
			textureYFactor += dispatchDim.y;
		}
	}
}
#ifdef RADIUS
[numthreads(DIMX, DIMY, 1)]
void DownSamplingUseKernel(int3 dispatchThreadID : SV_DispatchThreadID)
{
	if (destSize.x <= dispatchThreadID.x || destSize.y <= dispatchThreadID.y)
		return;
  
	uint textureXFactor = dispatchThreadID.x;
	uint textureYFactor = dispatchThreadID.y;
	const float2 texelSize = 1.0f / srcSize;
	const uint maxPixelCount = destSize.x * destSize.y;
	uint xMod = srcSize.x % 2;
	uint yMod = srcSize.y % 2;
	uint dimension = 0;

	if (xMod == 0 && yMod == 0)
		dimension = WIDTH_HEIGHT_EVEN;
	else if (xMod == 0)
		dimension = WIDTH_EVEN_HEIGHT_ODD;
	else if (yMod == 0)
		dimension = WIDTH_ODD_HEIGHT_EVEN;
	else
		dimension = WIDTH_HEIGHT_ODD;
	
	while (maxPixelCount > (textureXFactor + (textureYFactor * destSize.x)))
	{
		int2 destIndex = int2(textureXFactor, textureYFactor);
		int2 srcIndex = destIndex * 2;
		
		float4 sum = float4(0, 0, 0, 0);
		[unroll]
		for(int i = 0; i < BLUR_FACTOR; ++i)
		{
			[unroll]
			for(int j = 0; j < BLUR_FACTOR; ++j)
			{
				float4 sampleColor = float4(0, 0, 0, 0);
				float2 sampleIndex = (srcIndex + int2(i, j));
				if (samplingState == WIDTH_HEIGHT_EVEN)
				{
					sampleColor = SampleSrcTexure(sampleIndex * texelSize);
				}
				else if (samplingState ==WIDTH_EVEN_HEIGHT_ODD)
				{
					sampleColor = SampleSrcTexure((sampleIndex + float2(0, 0.75f)) * texelSize);
					sampleColor += SampleSrcTexure((sampleIndex + float2(0, 1.25f))* texelSize);
					sampleColor *= 0.5f;
				}
				else if (samplingState ==WIDTH_ODD_HEIGHT_EVEN)
				{ 
					sampleColor = SampleSrcTexure((sampleIndex + float2(0.75f, 0)) * texelSize);
					sampleColor += SampleSrcTexure((sampleIndex + float2(1.25f, 0)) * texelSize);
					sampleColor *= 0.5f;		
				}
				else
				{ 
					sampleColor = SampleSrcTexure((sampleIndex + float2(0.75f, 0.75f)) * texelSize);
					sampleColor += SampleSrcTexure((sampleIndex + float2(0.75f, 1.25f)) * texelSize);
					sampleColor = SampleSrcTexure((sampleIndex + float2(1.25f, 0.75f)) * texelSize);
					sampleColor += SampleSrcTexure((sampleIndex + float2(1.25f, 1.25f)) * texelSize);
					sampleColor *= 0.25f;
				}
				sum += sampleColor;
			}
		}
		 
		destTexture[destIndex.xy] = sum / (BLUR_FACTOR * BLUR_FACTOR); 
		textureXFactor += dispatchDim.x;
		if (textureXFactor >= destSize.x)
		{
			textureXFactor = dispatchThreadID.x;
			textureYFactor += dispatchDim.y;
		}
	}
}
#endif
#endif
